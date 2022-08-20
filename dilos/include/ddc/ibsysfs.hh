#pragma once

#include <memory>

#include "fs/pseudofs/pseudofs.hh"

int ibsysfs_prepare(std::shared_ptr<pseudofs::pseudo_dir_node> infiniband,
                    std::shared_ptr<pseudofs::pseudo_dir_node> infiniband_verbs,
                    uint64_t &inode_count);

int ibsysfs_mount();