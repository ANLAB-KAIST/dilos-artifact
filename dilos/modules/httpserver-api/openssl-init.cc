/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <iostream>
#include <fstream>
#include <mutex>
#include <osv/debug.hh>
#include <assert.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>

#include "openssl-init.hh"

static void seed_openssl()
{
    debug("Seeding OpenSSL...\n");

    for (;;) {
        char buf[32];
        std::ifstream entropy_source("/dev/random", std::ios::binary);
        if (!entropy_source) {
            throw std::runtime_error("Failed to open random device");
        }
        entropy_source.read(buf, sizeof(buf));
        assert(entropy_source.gcount() > 0);
        RAND_seed(buf, entropy_source.gcount());

        if (RAND_status()) {
            break;
        }

        // Different versions of OpenSSL require different
        // amount of entropy to get unblocked. For example
        // fedora's openssl-1.0.1h-5.fc20 needs 48 bytes instead
        // of 32, which is what is required by its upstream version
        // (tag OpenSSL_1_0_1h). In general, we should make no assumptions.
        debug("Still not seeded, retrying\n");
    }

    debug("OpenSSL seeding done.\n");
}

void ensure_openssl_initialized()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        SSL_load_error_strings();

        if (!SSL_library_init()) {
            throw std::runtime_error("SSL_library_init() failed");
        }

        seed_openssl();
    });
}
