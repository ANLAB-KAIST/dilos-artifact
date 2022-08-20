

#include <algorithm>
#include <array>
#include <boost/intrusive/set.hpp>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <random>

#define KEY_RANGE 100000000
#define VALUE_SIZE 150

#define DI_PAGE_SIZE (4096)
#define DI_UNIT_SIZE (64)
#define DI_USED_SIZE ((DI_PAGE_SIZE / DI_UNIT_SIZE) + 1)
#define DI_MAX_THREAD (32)
static size_t di_used[DI_USED_SIZE];

namespace bi = boost::intrusive;

struct object {
    bi::set_member_hook<> hook;
    size_t key;
    char value[VALUE_SIZE];
    object(size_t key) : key(key) { memset(value, 12, VALUE_SIZE); }
};

class compare {
   public:
    bool operator()(const object &a, const object &b) const {
        return a.key < b.key;
    }
};

typedef bi::set<object, bi::compare<compare>,
                bi::member_hook<object, bi::set_member_hook<>, &object::hook>,
                bi::optimize_size<true> >
    set_base;

extern "C" {
// size_t mi_thread_stats_fragmentation(size_t page_size, size_t unit_size,
//                                      size_t *used);
// void mi_stats_print(void *out);
}

static std::array<size_t, KEY_RANGE> key_sequence;

int main(int argc, char *argv[]) {
    std::cout << "Starting..." << std::endl;
    set_base testing;
    std::random_device rd;
    std::mt19937 g(rd());

    for (size_t i = 0; i < KEY_RANGE; ++i) {
        key_sequence[i] = i;
    }
    std::cout << "Key seqence populate done" << std::endl;
    std::shuffle(key_sequence.begin(), key_sequence.end(), g);
    std::cout << "Shuffle done" << std::endl;

    for (size_t i = 0; i < KEY_RANGE; ++i) {
        testing.insert(*new object(i));
    }
    std::cout << "Populate done" << std::endl;

    // size_t total_page =
    //     mi_thread_stats_fragmentation(DI_PAGE_SIZE, DI_UNIT_SIZE, di_used);
    // printf("initial:\n");
    // printf("%ld\n", total_page);
    // for (size_t j = 0; j < DI_USED_SIZE; ++j) {
    //     printf("%ld\n", di_used[j]);
    // }
    // printf("\n\n");
    std::uniform_int_distribution<> distrib(0, KEY_RANGE - 1);
    for (size_t iter = 0; iter < 13; ++iter) {
        // remove 10000000
        for (size_t i = 0; i < KEY_RANGE / 10; ++i) {
            size_t key = distrib(g);
            auto it = testing.find(key);
            if (it != testing.end()) {
                auto &elem = *it;
                testing.erase(it);
                delete &elem;
            }
        }
        // size_t total_page =
        //     mi_thread_stats_fragmentation(DI_PAGE_SIZE, DI_UNIT_SIZE,
        //     di_used);
        // printf("iter: %ld, keys = %ld\n", iter, testing.size());
        // printf("%ld\n", total_page);
        // for (size_t j = 0; j < DI_USED_SIZE; ++j) {
        //     printf("%ld\n", di_used[j]);
        // }
        // printf("\n\n");
    }

    return 0;
}