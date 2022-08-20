#pragma once

#include <osv/mmu.hh>
namespace ddc {

constexpr int base_page_level = 0;
constexpr int base_page_shift = 12;
constexpr size_t base_page_size = mmu::page_size_level(base_page_level);
static_assert(base_page_size == (1ULL << base_page_shift));

void reduce_total_memory(size_t memory);

}  // namespace ddc
