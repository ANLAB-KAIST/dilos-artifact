#pragma once

#include <osv/types.h>

#include <array>
#include <boost/circular_buffer.hpp>
#include <ddc/memory.hh>
#include <osv/mmu.hh>

namespace ddc {

struct tlb_flusher {
   public:
    static constexpr size_t max_tokens = 512;

    boost::circular_buffer<uintptr_t> tokens;
    tlb_flusher() : tokens(max_tokens) {}

    virtual void tlb_flush_after(uintptr_t token) = 0;

    void tlb_push(uintptr_t token) {
        if (tokens.full()) tlb_flush();
        tokens.push_back(token);
    }
    inline bool tlb_empty() { return tokens.empty(); }
    size_t tlb_flush() {
        if (tlb_empty()) {
            return 0;
        }
        mmu::flush_tlb_all();
        size_t tokens_size = tokens.size();
        for (auto i = 0u; i < tokens_size; ++i) {
            auto token = tokens.front();
            tokens.pop_front();
            tlb_flush_after(token);
        }
        return tokens_size;
    }
};

};  // namespace ddc