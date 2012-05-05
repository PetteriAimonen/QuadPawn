/*
 * klibc/extern.h
 */

#ifndef _KLIBC_EXTERN_H
#define _KLIBC_EXTERN_H

#ifdef __cplusplus
#define __extern extern "C"
#else
#define __extern extern
#endif

#define __alias(x) __attribute__((weak, alias(x)))

#endif				/* _KLIBC_EXTERN_H */
