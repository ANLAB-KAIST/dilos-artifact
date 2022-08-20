#ifndef MIMALLOC_BM_OPTION_H
#define MIMALLOC_BM_OPTION_H

#include <stdlib.h>


#define BM_MIN_SIZE (25)
#define BM_MIN_BSIZE (32)
#define BM_MAX_BSIZE (1024)

#define BM_MIN_WSIZE ((BM_MIN_BSIZE/sizeof(uintptr_t)) - 1)
#define BM_MAX_WSIZE ((BM_MAX_BSIZE/sizeof(uintptr_t)) - 1)


#define be_malloc malloc
#define be_calloc calloc
#define be_free  free


static inline bool is_bm_block_size(uint32_t block_size){
    return block_size >= BM_MIN_BSIZE && block_size <= BM_MAX_BSIZE;
}
static inline bool is_bm_wsize(size_t size){
    return size >= BM_MIN_WSIZE && size <= BM_MAX_WSIZE;
}

static inline bool is_bm_page(const mi_page_t* page){
    return is_bm_block_size(page->xblock_size);
}
static inline bool check_bm_page(const mi_bm_page_t* page){
    return is_bm_block_size(BM_TO_CONST_PAGE(page)->xblock_size);
}

#endif