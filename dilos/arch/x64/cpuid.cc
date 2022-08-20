/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "cpuid.hh"
#include "processor.hh"
#include "xen.hh"
#include "drivers/clock.hh"
#include <dev/hyperv/include/hyperv.h>

namespace processor {

const features_type& features()
{
    // features() can be used very early, make sure it is initialized
    static features_type f;
    return f;
}

namespace {

struct signature {
    u32 b;
    u32 c;
    u32 d;
};

struct cpuid_bit {
    unsigned leaf;
    char word;
    unsigned bit;
    bool features_type::*flag;
    unsigned subleaf;
    signature* sign;
    const char *name;
};

signature kvm_signature = {
    0x4b4d564b, 0x564b4d56, 0x4d
};

typedef features_type f;

cpuid_bit cpuid_bits[] = {
    { 1, 'c', 0, &f::sse3, 0, nullptr, "sse3" },
    { 1, 'c', 9, &f::ssse3, 0, nullptr, "ssse3" },
    { 1, 'c', 13, &f::cmpxchg16b, 0, nullptr, "cmpxchg16b" },
    { 1, 'c', 19, &f::sse4_1, 0, nullptr, "sse4.1" },
    { 1, 'c', 20, &f::sse4_2, 0, nullptr, "sse4.2" },
    { 1, 'c', 21, &f::x2apic, 0, nullptr, "x2apic" },
    { 1, 'c', 24, &f::tsc_deadline, 0, nullptr, "tsc_deadline" },
    { 1, 'c', 26, &f::xsave, 0, nullptr, "xsave" },
    { 1, 'c', 27, &f::osxsave, 0, nullptr, "osxsave" },
    { 1, 'c', 28, &f::avx, 0, nullptr, "avx" },
    { 1, 'c', 30, &f::rdrand, 0, nullptr, "rdrand" },
    { 1, 'd', 19, &f::clflush, 0, nullptr, "clflush" },
    { 7, 'b', 0, &f::fsgsbase, 0, nullptr, "fgsbase" },
    { 7, 'b', 9, &f::repmovsb, 0, nullptr, "repmovsb" },
    { 0x80000001, 'd', 26, &f::gbpage, 0, nullptr, "gbpage" },
    { 0x80000007, 'd', 8, &f::invariant_tsc, 0, nullptr, "invariant_tsc"},
    { 0x40000001, 'a', 0, &f::kvm_clocksource, 0, &kvm_signature, "kvmclock" },
    { 0x40000001, 'a', 3, &f::kvm_clocksource2, 0, &kvm_signature, "kvmclock2" },
    { 0x40000001, 'a', 6, &f::kvm_pv_eoi, 0, &kvm_signature, "kvm_pv_eoi" },
    { 0x40000001, 'a', 24, &f::kvm_clocksource_stable, 0, &kvm_signature, "kvmclock_stable" },
};

constexpr unsigned nr_cpuid_bits = sizeof(cpuid_bits) / sizeof(*cpuid_bits);

void process_cpuid_bit(features_type& features, const cpuid_bit& b)
{
    bool subleaf = b.leaf == 7;
    auto base = b.leaf & 0xf0000000;
    auto s = b.sign;
    if (s) {
        auto x = cpuid(base);
        if (x.b != s->b || x.c != s->c || x.d != s->d) {
            return;
        }
    }
    unsigned max = cpuid(base).a;
    if (base == 0x40000000 && s == &kvm_signature && max == 0) {
        max = 0x40000001; // workaround kvm bug
    }
    if (b.leaf > max) {
        return;
    }
    if (subleaf && b.subleaf > cpuid(b.leaf, 0).a) {
        return;
    }
    static unsigned cpuid_result::*regs[] = {
        &cpuid_result::a, &cpuid_result::b, &cpuid_result::c, &cpuid_result::d
    };
    auto w = cpuid(b.leaf, b.subleaf).*regs[b.word - 'a'];
    features.*(b.flag) = (w >> b.bit) & 1;
}

void process_xen_bits(features_type &features)
{
    signature sig = { 0x566e6558, 0x65584d4d, 0x4d4d566e };

    for (unsigned base = 0x40000000; base < 0x40010000; base += 0x100) {
        auto x = cpuid(base);
        if (x.b != sig.b || x.c != sig.c || x.d != sig.d) {
            continue;
        }
        xen::xen_init(features, base);
        break;
    }
}

void process_hyperv_bits(features_type &features) {
    if(hyperv_identify() && hyperv_is_timecount_available()) {
        features.hyperv_clocksource = true;
    }
}

void process_cpuid(features_type& features)
{
    for (unsigned i = 0; i < nr_cpuid_bits; ++i) {
        process_cpuid_bit(features, cpuid_bits[i]);
    }
    process_xen_bits(features);
    process_hyperv_bits(features);
}

}

const std::string& features_str()
{
    static std::string cpuid_str;
    if (cpuid_str.size()) {
        return cpuid_str;
    }

    for (unsigned i = 0; i < nr_cpuid_bits; ++i) {
        if (features().*(cpuid_bits[i].flag)) {
            cpuid_str += std::string(cpuid_bits[i].name) + std::string(" ");
        }
    }

    // FIXME: Even though Xen does not have its features in cpuid, there has to
    // be a better way to do it, by creating a string map directly from the xen
    // PV features. But since we add features here very rarely, leave it be for now.
    if (features().xen_clocksource) {
        cpuid_str += std::string("xen_clocksource ");
    }
    if (features().xen_vector_callback) {
        cpuid_str += std::string("xen_vector_callback ");
    }
    if (features().xen_pci) {
        cpuid_str += std::string("xen_pci ");
    }
    cpuid_str.pop_back();
    cpuid_str += "\n";
    cpuid_str += "cpu MHz		: " + std::to_string(1000000000000.0f / clock::get()->processor_to_nano(1000000000));

    return cpuid_str;
}

features_type::features_type()
{
    process_cpuid(*this);
}

}
