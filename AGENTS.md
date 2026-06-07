# Bare-Metal & Firmware Development Guidelines

This document outlines key technical guidelines, architectural constraints, and debugging practices for developing relocatable bare-metal modules (BIOS VBE and UEFI GOP) within this repository.

---

## 1. Position-Independent Code (PIC) and Data Layout

When compiling code for embedded bootloaders or firmware environments (such as custom GRUB4DOS/GRUB4EFI external programs), the binaries are loaded at arbitrary physical memory addresses. The loading mechanism is lightweight and **does not process dynamic relocations** at runtime.

### The Global Pointer Array Pitfall (Absolute Relocations)
Declaring an array of pointers (like string lists or frames of an animation) using pointer types:
```c
// CRITICAL BUG in un-relocated loader environments
static const char *frames[] = {
    "frame_data_1",
    "frame_data_2"
};
```
creates a table of absolute addresses (pointers) pointing to virtual or compile-time static locations. Since the dynamic relocations are not resolved at runtime, dereferencing `frames[i]` causes immediate page faults, GPFs, or silent crashes.

### The Solution: Multi-Dimensional Char Arrays
To ensure all memory accesses are compiled as instruction-pointer relative (`RIP-relative`), pack data contiguously in value arrays:
```c
// SAFE & POSITION-INDEPENDENT
static const char frames[12][64][64] = {
    { ... },
    { ... }
};
```
This forces the compiler to generate `lea` instruction sequences relative to `%rip` (for x86_64) or basic PC-relative offsets, entirely eliminating absolute relocations.

---

## 2. Compiling for Bare-Metal/Firmware Stacks

### The Red Zone Threat (x86_64)
Under the standard System V ABI for x86_64, a 128-byte region below the stack pointer (`%rsp`) is designated as the **Red Zone**. Compilers optimize function prologues by using this area for local variables without adjusting `%rsp`.

* **The Problem:** In bare-metal or firmware environments, asynchronous hardware interrupts (e.g., timer ticks) or UEFI callback routines can execute on the same active stack. When an interrupt fires, the CPU or interrupt handler pushes registers and the interrupt frame directly below `%rsp`, silently **overwriting and corrupting** any local variables stored in the Red Zone.
* **The Cure:** Every UEFI/x86_64 target compilation must include the `-mno-red-zone` compiler flag to guarantee that `%rsp` is always decremented to reserve stack space for local variables before they are written.

### Calling Conventions: ms_abi vs. sysv_abi
* **UEFI Services** utilize the Microsoft x64 calling convention (`ms_abi`).
* **GCC/Linux** targets default to the System V AMD64 ABI (`sysv_abi`).
* When calling firmware functions or defining callbacks, decorate them with the `EFIAPI` macro (defined as `__attribute__((ms_abi))` on 64-bit systems) to ensure arguments are passed in the correct registers (`%rcx`, `%rdx`, `%r8`, `%r9` for MS ABI vs `%rdi`, `%rsi`, `%rdx`, `%rcx`, `%r8`, `%r9` for System V).

### Explicit BSS Zeroing
Bare-metal binary formats do not guarantee that the loader initializes memory to zero. To ensure global/static variables behave predictably, the entry wrapper must clear the BSS segment manually at startup.

Declare BSS boundaries as hidden symbols in the headers to guarantee RIP-relative address calculation:
```c
__attribute__((visibility("hidden"))) extern char __BSS_START[];
__attribute__((visibility("hidden"))) extern char __BSS_END[];
```
Zeroing loop in `main` (see [bootie.h](file:///Users/bachnx/Projects/super-rescue-disk/bootie-ext/include/bootie.h#L45-L56)):
```c
char *bss = __BSS_START;
char *bss_end = __BSS_END;
while (bss < bss_end) {
    *bss++ = 0;
}
```

---

## 3. Raw Binary Format & Entry Points

When converting ELF object files to raw flat binaries (using `objcopy -O binary`), all symbol tables, headers, and metadata are stripped.

* **Offset 0 Execution:** The loader transfers control directly to the very first byte of the binary image.
* **Linker Scripts:** Linker scripts (`linker_uefi.ld`/`linker_bios.ld`) must dictate the entry point. The `.text` section must start with the entry-point routine (e.g., `*(.text.startup)` or `*(.text)` containing `main`).
* **Source Ordering:** To guarantee safety, either rely on linker script section grouping or ensure the primary entry function (`main` / `gmain`) is compiled/linked first.

---

## 4. Graphics Color Space and Alpha Channels

Firmware interfaces (UEFI GOP and BIOS VBE) do not perform alpha compositing for solid fills or direct pixel manipulation in hardware.

* **UEFI GOP:** A linear 32-bit framebuffer. The pixel format (e.g., `PixelRedGreenBlueReserved8BitPerColor` or custom bitmasks) defines the color channel layouts.
* **BIOS VBE:** Supports 15, 16, 24, or 32 bits per pixel.
* **Alpha Handling:** Write the reserved byte (top 8 bits in a 32-bit pixel) as `0x00` or ignore it. Hardware ignores the alpha channel, treating it as padding. Solid fills and drawing loops write raw XRGB / XBGR values directly into the mapped framebuffer.

---

## 5. QEMU Debugging Practices

To verify bare-metal behavior and trace crashes without flashing hardware, utilize QEMU connected to debuggers.

### 1. Launching QEMU with Debug Hooks
Run QEMU with the GDB stub enabled (`-s`) and optionally frozen at boot (`-S`):
```bash
# Run BIOS target
qemu-system-x86_64 -s -S -m 2G -drive file=runner/scratch.img,format=raw -drive file=fat:rw:runner/data-part,format=raw

# Run UEFI target (specify OVMF bios image)
qemu-system-x86_64 -s -S -bios resources/OVMF.fd -m 2G -drive file=runner/scratch.img,format=raw -drive file=fat:rw:runner/data-part,format=raw
```

### 2. Connecting a Debugger (GDB / LLDB)

#### LLDB (Standard on macOS):
```bash
lldb
(lldb) gdb-remote 1234
```

#### GDB:
```bash
gdb
(gdb) target remote :1234
```

### 3. Loading Debug Symbols
Since raw binaries do not contain symbols, you must load symbols from the compiled ELF output file (`.o` or intermediate target) at the correct runtime load address.
* Identify the load address from the loader output (or linker mapping).
* In GDB:
  ```gdb
  add-symbol-file mod/uefi/nyan.o 0x<load-address>
  ```
* In LLDB:
  ```lldb
  target modules load --file mod/uefi/nyan.o --slide 0x<load-address>
  ```
