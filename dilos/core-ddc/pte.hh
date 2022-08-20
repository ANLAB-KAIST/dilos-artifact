#pragma once

#include <osv/types.h>

#include <cassert>
#include <osv/mmu-defs.hh>

namespace ddc {

using namespace mmu;

/*

PTE MAP

|              |    |     |    |        | u | w | v |
|              | 63 | ... | 48 | .......| 2 | 1 | 0 |
| ------------ | -- | --- | -- | -------|-- | - | - |
| Local        |                                | 1 |
| First        |  all zero                  | 0 | 0 |
| Remote (vec) |  12 bits vector        | 1 | 0 | 0 |
| Protect      |  CPU id                | 0 | 1 | 0 |
| ETC          |  data                  | 1 | 1 | 0 |
*/

constexpr size_t remote_pte_offset = 2;
constexpr size_t protect_pte_offset = 3;

constexpr uint64_t remote_vec_full = 64;

inline uint64_t pack_vec(uint8_t *starts, uint8_t *counts, uint8_t len) {
    uint64_t vec = 0;
    for (uint8_t i = 0; i < len; ++i) {
        vec <<= 7;
        vec |= starts[i] & 0x7F;
        vec <<= 7;
        vec |= counts[i] & 0x7F;
    }
    return vec;
}

template <int N>
inline pt_element<N> remote_pte_vec(hw_ptep<N> &_ptep, uint64_t vec) {
    vec <<= 3;
    auto pte = pt_element<N>(vec);
    pte.set_user(true);       // Use user bit
    pte.set_writable(false);  // Use writable bit
    pte.set_valid(false);     // ensure valid is zero return pte;
    return pte;
}

template <int N>
inline pt_element<N> remote_pte_full(hw_ptep<N> &_ptep) {
    return remote_pte_vec(_ptep, remote_vec_full);
}

template <int N>
inline pt_element<N> protected_pte(hw_ptep<N> &_, unsigned cpu_id) {
    assert(cpu_id < (1ULL << (64 - protect_pte_offset)));
    auto pte = pt_element<N>(cpu_id << protect_pte_offset);

    pte.set_writable(true);  // Use writable bit for marking pte type protected
    pte.set_valid(false);    // ensure valid is zero
    pte.set_user(false);     // ensure valid is zero
    return pte;
}

template <int N>
inline pt_element<N> etc_pte(hw_ptep<N> &_, unsigned data) {
    assert(data < (1ULL << (64 - protect_pte_offset)));
    auto pte = pt_element<N>(data << protect_pte_offset);

    pte.set_writable(true);  // Use writable bit for marking pte type protected
    pte.set_valid(false);    // ensure valid is zero
    pte.set_user(true);      // ensure valid is zero
    return pte;
}

template <int N>
inline bool is_remote(pt_element<N> element) {
    return element.user() && !element.valid() && !element.writable();
}
template <int N>
inline bool is_protected(pt_element<N> element) {
    return !element.valid() && element.writable() && !element.user();
}

template <int N>
inline bool is_etc(pt_element<N> element) {
    return !element.valid() && element.writable() && element.user();
}

template <int N>
inline unsigned get_cpu_id(pt_element<N> element) {
    return element.raw() >> protect_pte_offset;
}

template <int N>
inline unsigned get_etc_data(pt_element<N> element) {
    return element.raw() >> protect_pte_offset;
}

template <int N>
inline unsigned get_mask(pt_element<N> element) {
    return element.raw();
}

template <int N>
inline bool is_first(mmu::pt_element<N> pte) {
    return pte.raw() == 0x0;
}
}  // namespace ddc
