#include <stdlib.h>

namespace ddc {
namespace alloc {
bool pre_init(void *addr, size_t len);
void *pre_alloc_buffer(size_t size);
void pre_free_buffer(void *ptr);
}  // namespace alloc

}  // namespace ddc