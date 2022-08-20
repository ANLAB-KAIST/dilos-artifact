#pragma once

#include <memory>

namespace helpers {
template <class F>
static inline auto finally(F f) noexcept(noexcept(F(std::move(f)))) {
  auto x = [f = std::move(f)](void *) { f(); };
  return std::unique_ptr<void, decltype(x)>(reinterpret_cast<void *>(1),
                                            std::move(x));
}
}