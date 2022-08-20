#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ddc/remote.hh>
#include <iostream>
#include <osv/pagealloc.hh>
#include <random>

#ifdef NONOSV
#include <remote/nonosv.hh>
#endif

using namespace ddc;
using namespace std::chrono;

#define DO_SG

constexpr size_t num_page = 1024 * 1024;
constexpr size_t max_wq = 32;
remote_queue test_queue({max_wq}, {max_wq});
uint8_t *pages[num_page];
high_resolution_clock::time_point start[num_page];
high_resolution_clock::time_point mid[num_page];
high_resolution_clock::time_point end[num_page];

#ifdef DO_SG
constexpr uintptr_t default_value = 0x82021020;
// constexpr uintptr_t default_value = 64;
#else
constexpr uintptr_t default_value = 4096;
#endif
// constexpr uintptr_t mask = 0x55555555555555FF;

template <typename T>
static inline void is_same(T a, T b) {
    if (a != b) {
        printf("ERROR!!\n");
        abort();
    };
}

int main(int argc, char *argv[]) {
#ifdef NONOSV
    memory::non_osv_init("mlx5_0", 1, "172.16.0.3", 12345, 1, num_page * 4096);
    ddc::remote_init();
#endif
    uintptr_t param = default_value;
    if (argc == 2) {
        param = std::stoull(argv[1], NULL, 16);
    }

    char dummy[4096];

    high_resolution_clock::duration sum(0);
    std::cout << "param: " << std::hex << param << std::dec << std::endl;

    // remote_queue test_queue({}, {max_wq});
    test_queue.setup();
    std::default_random_engine re(0);
    std::uniform_int_distribution<int> uniform_dist(0, 99999);

    re.seed(0);
    for (size_t i = 0; i < num_page; ++i) {
        pages[i] = (uint8_t *)memory::alloc_page();
        start[i] = high_resolution_clock::now();
        memcpy(pages[i], dummy, 2048);
        mid[i] = high_resolution_clock::now();
        memset(pages[i], 0, 4096);
        end[i] = high_resolution_clock::now();
        *(int *)(pages[i]) = uniform_dist(re);
        *(int *)(pages[i] + 1024) = uniform_dist(re);
        *(int *)(pages[i] + 2048) = uniform_dist(re);
        *(int *)(pages[i] + 3072) = uniform_dist(re);
    }
    sum = high_resolution_clock::duration::zero();
    for (size_t i = 0; i < num_page; ++i) {
        sum += end[i] - start[i];
    }
    sum /= num_page;
    std::cout << "memset 4k: " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;

    sum = high_resolution_clock::duration::zero();
    for (size_t i = 0; i < num_page; ++i) {
        sum += mid[i] - start[i];
    }
    sum /= num_page;
    std::cout << "memcpy 4k: " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;

    bool ret;
    int polled = 0;
    uintptr_t token;
    for (size_t i = 0; i < num_page; ++i) {
        start[i] = high_resolution_clock::now();
#ifdef DO_SG
        ret = test_queue.push_vec(0, i, pages[i], i << 12, param);
#else
        ret = test_queue.push(0, i, pages[i], i << 12, param);
#endif
        assert(ret);
        mid[i] = high_resolution_clock::now();
        do {
            polled = test_queue.poll(&token, 1);
        } while (polled == 0);
        assert(token == i);
        end[i] = high_resolution_clock::now();
    }

    sum = high_resolution_clock::duration::zero();

    for (size_t i = 0; i < num_page; ++i) {
        sum += end[i] - start[i];
    }
    sum /= num_page;
    std::cout << "PUSH: " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;

    sum = high_resolution_clock::duration::zero();

    for (size_t i = 0; i < num_page; ++i) {
        sum += mid[i] - start[i];
    }
    sum /= num_page;
    std::cout << "PUSH(ISSUE): " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;

    for (size_t i = 0; i < num_page; ++i) {
        memset(pages[i], 0, 4096);
    }

    for (size_t i = 0; i < num_page; ++i) {
        start[i] = high_resolution_clock::now();
#ifdef DO_SG
        ret = test_queue.fetch_vec(0, i, pages[i], i << 12, param);
#else
        ret = test_queue.fetch(0, i, pages[i], i << 12, param);
#endif
        assert(ret);
        mid[i] = high_resolution_clock::now();
        do {
            polled = test_queue.poll(&token, 1);
        } while (polled == 0);
        assert(token == i);
        end[i] = high_resolution_clock::now();
    }

    sum = high_resolution_clock::duration::zero();

    for (size_t i = 0; i < num_page; ++i) {
        sum += end[i] - start[i];
    }
    sum /= num_page;
    std::cout << "FETCH: " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;
    re.seed(0);

    sum = high_resolution_clock::duration::zero();

    for (size_t i = 0; i < num_page; ++i) {
        sum += mid[i] - start[i];
    }
    sum /= num_page;
    std::cout << "FETCH(ISSUE): " << duration_cast<nanoseconds>(sum).count()
              << " nanosecodes" << std::endl;

    for (size_t i = 0; i < num_page; ++i) {
        is_same(*(int *)(pages[i]), uniform_dist(re));
        is_same(*(int *)(pages[i] + 1024), uniform_dist(re));
        is_same(*(int *)(pages[i] + 2048), uniform_dist(re));
        is_same(*(int *)(pages[i] + 3072), uniform_dist(re));
    }

    return 0;
}