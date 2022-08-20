#pragma once

#include <ddc/mman.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define PREFETCH_PAGE_SIZE 4096

struct ddc_event_t {
    enum {
        DDC_EVENT_PREFETCH_START,
        DDC_EVENT_SUBPAGE_FETCHED,
    } type;
    uintptr_t fault_addr;  // initial fault address

    union {
        struct {
            size_t hits;       // previous hits (thread local)
            uintptr_t *pages;  // hit pages (size: hits)
        } start;
        struct {
            uintptr_t addr;           // virtual address of sub page
            void *sub_page;           // dereferencable sub page ptr
            size_t size_of_sub_page;  // size of sub page
            unsigned long metadata;   // metadata for recursion
        } sub_page;
    };
};

typedef int (*ddc_handler_t)(const struct ddc_event_t *event);

// TODO: review unused enum
enum ddc_prefetch_result_t {
    DDC_RESULT_OK_ISSUED,
    DDC_RESULT_OK_LOCAL,
    DDC_RESULT_OK_PROCESSING,
    DDC_RESULT_ERR_GIVEUP,
    DDC_RESULT_ERR_QP_FULL,
    DDC_RESULT_ERR_NOT_DDC,
    DDC_RESULT_ERR_UNKNOWN,
};

struct ddc_prefetch_t {
    enum ddc_prefetch_type_t {
        DDC_PREFETCH_PAGE,
        DDC_PREFETCH_SUBPAGE,
        DDC_PREFETCH_BOTH,
    } type;
    uintptr_t addr;

    union {
        // Map page
        //  addr: page addr
        //  length: length of continuous pages (TODO)
        // ok : OK_ISSUED or OK_LOCAL
        // err: ERR_QP_FULL or ERR_NOT_DDC or ERR_UNKNOWN
        struct {
        } page;

        struct {
            size_t count;
        } page_range;
        // Get sub page
        //  addr: sub page addr
        //  length: length of sub page ( < 128B )
        //  buffer: read buffer for locally available sub page
        // ok : OK_ISSUED or OK_LOCAL (with buffer)
        // err: ERR_QP_FULL or ERR_NOT_DDC or ERR_UNKNOWN
        struct {
            void *buff;
            size_t length;
        } sub_page;

        // Get sub page and map its page.
        //  addr: sub page addr
        //  length: length of sub page ( < 128B )
        //  buffer: read buffer for locally available sub page
        // ok : OK_ISSUED or OK_LOCAL (with buffer)
        // err: ERR_QP_FULL or ERR_NOT_DDC or ERR_UNKNOWN
        struct {
            void *buff;
            size_t length;
        } both;
    };
    unsigned long metadata;
};

enum ddc_prefetch_result_t ddc_prefetch(const struct ddc_event_t *event,
                                        struct ddc_prefetch_t *command);

#define DDC_HANDLER_HITTRACK 1
void ddc_register_handler(ddc_handler_t handler, int option);

#ifdef __cplusplus
}
#endif