#include <bootie.h>
#include <bootie-io.h>
#include <stdint.h>
#include <string.h>

int gmain(int argc, char *argv[], int flags) {
    (void)flags;

    const char *path = (argc >= 2) ? argv[1] : "/";

    printf("test_ls: listing '%s'\n\n", path);

    struct bt_dir_entry *list = bt_directory_list(path);
    if (!list) {
        printf("error: bt_directory_list failed\n");
        return 1;
    }

    int count = 0;
    for (struct bt_dir_entry *p = list; p->name; p++) {
        printf("  %c  %s\n", p->is_dir ? 'D' : 'F', p->name);
        count++;
        free(p->name);
    }
    free(list);
    printf("\n%d entries\n", count);
    return 0;
}
