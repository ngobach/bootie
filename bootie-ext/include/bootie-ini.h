#ifndef BOOTIE_INI_H
#define BOOTIE_INI_H

#include <bootie.h>
#include <bootie-io.h>

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
        if (ini->buffer) {
            free(ini->buffer);
            ini->buffer = NULL;
        }
        if (ini->sections) {
            free(ini->sections);
            ini->sections = NULL;
        }
        ini->section_count = 0;
    }
}

/* Parses the INI content from a string. */
static inline int bt_ini_parse(struct bt_ini *ini, const char *content) {
    if (!ini || !content) return -1;
    memset(ini, 0, sizeof(struct bt_ini));

    int content_len = strlen(content);
    char *buf = malloc(content_len + 1);
    if (!buf) {
        return -1;
    }
    strcpy(buf, content);

    /* Pass 1a: Count section headers and determine if there is a global section */
    int section_header_count = 0;
    int has_global = 0;
    
    const char *p = buf;
    while (*p) {
        const char *line_start = p;
        while (*p && *p != '\n' && *p != '\r') {
            p++;
        }
        int line_len = p - line_start;
        while (*p == '\n' || *p == '\r') {
            p++;
        }
        
        const char *line_p = line_start;
        const char *line_end = line_start + line_len;
        while (line_p < line_end && bt_ini_isspace((unsigned char)*line_p)) {
            line_p++;
        }
        
        if (line_p >= line_end || *line_p == ';' || *line_p == '#') {
            continue;
        }
        
        if (*line_p == '[') {
            line_p++;
            const char *close_bracket = line_p;
            while (close_bracket < line_end && *close_bracket != ']') {
                close_bracket++;
            }
            if (close_bracket < line_end && *close_bracket == ']') {
                section_header_count++;
            }
            continue;
        }
        
        const char *eq = line_p;
        while (eq < line_end && *eq != '=') {
            eq++;
        }
        if (eq < line_end && *eq == '=') {
            if (section_header_count == 0) {
                has_global = 1;
            }
        }
    }

    int total_sections = section_header_count + has_global;
    if (total_sections == 0) {
        ini->buffer = buf;
        ini->sections = NULL;
        ini->section_count = 0;
        return 0;
    }

    int *entry_counts = zalloc(total_sections * sizeof(int));
    if (!entry_counts) {
        free(buf);
        return -1;
    }

    /* Pass 1b: Count entries per section */
    p = buf;
    int current_section_headers = 0;
    while (*p) {
        const char *line_start = p;
        while (*p && *p != '\n' && *p != '\r') {
            p++;
        }
        int line_len = p - line_start;
        while (*p == '\n' || *p == '\r') {
            p++;
        }
        
        const char *line_p = line_start;
        const char *line_end = line_start + line_len;
        while (line_p < line_end && bt_ini_isspace((unsigned char)*line_p)) {
            line_p++;
        }
        
        if (line_p >= line_end || *line_p == ';' || *line_p == '#') {
            continue;
        }
        
        if (*line_p == '[') {
            line_p++;
            const char *close_bracket = line_p;
            while (close_bracket < line_end && *close_bracket != ']') {
                close_bracket++;
            }
            if (close_bracket < line_end && *close_bracket == ']') {
                current_section_headers++;
            }
            continue;
        }
        
        const char *eq = line_p;
        while (eq < line_end && *eq != '=') {
            eq++;
        }
        if (eq < line_end && *eq == '=') {
            int curr_idx = has_global ? current_section_headers : (current_section_headers - 1);
            if (curr_idx >= 0 && curr_idx < total_sections) {
                entry_counts[curr_idx]++;
            }
        }
    }

    int total_entries = 0;
    for (int i = 0; i < total_sections; i++) {
        total_entries += entry_counts[i];
    }

    bt_ini_section_t *sections = zalloc(total_sections * sizeof(bt_ini_section_t) + total_entries * sizeof(bt_ini_entry_t));
    if (!sections) {
        free(entry_counts);
        free(buf);
        return -1;
    }

    bt_ini_entry_t *flat_entries = (bt_ini_entry_t *)(sections + total_sections);
    int entry_offset = 0;
    for (int i = 0; i < total_sections; i++) {
        sections[i].entries = &flat_entries[entry_offset];
        sections[i].entry_count = 0;
        sections[i].name = NULL;
        entry_offset += entry_counts[i];
    }

    static const char default_section_name[1] = {0};
    if (has_global) {
        sections[0].name = default_section_name;
    }

    /* Pass 2: Parse in-place and populate structures */
    char *cursor = buf;
    current_section_headers = 0;
    while (*cursor) {
        char *line_start = cursor;
        while (*cursor && *cursor != '\n' && *cursor != '\r') {
            cursor++;
        }
        
        char saved_char = *cursor;
        *cursor = '\0';
        
        char *line = bt_ini_trim(line_start);
        
        if (saved_char != '\0') {
            *cursor = saved_char;
            while (*cursor == '\n' || *cursor == '\r') {
                cursor++;
            }
        }
        
        if (line[0] == '\0' || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        if (line[0] == '[') {
            char *close_bracket = bt_ini_strchr(line, ']');
            if (close_bracket) {
                *close_bracket = '\0';
                char *sec_name = bt_ini_trim(line + 1);
                current_section_headers++;
                int curr_idx = has_global ? current_section_headers : (current_section_headers - 1);
                if (curr_idx >= 0 && curr_idx < total_sections) {
                    sections[curr_idx].name = sec_name;
                }
            }
            continue;
        }
        
        char *eq = bt_ini_strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *key = bt_ini_trim(line);
            char *val = eq + 1;
            
            bt_ini_strip_comments(val);
            val = bt_ini_trim(val);
            val = bt_ini_trim_quotes(val);
            
            int curr_idx = has_global ? current_section_headers : (current_section_headers - 1);
            if (curr_idx >= 0 && curr_idx < total_sections) {
                int ec = sections[curr_idx].entry_count;
                if (ec < entry_counts[curr_idx]) {
                    sections[curr_idx].entries[ec].key = key;
                    sections[curr_idx].entries[ec].value = val;
                    sections[curr_idx].entry_count++;
                }
            }
        }
    }

    free(entry_counts);

    ini->buffer = buf;
    ini->sections = sections;
    ini->section_count = total_sections;

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

#endif /* BOOTIE_INI_H */
