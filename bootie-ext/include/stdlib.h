#ifndef _STDLIB_H
#define _STDLIB_H

#include <stdint.h>

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef __SIZE_TYPE__ size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

static inline int abs(int x) {
    return x < 0 ? -x : x;
}

static inline long labs(long x) {
    return x < 0 ? -x : x;
}

#endif /* _STDLIB_H */
