#pragma once

#include <ddc/prefetch.h>

namespace ddc {

int handler_readahead(const ddc_event_t *event);
int handler_majority(const ddc_event_t *event);

}  // namespace ddc
