#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/mman.h>
#include <stdint.h>
#include <sys/mman.h>
#define MAP_DDC 0x100000
#define MAP_LOCAL 0x200000
#define MAP_FIX_FIRST_TWO 0x400000

#define MAP_ALIGNED(n) ((n) << MAP_ALIGNMENT_SHIFT)
#define MAP_ALIGNMENT_SHIFT 24

#define ADDR_SG (0x400000000000ULL)

#define MADV_DDC_CLEAN 0x100
#define MADV_DDC_PAGEOUT 0x101
#define MADV_DDC_PAGEOUT_SG 0x102
#define MADV_DDC_PRINT_STAT 0x103

#define MADV_DDC_MASK (0x10000 - 1)

#ifdef __cplusplus
}
#endif