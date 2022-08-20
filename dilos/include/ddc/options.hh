#ifndef DDC_OPTIONS_HH
#define DDC_OPTIONS_HH

#include <functional>
#include <osv/options.hh>

namespace ddc {
namespace options {
void parse_cmdline(
    std::map<std::string, std::vector<std::string>> &options_values);

extern std::string ib_device;
extern int ib_port;
extern std::string ms_ip;
extern int ms_port;
extern int gid_idx;
extern std::string prefetcher;
extern bool no_eviction;
extern int cpu_pin;
extern void *prefetcher_init_f;
}  // namespace options

}  // namespace ddc

#endif
