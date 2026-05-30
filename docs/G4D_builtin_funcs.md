# GRUB4DOS Built-in System Functions (`Fn.X`)

In **GRUB4DOS**, dynamic batch scripts (starting with the `!BAT` header) can call internal system functions using the `call Fn.X` syntax. 

The index `X` in `Fn.X` corresponds directly to the offset in the core GRUB4DOS system functions table at memory address `0x8300` (defined in the GRUB4DOS SDK header `grub4dos.h`).

Below is the complete, documented list of all available system functions, their arguments, and their descriptions:

| Fn Index | Function Name | Arguments | Description |
| :--- | :--- | :--- | :--- |
| **`Fn.0`** | `sprintf` | `char *buf`, `const char *fmt`, `...` | Formatted output to a buffer. Equivalent to `printf` when `buf` is `NULL`. |
| **`Fn.1`** | `putstr` | `const char *str` | Outputs a string to the console. |
| **`Fn.2`** | `putchar` | `int c` | Outputs a single character `c` to the console. |
| **`Fn.3`** | `get_cmdline_obsolete` | `struct get_cmdline_arg cmdline` | Command line reading (obsolete). |
| **`Fn.4`** | `getxy` | `void` | Gets the cursor position. Returns coordinates in the format `(X << 8) \| Y`. |
| **`Fn.5`** | `gotoxy` | `int x`, `int y` | Moves the console cursor to coordinates `(x, y)`. |
| **`Fn.6`** | `cls` | `void` | Clears the console screen. |
| **`Fn.7`** | `wee_skip_to` | `char *s`, `int c` | Skip characters in a string up to character `c` (used by Wee compiler/parser). |
| **`Fn.8`** | `nul_terminate` | `char *s` | Replaces trailing whitespace/characters to null-terminate a string. |
| **`Fn.9`** | `safe_parse_maxint_with_suffix` | `char **str_ptr`, `unsigned long long *myint_ptr`, `int unitshift` | Safely parses a 64-bit integer from a string, parsing suffixes (e.g. `K`, `M`, `G`). |
| **`Fn.10`** | `substring` | `const char *s1`, `const char *s2`, `int case_insensitive` | Checks if `s2` is a substring of `s1`. |
| **`Fn.11`** | `strstr` | `const char *s1`, `const char *s2` | Standard C `strstr`: finds the first occurrence of `s2` in `s1`. |
| **`Fn.12`** | `strlen` | `const char *str` | Standard C `strlen`: returns the length of string `str`. |
| **`Fn.13`** | `strtok` | `char *s`, `const char *delim` | Standard C `strtok`: tokenizes a string `s` using delimiters `delim`. |
| **`Fn.14`** | `strncat` | `char *s1`, `const char *s2`, `int n` | Standard C `strncat`: appends at most `n` characters of `s2` onto `s1`. |
| **`Fn.16`** | `strcpy` | `char *dest`, `const char *src` | Standard C `strcpy`: copies string `src` to `dest`. |
| **`Fn.19`** | `getkey` | `void` | Blocks and waits for a single keyboard key press, returning its key code. |
| **`Fn.20`** | `checkkey` | `void` | Non-blocking check for keyboard input. Returns non-zero if a key is pending in buffer. |
| **`Fn.22`** | `memcmp` | `const char *s1`, `const char *s2`, `int n` | Standard C `memcmp`: compares first `n` bytes of `s1` and `s2`. |
| **`Fn.23`** | `memmove` | `void *to`, `const void *from`, `int len` | Standard C `memmove`: copies `len` bytes from `from` to `to`, supporting overlap. |
| **`Fn.24`** | `memset` | `void *start`, `int c`, `int len` | Standard C `memset`: fills `len` bytes of memory starting at `start` with byte `c`. |
| **`Fn.26`** | `open` | `char *path` | Opens a file at `path` on the active device. Returns `1` on success, `0` on error. |
| **`Fn.27`** | `read` | `unsigned long long buf`, `unsigned long long len`, `unsigned long write` | Reads `len` bytes from the open file into `buf` (or writes if `write` is true). |
| **`Fn.28`** | `close` | `void` | Closes the currently open file. |
| **`Fn.31`** | `disk_read_hook` | `void (**hook)(unsigned long long, unsigned long long, unsigned long)` | Gets/Sets the system hook called during low-level disk sector reads. |
| **`Fn.32`** | `devread` | `unsigned long long sector`, `unsigned long long byte_offset`, `unsigned long long byte_len`, `unsigned long long buf`, `unsigned long write` | Reads/writes sectors from the currently open virtual partition or device. |
| **`Fn.33`** | `devwrite` | `unsigned long long sector`, `unsigned long long sector_len`, `unsigned long long buf` | Writes sectors to the currently active virtual partition or device. |
| **`Fn.34`** | `next_partition` | `void` | Iterates to the next partition structure on the current disk. |
| **`Fn.35`** | `open_device` | `void` | Opens and initializes the active drive/device. |
| **`Fn.36`** | `real_open_partition` | `int part` | Opens a specific partition number `part` on the active drive. |
| **`Fn.37`** | `set_device` | `char *device_name` | Selects and activates a device by its GRUB name (e.g., `(hd0,0)`). |
| **`Fn.38`** | `run_line` | `char *heap`, `int flags` | Executes a single raw GRUB4DOS command line string. |
| **`Fn.41`** | `parse_string` | `char *str` | Parses and executes a block of commands (possibly multi-line). |
| **`Fn.42`** | `hexdump` | `unsigned long long addr`, `char *buf`, `int len` | Hex dumps `len` bytes from memory address `addr` to console or `buf`. |
| **`Fn.43`** | `skip_to` | `int after_equal`, `char *cmdline` | Parses argument lines, skipping to the value after `=` if `after_equal` is true. |
| **`Fn.44`** | `builtin_cmd` | `char *cmd`, `char *arg`, `int flags` | Invokes a built-in GRUB4DOS command structure directly by its name `cmd`. |
| **`Fn.45`** | `get_datetime` | `unsigned long *date`, `unsigned long *time` | Reads current system CMOS Real-Time Clock date and time. |
| **`Fn.46`** | `find_command` | `char *cmd` | Resolves a command string `cmd` to its underlying `struct builtin` pointer. |
| **`Fn.47`** | `get_mmap_entry` | `char *buf`, `int index` | Fills `buf` with the BIOS INT15 E820 system memory map entry at `index`. |
| **`Fn.49`** | `zalloc` | `int size` | Allocates `size` bytes of zero-initialized memory from the internal heap. |
| **`Fn.50`** | `malloc` | `int size` | Allocates `size` bytes of memory from the internal heap. |
| **`Fn.51`** | `free` | `void *ptr` | Frees an allocated memory pointer back to the heap. |
| **`Fn.53`** | `realmode_run` | `long regs_ptr` | Invokes real-mode interrupt or BIOS routine using register values in `regs_ptr`. |
| **`Fn.61`** | `grub_dir` | `char *path` | Lists the files in `path` on the active filesystem. |
| **`Fn.62`** | `print_a_completion` | `char *s`, `int len` | Prints a tab-completion option. |
| **`Fn.63`** | `print_completions` | `int a`, `int b` | Prints a column-formatted list of completions. |
| **`Fn.64`** | `lba_to_chs` | `unsigned long lba`, `unsigned long *cl`, `unsigned long *ch`, `unsigned long *dh` | Converts LBA block address to Cylinder, Head, Sector coordinates. |
| **`Fn.65`** | `probe_bpb` | `struct master_and_dos_boot_sector *BS` | Probes FAT/NTFS BIOS Parameter Block (BPB) parameters from sector `BS`. |
| **`Fn.66`** | `probe_mbr` | `struct master_and_dos_boot_sector *BS`, `unsigned long start_sector1`, `unsigned long sector_count1`, `unsigned long part_start1` | Probes MBR partition records from boot sector `BS`. |
| **`Fn.67`** | `unicode_to_utf8` | `unsigned short *src`, `unsigned char *dest`, `unsigned long len` | Converts `len` characters of UTF-16 Unicode `src` to UTF-8 `dest`. |
| **`Fn.68`** | `rawread` | `unsigned long drive`, `unsigned long long sector`, `unsigned long byte_offset`, `unsigned long long byte_len`, `unsigned long long buf`, `unsigned long write` | Low-level direct read from `sector` on `drive` without partition translation. |
| **`Fn.69`** | `rawwrite` | `unsigned long drive`, `unsigned long sector`, `char *buf` | Low-level direct write of `sector` to `drive` without partition translation. |
| **`Fn.70`** | `setcursor` | `int on` | Configures console cursor state. `call Fn.70 0` hides cursor; `call Fn.70 1` shows it. |
| **`Fn.71`** | `tolower` | `int c` | Converts character `c` to lowercase. |
| **`Fn.72`** | `isspace` | `int c` | Checks if character `c` is a whitespace character (spaces, tabs, newlines). |
| **`Fn.73`** | `sleep` | `unsigned int seconds` | Pauses execution for `seconds` duration. |
| **`Fn.74`** | `mem64` | `int mode`, `unsigned long long dest`, `unsigned long long src`, `unsigned long long len` | Copies or fills memory blocks inside the 64-bit high memory address space. |
| **`Fn.75`** | `envi_cmd` | `const char *var`, `char *const env`, `int flags` | Handles environment variable retrieval and manipulation. |
| **`Fn.76`** | `strncmpx` | `const char *s1`, `const char *s2`, `unsigned long n`, `int case_insensitive` | Performs advanced string comparison with case-sensitivity and length limit controls. |
| **`Fn.77`** | `rectangle` | `int left`, `int top`, `int length`, `int width`, `int line` | Draws a graphical rectangle on screen at coordinates `(left, top)`. |
| **`Fn.78`** | `get_cmdline` | `void` | Interactively prompts and retrieves a line of keyboard input from the user. |
