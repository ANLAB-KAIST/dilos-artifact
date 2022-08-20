
#include <ddc/prefetch.h>

#include <atomic>
#include <boost/circular_buffer.hpp>
#include <cassert>
#include <cstdlib>
#include <osv/trace.hh>

// #define NO_USE_HIT
TRACEPOINT(trace_ddc_prefetch_majority_offset, "offset: %lx", uintptr_t);
TRACEPOINT(trace_ddc_prefetch_majority_trend,
           "start_offset: %lx mask: %lx, major_delta: %d", unsigned long, long,
           int);

namespace ddc {

static constexpr int max_pages = (1 << 3);

/* LEAP-like START */

struct swap_entry {
    long delta;
    unsigned long entry;
};

static thread_local boost::circular_buffer<swap_entry> trend_history(max_pages);
// back is recent

static int find_trend_in_region(int size, long *major_delta, int *major_count) {
    int maj_index = trend_history.size() - 1, count, i, j;
    long candidate;

    for (i = maj_index - 1, j = 1, count = 1; j < size; i = i - 1, j++) {
        if (trend_history[maj_index].delta == trend_history[i].delta)
            count++;
        else
            count--;
        if (count == 0) {
            maj_index = i;
            count = 1;
        }
    }

    candidate = trend_history[maj_index].delta;
    for (i = trend_history.size() - 1, j = 0, count = 0; j < size;
         i = i - 1, j++) {
        if (trend_history[i].delta == candidate) count++;
    }

    // printk("majority index: %d, candidate: %ld, count:%d\n", maj_index,
    // candidate, count);
    *major_delta = candidate;
    *major_count = count;
    return count > (size / 2);
}

static int find_trend(int *depth, long *major_delta, int *major_count) {
    int has_trend = 0, size = max_pages / 4;
    int max_size = trend_history.size();

    while (has_trend == 0 && size <= max_size) {
        has_trend = find_trend_in_region(size, major_delta, major_count);
        // printk( "at size: %d, trend found? %s\n", size, (has_trend == 0) ?
        // "false" : "true" );
        size *= 2;
    }
    *depth = size;
    return has_trend;
}

static unsigned long swapin_nr_pages(int hits, unsigned long offset) {
    unsigned int pages, last_ra;
    static thread_local unsigned long last_readahead_pages;

    /*
     * This heuristic has been found to work well on both sequential and
     * random loads, swapping to hard disk or to SSD: please don't ask
     * what the "+ 2" means, it just happens to work well, that's all.
     */
    pages = hits + 2;

    // always..
    unsigned int roundup = 4;
    while (roundup < pages) roundup <<= 1;
    pages = roundup;

    if (pages > max_pages) pages = max_pages;

    /* Don't shrink readahead too fast */
    last_ra = last_readahead_pages / 2;
    if (pages < last_ra) pages = last_ra;
    last_readahead_pages = pages;

    return pages;
}

/* LEAP-like END */

int handler_majority(const ddc_event_t *event) {
    if (event->type != ddc_event_t::DDC_EVENT_PREFETCH_START) return 0;

    uintptr_t fault_addr = event->fault_addr;

    for (size_t i = 0; i < event->start.hits; ++i) {
        size_t size = trend_history.size();
        unsigned long offset = event->start.pages[i] >> 12;
        if (size) {
            long offset_delta = offset - trend_history[size - 1].entry;
            trend_history.push_back({offset_delta, offset});
        } else {
            trend_history.push_back({0, offset});
        }
        trace_ddc_prefetch_majority_offset(offset);
    }

    // Push current access
    size_t size = trend_history.size();
    unsigned long offset = fault_addr >> 12;
    trace_ddc_prefetch_majority_offset(offset);
    if (size) {
        long offset_delta = offset - trend_history[size - 1].entry;
        trend_history.push_back({offset_delta, offset});
    } else {
        trend_history.push_back({0, offset});
    }

    unsigned long mask = swapin_nr_pages(event->start.hits, offset) - 1;
    static thread_local uintptr_t prev_fault_addr = 0;
    int has_trend = 0, depth, major_count;
    long major_delta;
    has_trend = find_trend(&depth, &major_delta, &major_count);

    if (has_trend) {
        size_t count = 0;
        unsigned long start_offset = offset;
        trace_ddc_prefetch_majority_trend(start_offset, mask, major_delta);

        // blk_start_plug(&plug);
        // Note: fault page handling has been already issued.
        for (offset = start_offset + 1; count <= mask - 1;
             offset += major_delta, count++) {
            uintptr_t prefetch_addr = offset << 12;

            ddc_prefetch_t command;
            command.type = ddc_prefetch_t::DDC_PREFETCH_PAGE;
            command.addr = prefetch_addr;

            ddc_prefetch_result_t ret = ddc_prefetch(event, &command);
            if (ret == DDC_RESULT_ERR_QP_FULL) {
                break;
            }
            if (ret != DDC_RESULT_OK_ISSUED && ret != DDC_RESULT_OK_LOCAL &&
                ret != DDC_RESULT_OK_PROCESSING &&
                ret != DDC_RESULT_ERR_NOT_DDC) {
                debug_early_u64("Unknown prefetcher return: ", (u64)ret);
                abort();
            }
        }

    } else {
        if (!mask) return 0;
        // Use Readahead (vma based one unlike Leap)
        bool increasing = offset > prev_fault_addr >> 12 ? true : false;
        unsigned long i;
        for (i = 1; i < mask; ++i) {
            uintptr_t prefetch_addr =
                increasing ? ((offset + i) << 12) : ((offset - i) << 12);

            ddc_prefetch_t command;
            command.type = ddc_prefetch_t::DDC_PREFETCH_PAGE;
            command.addr = prefetch_addr;

            ddc_prefetch_result_t ret = ddc_prefetch(event, &command);
            if (ret == DDC_RESULT_ERR_QP_FULL) {
                break;
            }
            if (ret != DDC_RESULT_OK_ISSUED && ret != DDC_RESULT_OK_LOCAL &&
                ret != DDC_RESULT_OK_PROCESSING &&
                ret != DDC_RESULT_ERR_NOT_DDC) {
                debug_early_u64("Unknown prefetcher return: ", (u64)ret);
                abort();
            }
        }
    }

    prev_fault_addr = fault_addr;

    return 0;
}
}  // namespace ddc
