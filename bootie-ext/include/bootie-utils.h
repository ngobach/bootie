#ifndef BOOTIE_UTILS_H
#define BOOTIE_UTILS_H

typedef struct {
    const char *key;
    const char *val;
} bt_token_t;

/* Replace %key% tokens in input using a token table. */
static inline void bt_token_replace(char *out, int outsz, const char *input,
                                     bt_token_t *tokens, int ntokens) {
    char tmp[512];
    int ti = 0;
    while (*input && ti < (int)sizeof(tmp) - 1) {
        if (*input == '%') {
            const char *end = strstr(input + 1, "%");
            if (end) {
                int klen = (int)(end - input - 1);
                const char *repl = NULL;
                for (int i = 0; i < ntokens; i++) {
                    int tlen = strlen(tokens[i].key);
                    if (klen == tlen && memcmp(input + 1, tokens[i].key, tlen) == 0) {
                        repl = tokens[i].val;
                        break;
                    }
                }
                if (repl) {
                    int replen = strlen(repl);
                    int copy = replen;
                    if (ti + copy > (int)sizeof(tmp) - 1)
                        copy = (int)sizeof(tmp) - 1 - ti;
                    memcpy(tmp + ti, repl, copy);
                    ti += copy;
                    input = end + 1;
                    continue;
                }
            }
        }
        tmp[ti++] = *input++;
    }
    tmp[ti] = '\0';
    int i = 0;
    while (i < outsz - 1 && tmp[i]) { out[i] = tmp[i]; i++; }
    out[i] = '\0';
}

/* Case-insensitive glob match: * = any sequence, ? = single char.
 * Requires <bootie.h> (for tolower) to be included first. */
static inline int bt_fnmatch(const char *p, const char *n) {
    while (*p) {
        if (*p == '*') {
            p++;
            if (!*p) return 1;
            while (*n) {
                if (bt_fnmatch(p, n)) return 1;
                n++;
            }
            return 0;
        } else if (*p == '?') {
            if (!*n) return 0;
            p++; n++;
        } else {
            if (tolower((unsigned char)*n) != tolower((unsigned char)*p))
                return 0;
            p++; n++;
        }
    }
    return !*n;
}

#endif /* BOOTIE_UTILS_H */
