#pragma once
#include <string>

namespace memory {
void non_osv_init(std::string ib_device, int ib_port, std::string ms_ip,
                  int ms_port, int gid_idx, size_t phy_mem);
}