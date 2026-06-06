#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef __SIZE_TYPE__ size_t;
#endif

#undef memcpy
#undef memset
#undef memmove
#undef memcmp
#undef strlen

static inline void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

static inline void *memset(void *s, int c, size_t n) {
    char *p = s;
    while (n--) {
        *p++ = (char)c;
    }
    return s;
}

static inline void *memmove(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

static inline int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

static inline size_t strlen(const char *s) {
    const char *p = s;
    while (*p) {
        p++;
    }
    return p - s;
}

static inline char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++)) ;
    return dest;
}

static inline char *strncpy(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (n && (*d++ = *src++)) {
        n--;
    }
    while (n--) {
        *d++ = '\0';
    }
    return dest;
}

static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static inline int strncmp(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    while (n-- && *s1 && (*s1 == *s2)) {
        if (n == 0 || *s1 == '\0') break;
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static inline char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    if (c == '\0') {
        return (char *)s;
    }
    return (void *)0;
}

#endif /* _STRING_H */
