#include <bootprog.h>

#if !defined(__i386__)
  #define platform_size_t unsigned long long
#else
  #define platform_size_t unsigned long
#endif

// Forward declarations
static void delay_ms(unsigned int ms);

// Entry point main() must be the very first function defined in the file.
int main(char *arg, int flags) {
  INIT_BOOT_MODULE();

  // Save original color state
  unsigned long long saved_color_64 = current_color_64bit;
  unsigned int saved_color = current_color;

  char *colors[] = {
    "red/red",
    "green/green",
    "blue/blue",
    "black/black",
    "white/white"
  };

  int exit_early = 0;

  for (int i = 0; i < 5; i++) {
    // Set color to the current test color
    builtin_cmd("color", colors[i], 0);
    cls();

    // Delay 2 seconds, but allow user to skip or exit
    unsigned int elapsed = 0;
    while (elapsed < 2000) {
      delay_ms(50);
      elapsed += 50;
      if (checkkey()) {
        int key = getkey();
        if (key == 27 || key == 0x1b) {
          exit_early = 1;
        }
        break; // skip to next color (or exit if Esc)
      }
    }

    if (exit_early) {
      break;
    }
  }

  // Restore original color state
  current_color_64bit = saved_color_64;
  current_color = saved_color;
  if (current_term->SETCOLOR) {
    unsigned long long colors_buf[3] = {saved_color_64, saved_color_64, saved_color_64};
    current_term->SETCOLOR(3, colors_buf);
  }
  cls();

  return 0;
}

static void delay_ms(unsigned int ms) {
#if defined(__i386__)
  // 1 tick = 55 milliseconds on BIOS system timer
  unsigned int ticks = (ms + 54) / 55;
  unsigned long start = *(volatile unsigned long *)0x46c;
  while ((*(volatile unsigned long *)0x46c - start) < ticks) {
    // busy wait
  }
#else
  // UEFI mode: Use BootServices->Stall
  efi_system_table_t *st = (efi_system_table_t *)grub_efi_system_table;
  if (st && st->boot_services) {
    st->boot_services->stall(ms * 1000); // stall expects microseconds
  } else {
    // fallback delay loop
    volatile unsigned long long i;
    for (i = 0; i < ms * 100000ULL; i++) {
      // noop
    }
  }
#endif
}
