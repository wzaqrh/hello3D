#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t digest[16]);

#ifdef __cplusplus
}
#endif