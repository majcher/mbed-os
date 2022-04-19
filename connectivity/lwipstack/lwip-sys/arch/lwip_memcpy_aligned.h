#ifndef LWIP_MEMCPY_ALIGNED_H_
#define LWIP_MEMCPY_ALIGNED_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

void* thumb2_aligned_memcpy(void* pDest, const void* pSource, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* LWIP_MEMCPY_ALIGNED_H_ */
