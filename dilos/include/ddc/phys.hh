#pragma once

#include <array>

namespace memory {

// NOTE: return virt of phys
void *get_phys_max();
constexpr size_t max_phys_list = 16;
const std::array<std::pair<void *, size_t>, max_phys_list> &get_phys_list(
    size_t &n);
}  // namespace memory