#if defined(TOOLCHAIN_GCC) && defined(__thumb2__)

#include "lwip_memcpy_aligned.h"

#define VLDN_ALIGN_TO 4

/* This is a hand written Thumb-2 assembly language version of the
   standard C memcpy() function that can be used by the lwIP networking
   stack to improve its performance.  It copies 4 bytes at a time and
   unrolls the loop to perform 4 of these copies per loop iteration.
*/
__attribute__((naked)) void lwip_memcpy_thumb2(void* pDest, const void* pSource, size_t length)
{
    __asm (
        ".syntax unified\n"
        ".thumb\n"

        // Copy 16 bytes at a time first.
        "    lsrs    r3, r2, #4\n"
        "    beq.n   2$\n"
        "1$: ldr     r12, [r1], #4\n"
        "    str     r12, [r0], #4\n"
        "    ldr     r12, [r1], #4\n"
        "    str     r12, [r0], #4\n"
        "    ldr     r12, [r1], #4\n"
        "    str     r12, [r0], #4\n"
        "    ldr     r12, [r1], #4\n"
        "    str     r12, [r0], #4\n"
        "    subs    r3, #1\n"
        "    bne     1$\n"

        // Copy byte by byte for what is left.
        "2$:\n"
        "    ands    r3, r2, #0xf\n"
        "    beq.n   4$\n"
        "3$: ldrb    r12, [r1], #1\n"
        "    strb    r12, [r0], #1\n"
        "    subs    r3, #1\n"
        "    bne     3$\n"

        // Return to caller.
        "4$: bx      lr\n"
    );
}

void* lwip_memcpy_byte_by_byte(void* pDest, const void* pSource, size_t length)
{
    char *pszDest = (char*) pDest;
    const char *pszSource = (const char*) pSource;
    if ((pszDest != NULL) && (pszSource != NULL)) {
        while (length) {
            //Copy byte by byte
            *(pszDest++) = *(pszSource++);
            --length;
        }
    }
    return pDest;
}

void* thumb2_aligned_memcpy(void* pDest, const void* pSource, size_t length)
{
    uintptr_t sourceAddress = (uintptr_t) pSource;
    int sourceAddressExtraBytes = sourceAddress % VLDN_ALIGN_TO;

    if (sourceAddress >= 0x60000000 && sourceAddress <= 0x6FFFFFFF && sourceAddressExtraBytes != 0) {
        char * pcDest = (char *) pDest;
        char const * pcSource = (char const *) pSource;
        int missingBytesForFullAlignment = VLDN_ALIGN_TO - sourceAddressExtraBytes;

//        printf("VLDN memcpy (misaligned - %d) %p -> %p, (%d)\n", sourceAddressExtraBytes, pSource, pDest, length);

        lwip_memcpy_byte_by_byte(pcDest, pcSource, missingBytesForFullAlignment);

        pcDest += missingBytesForFullAlignment;
        pcSource += missingBytesForFullAlignment;
        length -= missingBytesForFullAlignment;

        lwip_memcpy_thumb2(pcDest, pcSource, length);
    } else {
        lwip_memcpy_thumb2(pDest, pSource, length);
    }

    return pDest;
}

#endif
