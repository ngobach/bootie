#include <bootie-ini.h>
#include <bootie.h>

int gmain(int argc, char *argv[], int flags) {
  (void)argc;
  (void)argv;
  (void)flags;

  struct bt_ini ini;
  printf("Parsing /test.ini...\n");

  if (bt_ini_parse_file(&ini, "/test.ini") != 0) {
    printf("Error: failed to open or parse /test.ini\n");
    return 1;
  }

  printf("Successfully parsed /test.ini. Sections count: %d\n\n",
         ini.section_count);

  for (int i = 0; i < ini.section_count; i++) {
    const char *sec_name = ini.sections[i].name;
    if (sec_name && sec_name[0] == '\0') {
      printf("[global]\n");
    } else {
      printf("[%s]\n", sec_name);
    }

    for (int j = 0; j < ini.sections[i].entry_count; j++) {
      printf("- %s = %s\n", ini.sections[i].entries[j].key,
             ini.sections[i].entries[j].value);
    }
    printf("\n");
  }

  bt_ini_destroy(&ini);
  return 0;
}
