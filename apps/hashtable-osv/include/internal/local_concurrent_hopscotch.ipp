#pragma once

#include <cstring>

namespace far_memory {

FORCE_INLINE LocalGenericConcurrentHopscotch::BucketEntry::BucketEntry() {
  bitmap = timestamp = 0;
  ptr = nullptr;
}
} // namespace far_memory