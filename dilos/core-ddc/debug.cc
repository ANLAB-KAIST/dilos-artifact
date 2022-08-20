#include <ddc/debug.hh>
#include <osv/debug.hh>
#include <osv/trace.hh>
TRACEPOINT(trace_ddc_remote_push, "paddr=%p token=%lx, offset=%lx vec=%lx",
           void *, uint64_t, uint64_t, uint64_t);
TRACEPOINT(trace_ddc_remote_fetch, "paddr=%p token=%lx, offset=%lx vec=%lx",
           void *, uint64_t, uint64_t, uint64_t);
TRACEPOINT(trace_ddc_remote_poll, "token=%lx", uint64_t);
TRACEPOINT(trace_ddc_remote_sge, "paddr=%lx length=%lx, lkey=%x", uint64_t,
           uint64_t, uint32_t);

void ddc_debug(const char *text, uint64_t val) { debug_early_u64(text, val); }

void ddc_remote_log_push(void *paddr, uint64_t token, uint64_t offset,
                         uint64_t vec) {
    trace_ddc_remote_push(paddr, token, offset, vec);
}
void ddc_remote_log_fetch(void *paddr, uint64_t token, uint64_t offset,
                          uint64_t vec) {
    trace_ddc_remote_fetch(paddr, token, offset, vec);
}

void ddc_remote_log_sge(uint64_t paddr, uint64_t length, uint32_t lkey) {
    trace_ddc_remote_sge(paddr, length, lkey);
}

void ddc_remote_log_poll(uint64_t token) { trace_ddc_remote_poll(token); }