#ifndef BOOTIE_INI_H
#define BOOTIE_INI_H

#include <bootie.h>
#include <bootie-io.h>
#include <bootie-ds.h>

typedef struct bt_ini_entry {
    const char *key;
    const char *value;
} bt_ini_entry_t;

typedef struct bt_ini_section {
    const char *name;
    bt_ini_entry_t *entries;
    int entry_count;
} bt_ini_section_t;

typedef struct bt_ini {
    char *buffer;
    bt_ini_section_t *sections;
    int section_count;
} bt_ini_t;

/* Helper function to check if character is whitespace */
static inline int bt_ini_isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
}

/* Helper function to locate a character in a string */
static inline char *bt_ini_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) {
            return (char *)s;
        }
        s++;
    }
    return NULL;
}

/* Helper to trim leading/trailing whitespace */
static inline char *bt_ini_trim(char *str) {
    if (!str) return NULL;
    while (*str && bt_ini_isspace((unsigned char)*str)) {
        str++;
    }
    int len = strlen(str);
    while (len > 0 && bt_ini_isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\0';
        len--;
    }
    return str;
}

/* Helper to strip comments, preserving quoted strings */
static inline void bt_ini_strip_comments(char *str) {
    if (!str) return;
    int in_quote = 0;
    char quote_char = '\0';
    for (int i = 0; str[i]; i++) {
        if (!in_quote && (str[i] == '"' || str[i] == '\'')) {
            in_quote = 1;
            quote_char = str[i];
        } else if (in_quote && str[i] == quote_char) {
            in_quote = 0;
        } else if (!in_quote && (str[i] == ';' || str[i] == '#')) {
            str[i] = '\0';
            break;
        }
    }
}

/* Helper to trim matching double or single quotes */
static inline char *bt_ini_trim_quotes(char *str) {
    if (!str) return NULL;
    int len = strlen(str);
    if (len >= 2) {
        if ((str[0] == '"' && str[len - 1] == '"') ||
            (str[0] == '\'' && str[len - 1] == '\'')) {
            str[len - 1] = '\0';
            str++;
        }
    }
    return str;
}

/* Frees internal buffers allocated for the parsed representation. */
static inline void bt_ini_destroy(struct bt_ini *ini) {
    if (ini) {
        free(ini->buffer);
        ini->buffer = NULL;
        if (ini->sections) {
            for (int i = 0; i < ini->section_count; i++)
                arrfree(ini->sections[i].entries);
            arrfree(ini->sections);
        }
        ini->section_count = 0;
    }
}

/* Parses the INI content from a string — single pass, stb_ds-backed. */
static inline int bt_ini_parse(struct bt_ini *ini, const char *content) {
    if (!ini || !content) return -1;
    memset(ini, 0, sizeof(struct bt_ini));

    int content_len = strlen(content);
    char *buf = malloc(content_len + 1);
    if (!buf) return -1;
    strcpy(buf, content);

    ini->buffer = buf;
    ini->sections = NULL;

    bt_ini_section_t *cur_sec = NULL;

    char *cursor = buf;
    while (*cursor) {
        char *line_start = cursor;
        while (*cursor && *cursor != '\n' && *cursor != '\r') cursor++;

        char saved = *cursor;
        *cursor = '\0';
        if (saved) {
            *cursor = saved;
            while (*cursor == '\n' || *cursor == '\r') cursor++;
        }

        char *line = bt_ini_trim(line_start);
        if (line[0] == '\0' || line[0] == ';' || line[0] == '#')
            continue;

        if (line[0] == '[') {
            char *cb = bt_ini_strchr(line, ']');
            if (cb) {
                *cb = '\0';
                bt_ini_section_t sec;
                sec.name = bt_ini_trim(line + 1);
                sec.entries = NULL;
                sec.entry_count = 0;
                arrput(ini->sections, sec);
                cur_sec = &ini->sections[arrlen(ini->sections) - 1];
            }
            continue;
        }

        char *eq = bt_ini_strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *key = bt_ini_trim(line);
            char *val = bt_ini_trim(eq + 1);
            bt_ini_strip_comments(val);
            val = bt_ini_trim(val);
            val = bt_ini_trim_quotes(val);

            if (!cur_sec) {
                bt_ini_section_t sec;
                sec.name = "";
                sec.entries = NULL;
                sec.entry_count = 0;
                arrput(ini->sections, sec);
                cur_sec = &ini->sections[arrlen(ini->sections) - 1];
            }

            bt_ini_entry_t entry;
            entry.key = key;
            entry.value = val;
            arrput(cur_sec->entries, entry);
        }
    }

    for (int i = 0; i < arrlen(ini->sections); i++)
        ini->sections[i].entry_count = arrlen(ini->sections[i].entries);
    ini->section_count = arrlen(ini->sections);

    return 0;
}

/* Alias to parse string */
static inline int bt_ini_parse_string(struct bt_ini *ini, const char *content) {
    return bt_ini_parse(ini, content);
}

/* Reads and parses file content */
static inline int bt_ini_parse_file(struct bt_ini *ini, const char *path) {
    if (!ini || !path) return -1;
    unsigned long long size = bt_file_get_size(path);
    if (bt_file_open(path)) {
        char *buf = malloc(size + 1);
        if (!buf) {
            bt_file_close();
            return -1;
        }
        unsigned long long read_bytes = bt_file_read(buf, size);
        buf[read_bytes] = '\0';
        bt_file_close();
        
        int ret = bt_ini_parse(ini, buf);
        free(buf);
        return ret;
    }
    return -1;
}

/* Retrieves a pointer to the section struct by name (case-insensitive) */
static inline const struct bt_ini_section *bt_ini_get_section(const struct bt_ini *ini, const char *section_name) {
    if (!ini) return NULL;
    const char *target = section_name ? section_name : "";
    for (int i = 0; i < ini->section_count; i++) {
        if (stricmp(ini->sections[i].name, target) == 0) {
            return &ini->sections[i];
        }
    }
    return NULL;
}

/* Retrieves a value from a section struct (case-insensitive) */
static inline const char *bt_ini_section_get_value(const struct bt_ini_section *section, const char *key) {
    if (!section || !key) return NULL;
    for (int i = 0; i < section->entry_count; i++) {
        if (stricmp(section->entries[i].key, key) == 0) {
            return section->entries[i].value;
        }
    }
    return NULL;
}

/* Retrieves the value of a key within a section (case-insensitive) */
static inline const char *bt_ini_get_value(const struct bt_ini *ini, const char *section, const char *key) {
    const struct bt_ini_section *sec = bt_ini_get_section(ini, section);
    return bt_ini_section_get_value(sec, key);
}

/* Typo-compatible alias for bt_init_get_value */
static inline const char *bt_init_get_value(const struct bt_ini *ini, const char *section, const char *key) {
    return bt_ini_get_value(ini, section, key);
}

/* Retrieves a string value with a default fallback */
static inline const char *bt_ini_get_string(const struct bt_ini *ini, const char *section, const char *key, const char *default_val) {
    const char *val = bt_ini_get_value(ini, section, key);
    return val ? val : default_val;
}

/* Retrieves a boolean value from a section (accepts true/false/1/0) */
static inline int bt_ini_get_bool(const struct bt_ini *ini, const char *section, const char *key, int default_val) {
    const char *val = bt_ini_get_value(ini, section, key);
    if (!val) return default_val;
    if (stricmp(val, "true") == 0 || strcmp(val, "1") == 0)
        return 1;
    if (stricmp(val, "false") == 0 || strcmp(val, "0") == 0)
        return 0;
    return default_val;
}

/* Simple integer parser (no stdlib dependency) */
static inline int bt_ini_atoi(const char *s) {
    if (!s) return 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    int n = 0;
    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return sign * n;
}

/* Retrieves an integer value from a section */
static inline int bt_ini_get_int(const struct bt_ini *ini, const char *section, const char *key, int default_val) {
    const char *val = bt_ini_get_value(ini, section, key);
    if (!val) return default_val;
    /* Check that the value is entirely numeric (with optional sign) */
    const char *p = val;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '-' || *p == '+') p++;
    if (*p < '0' || *p > '9') return default_val;
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\0') return default_val;
    return bt_ini_atoi(val);
}

/* Retrieves a boolean value from a section struct */
static inline int bt_ini_section_get_bool(const struct bt_ini_section *section, const char *key, int default_val) {
    const char *val = bt_ini_section_get_value(section, key);
    if (!val) return default_val;
    if (stricmp(val, "true") == 0 || strcmp(val, "1") == 0)
        return 1;
    if (stricmp(val, "false") == 0 || strcmp(val, "0") == 0)
        return 0;
    return default_val;
}

/* Retrieves an integer value from a section struct */
static inline int bt_ini_section_get_int(const struct bt_ini_section *section, const char *key, int default_val) {
    const char *val = bt_ini_section_get_value(section, key);
    if (!val) return default_val;
    const char *p = val;
    while (*p == ' ' || *p == '\t') p++;
    if (*p == '-' || *p == '+') p++;
    if (*p < '0' || *p > '9') return default_val;
    while (*p >= '0' && *p <= '9') p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '\0') return default_val;
    return bt_ini_atoi(val);
}

#endif /* BOOTIE_INI_H */
