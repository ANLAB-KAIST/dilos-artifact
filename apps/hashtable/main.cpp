
#include <local_concurrent_hopscotch.hpp>
#include <zipf.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <random>
#include <thread>

std::atomic_flag flag;
thread_local std::unique_ptr<std::mt19937> generator;
__thread uint32_t per_core_req_idx = 0;




namespace far_memory {
using GenericConcurrentHopscotch = LocalGenericConcurrentHopscotch;


constexpr static uint32_t kNumKVPairs = 1 << 27;
constexpr static uint32_t kReqSeqLenPerCore = kNumKVPairs;
thread_local uint32_t all_zipf_key_indice[kReqSeqLenPerCore];
class FarMemTest {
private:
  constexpr static uint64_t kFarMemSize = (1ULL << 30);
  constexpr static uint32_t kNumGCThreads = 100;
  constexpr static uint32_t kKeyLen = 12;
  constexpr static uint32_t kValueLen = 4;
  constexpr static uint32_t kLocalHashTableNumEntriesShift =
      28; // 128 M entries
  constexpr static uint32_t kRemoteHashTableNumEntriesShift =
      27; // 128 M entries
  constexpr static uint64_t kRemoteHashTableDataSize = (4ULL << 30); // 4 GB
  constexpr static uint32_t kNumItersPerScope = 64;
  constexpr static uint32_t kNumMutatorThreads = 400;
  constexpr static uint32_t kMonitorPerIter = 262144;
  constexpr static uint32_t kMinMonitorIntervalUs = 10 * 1000 * 1000;
  constexpr static uint32_t kMaxRunningUs = 200 * 1000 * 1000; // 200 seconds
  constexpr static double kZipfParamS = 0;

  struct Key {
    char data[kKeyLen];
  };

  struct Value {
    char data[kValueLen];
  };

  struct alignas(64) Cnt {
    uint64_t c;
  };

  alignas(helpers::kHugepageSize) Key all_gen_keys[kNumKVPairs];
  Cnt cnts[kNumMutatorThreads];
  std::vector<double> mops_vec;

  uint64_t prev_sum_cnts = 0;
  uint64_t prev_us = 0;
  uint64_t running_us = 0;

  inline void append_uint32_to_char_array(uint32_t n, uint32_t suffix_len,
                                          char *array) {
    uint32_t len = 0;
    while (n) {
      auto digit = n % 10;
      array[len++] = digit + '0';
      n = n / 10;
    }
    while (len < suffix_len) {
      array[len++] = '0';
    }
    std::reverse(array, array + suffix_len);
  }

  inline void random_string(char *data, uint32_t len) {
    std::uniform_int_distribution<int> distribution('a', 'z' + 1);
    for (uint32_t i = 0; i < len; i++) {
      data[i] = char(distribution(*generator));
    }
  }

  inline void random_key(char *data, uint32_t tid) {
    auto tid_len = helpers::static_log(10, kNumMutatorThreads);
    random_string(data, kKeyLen - tid_len);
    append_uint32_to_char_array(tid, tid_len, data + kKeyLen - tid_len);
  }

  void prepare(GenericConcurrentHopscotch *hopscotch_ptr) {


    zipf_table_distribution<> zipf(kNumKVPairs, kZipfParamS);


    uint32_t* all_zipf_key_indice_init = (uint32_t*)calloc(kReqSeqLenPerCore, sizeof(uint32_t));
    for (uint32_t i = 0; i < kReqSeqLenPerCore; i++) {
      auto idx = zipf(*generator);
      BUG_ON(idx >= kNumKVPairs);
      all_zipf_key_indice_init[i] = idx;
    }

    std::vector<std::thread> threads;
    for (uint32_t tid = 0; tid < kNumMutatorThreads; tid++) {
      threads.emplace_back(std::thread([&, tid]() {
      std::random_device rd;
      generator.reset(new std::mt19937(rd()));
        auto num_kv_pairs = kNumKVPairs / kNumMutatorThreads;
        if (tid == kNumMutatorThreads - 1) {
          num_kv_pairs += kNumKVPairs % kNumMutatorThreads;
        }
        auto *thread_gen_keys =
            &all_gen_keys[tid * (kNumKVPairs / kNumMutatorThreads)];
        Key key;
        Value val;
        for (uint32_t i = 0; i < num_kv_pairs; i++) {
          random_key(key.data, tid);
          random_string(val.data, kValueLen);
          hopscotch_ptr->put(kKeyLen, (const uint8_t *)key.data,
                             kValueLen, (const uint8_t *)val.data);
          thread_gen_keys[i] = key;
        }
        memcpy(all_zipf_key_indice, all_zipf_key_indice_init,
             sizeof(uint32_t) * kReqSeqLenPerCore);
      }));
    }
    for (auto &thread : threads) {
      thread.join();
    }

    free(all_zipf_key_indice_init);

  }

  void monitor_perf() {
    if (!flag.test_and_set()) {
      auto us = microtime();
      if (us - prev_us > kMinMonitorIntervalUs) {
        uint64_t sum_cnts = 0;
        for (uint32_t i = 0; i < kNumMutatorThreads; i++) {
          sum_cnts += ACCESS_ONCE(cnts[i].c);
        }
        us = microtime();
        auto mops = (double)(sum_cnts - prev_sum_cnts) / (us - prev_us);
        mops_vec.push_back(mops);
        running_us += (us - prev_us);
        if (running_us >= kMaxRunningUs) {
          std::vector<double> last_5_mops(
              mops_vec.end() - std::min(static_cast<int>(mops_vec.size()), 5),
              mops_vec.end());
          std::cout << "mops = "
                    << std::accumulate(last_5_mops.begin(), last_5_mops.end(),
                                       0.0) /
                           last_5_mops.size()
                    << std::endl;
          std::cout << "Done. Force exiting..." << std::endl;
          exit(0);
        }
        prev_us = us;
        prev_sum_cnts = sum_cnts;
      }
      flag.clear();
    }
  }

  void bench_get(GenericConcurrentHopscotch *hopscotch_ptr) {
    prev_us = microtime();
    std::vector<std::thread> threads;
    for (uint32_t tid = 0; tid < kNumMutatorThreads; tid++) {
      threads.emplace_back(std::thread([&, tid]() {
        uint32_t cnt = 0;
        while (1) {
          if (unlikely(cnt % kMonitorPerIter == 0)) {
            monitor_perf();
          }
          auto key_idx =
              all_zipf_key_indice[per_core_req_idx++];
          if (unlikely(per_core_req_idx == kReqSeqLenPerCore)) {
            per_core_req_idx = 0;
          }
          auto &key = all_gen_keys[key_idx];
          uint16_t val_len;
          Value val;
          hopscotch_ptr->get(kKeyLen, (const uint8_t *)key.data,
                             &val_len, (uint8_t *)val.data);
          ACCESS_ONCE(cnts[tid].c)++;
          DONT_OPTIMIZE(val);
        }
      }));
    }
    for (auto &thread : threads) {
      thread.join();
    }
  }

public:
  void run() {

    auto hopscotch = std::unique_ptr<GenericConcurrentHopscotch>(
        new GenericConcurrentHopscotch(
            kLocalHashTableNumEntriesShift,
            kRemoteHashTableDataSize     
            ));
    std::cout << "Prepare..." << std::endl;
    prepare(hopscotch.get());
    std::cout << "Get..." << std::endl;
    bench_get(hopscotch.get());
    hopscotch.reset();
  }
};
} // namespace far_memory


int main(){
  far_memory::FarMemTest test;
  test.run();
;;
    return 0;
}