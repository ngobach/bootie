#include <bootprog.h>

#if defined(__i386__)
#define platform_size_t unsigned long
#else
#define platform_size_t unsigned long long
#endif

// Forward declarations of helper functions
static void set_putchar_hook(void *buf);
static void restore_putchar_hook(platform_size_t val);
static int glob_match(const char *pat, const char *str);

// Entry point gmain() will be called by main() in bootprog.h.
int gmain(int argc, char *argv[], int flags) {

  // Validate arguments (we expect "eachtitle" + 3 arguments, so argc must be >= 4)
  if (argc < 4) {
    printf("Usage: eachtitle <base_dir> <pattern> <callback>\n");
    return ERR_BAD_ARGUMENT;
  }

  char *base_dir = argv[1];
  char *pattern = argv[2];
  char *callback = argv[3];

  // Allocate a buffer to capture print output (128 KB)
  char *buf = malloc(131072);
  if (!buf) {
    printf("Error: failed to allocate memory buffer\n");
    return ERR_NOT_ENOUGH_MEMORY;
  }

  // Clear errnum before invoking grub_dir
  errnum = ERR_NONE;

  // Save the original putchar_hooked pointer
  platform_size_t saved_putchar_hooked = putchar_hooked;

  // Set putchar_hooked to our buffer to capture output
  set_putchar_hook(buf);

  // Call the internal grub_dir function to list the directory
  grub_dir(base_dir);

  // Read the final pointer to get the end of captured data
  char *end_ptr = (char *)putchar_hooked;

  // Restore the original putchar_hooked pointer
  restore_putchar_hook(saved_putchar_hooked);

  // Check if an error occurred during directory listing
  if (errnum != ERR_NONE) {
    printf("Error listing directory '%s' (errnum: %d)\n", base_dir, errnum);
    free(buf);
    return errnum;
  }

  // Null-terminate the captured string
  if (end_ptr >= buf && end_ptr < buf + 131072) {
    *end_ptr = '\0';
  } else {
    buf[131071] = '\0';
  }

  // Parse the captured output, match against pattern, and print formatted
  // titles
  char *p = buf;

  // Skip leading spaces
  while (*p == ' ') {
    p++;
  }

  char *filename_start = p;
  char *write_ptr = p;

  while (*p) {
    if (*p == '\\') {
      p++;
      if (*p) {
        *write_ptr++ = *p++;
      }
    } else if (*p == ' ') {
      *write_ptr = '\0';
      if (write_ptr > filename_start) {
        int name_len = strlen(filename_start);
        // Skip directories (indicated by a trailing slash)
        if (!(name_len > 0 && filename_start[name_len - 1] == '/')) {
          if (glob_match(pattern, filename_start)) {
            printf("title Boot \"%s\"\n", filename_start);
            printf("%s \"%s\"\n", callback, filename_start);
            printf("echo There are problem booting the entry\n");
            printf("pause --wait=10\n");
            printf("reboot\n\n");
          }
        }
      }
      p++;
      while (*p == ' ') {
        p++;
      }
      filename_start = write_ptr = p;
    } else {
      *write_ptr++ = *p++;
    }
  }

  // Handle the last entry
  if (write_ptr > filename_start) {
    *write_ptr = '\0';
    int name_len = strlen(filename_start);
    if (!(name_len > 0 && filename_start[name_len - 1] == '/')) {
      if (glob_match(pattern, filename_start)) {
        printf("title Boot \"%s\"\n", filename_start);
        printf("%s \"%s\"\n", callback, filename_start);
        printf("echo There are problem booting the entry\n");
        printf("pause --wait=10\n");
        printf("reboot\n\n");
      }
    }
  }

  free(buf);
  return 0;
}

// Helper functions to get/set the putchar_hooked pointer
#if defined(__i386__)
static void set_putchar_hook(void *buf) { putchar_hooked = (unsigned long)buf; }
static void restore_putchar_hook(platform_size_t val) {
  putchar_hooked = (unsigned long)val;
}
#else
static void set_putchar_hook(void *buf) {
  putchar_hooked = (unsigned long long)buf;
}
static void restore_putchar_hook(platform_size_t val) {
  putchar_hooked = (unsigned long long)val;
}
#endif

// Case-insensitive wildcard glob matching helper function
static int glob_match(const char *pat, const char *str) {
  if (*pat == '\0') {
    return *str == '\0';
  }
  if (*pat == '*') {
    while (*pat == '*') {
      pat++;
    }
    if (*pat == '\0') {
      return 1;
    }
    while (*str) {
      if (glob_match(pat, str)) {
        return 1;
      }
      str++;
    }
    return 0;
  }
  if (*pat == '?') {
    if (*str == '\0') {
      return 0;
    }
    return glob_match(pat + 1, str + 1);
  }
  if (tolower((unsigned char)*pat) == tolower((unsigned char)*str)) {
    return glob_match(pat + 1, str + 1);
  }
  return 0;
}
