#pragma once
#include <cstdint>

void ddc_debug(const char *text, uint64_t val);
void ddc_remote_log_push(void *paddr, uint64_t token, uint64_t offset,
                         uint64_t vec);
void ddc_remote_log_fetch(void *paddr, uint64_t token, uint64_t offset,
                          uint64_t vec);
void ddc_remote_log_poll(uint64_t token);
void ddc_remote_log_sge(uint64_t paddr, uint64_t length, uint32_t lkey);