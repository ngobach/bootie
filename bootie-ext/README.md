# GRUB4DOS / GRUB4EFI Unified API Reference

This document provides a comprehensive developer reference for the unified system variables, function tables, and structures exposed by the **GRUB4DOS** and **GRUB4EFI** bootloaders. 

External modules compiled with `bootprog.h` run bare-metal without standard runtime libraries. Instead, they interact with the host bootloader environment by mapping function pointer macros to the system dispatch table at fixed offset arrays.

---

## 1. Global System Variables

These macros map directly to fixed physical bootloader memory addresses (e.g., `0x8280`–`0x835C`) to query or modify the bootloader's active state:

| Variable | Type / Address | Description |
| :--- | :--- | :--- |
| `boot_drive` | `unsigned long long` | The index of the drive that GRUB booted from (e.g., `0x80` for HDD, `0x00` for Floppy). |
| `current_drive` | `unsigned long` | Active drive index currently being accessed. |
| `current_partition` | `unsigned long` | Active partition index within the current drive. |
| `current_slice` | `unsigned long` | Active partition table slice entry. |
| `filesize` | `unsigned long long` | Size of the last opened file in bytes. |
| `filepos` | `unsigned long long` | Current byte offset pointer inside the opened file. |
| `filemax` | `unsigned long long` | Upper boundary/size allocation of the open file buffer. |
| `free_mem_start` | `unsigned long` | Start physical address of free conventional memory available for heap use. |
| `free_mem_end` | `unsigned long` | End physical address of free conventional memory. |
| `errnum` | `grub_error_t` | Global error number holding the active execution status code. |
| `debug` | `int` | Debug printing state (`1` to enable verbose bootloader output, `0` to disable). |
| `grub_timeout` | `int` | Global menu countdown timeout variable. |
| `pxe_yip` / `pxe_sip` / `pxe_gip` | `unsigned long` | PXE network configuration: Your IP, Server IP, Gateway IP. |

---

## 2. Console and I/O APIs

Basic input, output, and terminal control functions:

### Output
*   **`printf(const char *format, ...)`**
    *   Formated printing to console. Under the hood, this compiles to `sprintf(NULL, format, ...)`.
*   **`putstr(const char *str)`**
    *   Prints a raw string to the console.
*   **`putchar(int c)`**
    *   Prints a single character.
*   **`cls(void)`**
    *   Clears the terminal screen.
*   **`gotoxy(int x, int y)`**
    *   Moves the cursor to column `x`, row `y`.
*   **`getxy(void)`**
    *   Returns the current cursor position formatted as `(X << 8) \| Y`.

### Input
*   **`getkey(void)`**
    *   Blocks until a key is pressed, returning its ASCII code/BIOS scan code.
*   **`checkkey(void)`**
    *   Checks if a keyboard keypress is waiting in the input buffer (non-blocking). Returns `1` if a key is available, `0` otherwise.

---

## 3. String & Memory Utilities

Standard utility functions optimized for self-contained binaries:

### String Handling
*   **`strlen(const char *str)`** -> Returns the string length.
*   **`strcpy(char *dest, const char *src)`** -> Copies a string to destination.
*   **`strncat(char *s1, const char *s2, int n)`** -> Concatenates up to `n` characters.
*   **`strstr(const char *s1, const char *s2)`** -> Finds substring occurrences.
*   **`strtok(char *s, const char *delim)`** -> Tokenizes string `s` using delimiters.
*   **`tolower(int c)`** -> Converts a character to lowercase.
*   **`isspace(int c)`** -> Returns `1` if character `c` is whitespace.

### String Comparisons (`strncmpx` backend)
*   **`strcmp(s1, s2)`** -> Compares two strings.
*   **`strncmp(s1, s2, n)`** -> Compares first `n` characters of two strings.
*   **`stricmp(s1, s2)`** -> Case-insensitive string comparison.
*   **`strnicmp(s1, s2, n)`** -> Case-insensitive comparison of first `n` characters.

### Raw Memory Operations
*   **`memset(void *start, int c, int len)`** -> Fills memory segment with byte `c`.
*   **`memmove(void *to, const void *from, int len)`** -> Safe overlapping memory copy.
*   **`memcmp(const char *s1, const char *s2, int n)`** -> Compares `n` bytes of memory.
*   **`malloc(int size)`** -> Allocates a memory block on the bootloader heap.
*   **`free(void *ptr)`** -> Frees a previously allocated heap block.

---

## 4. File and Device I/O APIs

Read/write operations on mounted filesystems and raw disk sectors:

*   **`open(char *filename)`**
    *   Opens a file (e.g., `(hd0,0)/boot/grub/menu.lst`). Returns `1` on success, `0` on failure. Updates `filesize`.
*   **`read(unsigned long long buf, unsigned long long len, unsigned long write)`**
    *   Reads `len` bytes from the open file into `buf`. If `write` is `GRUB_WRITE`, it performs a write operation instead (supported on some filesystems/memory drives).
*   **`close(void)`**
    *   Closes the currently open file.
*   **`devread(unsigned long long sector, unsigned long long byte_offset, unsigned long long byte_len, unsigned long long buf, unsigned long write)`**
    *   Low-level read of partition/drive relative sectors.
*   **`devwrite(unsigned long long sector, unsigned long long sector_len, unsigned long long buf)`**
    *   Writes raw sectors directly to the partition.
*   **`rawread` / `rawwrite`**
    *   Reads/writes raw physical disk sectors (bypassing partition offset mapping).

---

## 5. Command Execution and Shell integration

Functions to execute built-in commands or interact with the environment:

*   **`run_line(char *line, int flags)`**
    *   Executes a full GRUB4DOS command line string (e.g. `"map --mem /win.iso (hd32)"`).
*   **`builtin_cmd(char *cmd, char *arg, int flags)`**
    *   Runs a GRUB built-in command directly by name (e.g. `builtin_cmd("kernel", "/vmlinuz", 0)`).
*   **`envi_cmd(const char *var, char *const env, int flags)`**
    *   Gets, sets, or manages variables inside the GRUB4DOS command environment.

---

## 6. BIOS and Real Mode Calling (BIOS Mode Only)

*   **`realmode_run(long regs_ptr)`**
    *   Suspends 32-bit protected-mode execution, switches the processor to 16-bit real-mode, and calls BIOS software interrupts (configured in a register structure). This enables raw access to hardware interrupts, video modes, and drive controllers.

---

## 7. Structure Definitions

### Disk Geometry (`struct geometry`)
Used to retrieve CHS (Cylinder/Head/Sector) properties and total sector capacity:
```c
struct geometry
{
  unsigned long cylinders;
  unsigned long heads;
  unsigned long sectors;
  unsigned long long total_sectors;
  unsigned long sector_size;
  unsigned long flags;
};
```

### Drive Map Slot (`struct drive_map_slot`)
Used to configure disk redirection/mapping properties:
```c
struct drive_map_slot
{
  unsigned char from_drive;
  unsigned char to_drive;
  unsigned char max_head;
  unsigned char max_sector;
  unsigned short to_cylinder;
  unsigned char to_head;
  unsigned char to_sector;
  unsigned long long start_sector;
  unsigned long long sector_count;
};
```
