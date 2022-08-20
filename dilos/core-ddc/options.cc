#include <ddc/options.hh>
#include <osv/debug.hh>

namespace ddc {
namespace options {

std::string ib_device;
int ib_port;
std::string ms_ip;
int ms_port;
int gid_idx;
std::string prefetcher;
bool no_eviction = false;
int cpu_pin = -1;

static void handle_parse_error(const std::string &message) {
    abort(message.c_str());
}

void parse_cmdline(
    std::map<std::string, std::vector<std::string>> &options_values) {
    if (::options::option_value_exists(options_values, "ib_device")) {
        ib_device =
            ::options::extract_option_value(options_values, "ib_device");
    }

    if (::options::option_value_exists(options_values, "ib_port")) {
        ib_port = std::stoi(
            ::options::extract_option_value(options_values, "ib_port"));
    } else {
        ib_port = -1;
    }

    if (::options::option_value_exists(options_values, "cpu_pin")) {
        cpu_pin = std::stoi(
            ::options::extract_option_value(options_values, "cpu_pin"));
    } else {
        cpu_pin = -1;
    }

    if (::options::option_value_exists(options_values, "ms_ip")) {
        ms_ip = ::options::extract_option_value(options_values, "ms_ip");
    }

    if (::options::option_value_exists(options_values, "ms_port")) {
        ms_port = std::stoi(
            ::options::extract_option_value(options_values, "ms_port"));
    } else {
        ms_port = -1;
    }

    if (::options::option_value_exists(options_values, "gid_idx")) {
        gid_idx = std::stoi(
            ::options::extract_option_value(options_values, "gid_idx"));
    } else {
        gid_idx = -1;
    }
    if (::options::option_value_exists(options_values, "prefetcher")) {
        prefetcher =
            ::options::extract_option_value(options_values, "prefetcher");
    }
    if (::options::option_value_exists(options_values, "prefetcher")) {
        prefetcher =
            ::options::extract_option_value(options_values, "prefetcher");
    }
    no_eviction = ::options::extract_option_flag(options_values, "no_eviction",
                                                 handle_parse_error);
}

}  // namespace options
}  // namespace ddc