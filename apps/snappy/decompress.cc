#include <cstdint>
#include <string>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <iostream>

#include <snappy.h>

#include "helper.hh"

using namespace std;

constexpr uint32_t kCompressedFileSize = 507860747;
constexpr uint32_t kNumCompressedFiles = 30;
void *buffers[kNumCompressedFiles - 1];

string read_file_to_string(const string &file_path) {
  ifstream fs(file_path);
  auto guard = helpers::finally([&]() { fs.close(); });
  return string((std::istreambuf_iterator<char>(fs)),
                std::istreambuf_iterator<char>());
}

void write_file_to_string(const string &file_path, const string &str) {
  std::ofstream fs(file_path);
  fs << str;
  fs.close();
}

void uncompress_files_bench(const string &in_file_path,
                            const string &out_file_path) {
  string in_str = read_file_to_string(in_file_path);
  string out_str;

  for (uint32_t i = 0; i < kNumCompressedFiles - 1; i++) {
    buffers[i] = malloc(kCompressedFileSize);
    if (buffers[i] == nullptr) {
      abort();
    }
    memcpy(buffers[i], in_str.data(), in_str.size());
  }

  auto start = chrono::steady_clock::now();
  for (uint32_t i = 0; i < kNumCompressedFiles; i++) {
    std::cout << "Uncompressing file " << i << std::endl;
    if (i == 0) {
      snappy::Uncompress(in_str.data(), in_str.size(), &out_str);
    } else {
      snappy::Uncompress((const char *)buffers[i - 1], kCompressedFileSize,
                         &out_str);
    }
  }
  auto end = chrono::steady_clock::now();
  cout << "Elapsed time in microseconds : "
       << chrono::duration_cast<chrono::microseconds>(end - start).count()
       << " µs" << endl;

  for (uint32_t i = 0; i < kNumCompressedFiles - 1; i++) {
    free(buffers[i]);
  }

  // write_file_to_string(out_file_path, out_str);
}

int main() {
  uncompress_files_bench("/mnt/enwik9.compressed",
                         "/mnt/enwik9.uncompressed.tmp");
  return 0;
}