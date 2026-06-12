#include <bootie.h>
#include <string.h>
#include <bootie-gfx.h>
#include <bootie-anim.h>
#include <stdint.h>

#define DELAY_MS        100.0f
#define STAGGER_MS      80.0f
#define CHAR_DUR_MS     500.0f
#define FIXED_DT        16.0f
#define FONT_SZ         48

struct bt_color bg_color   = { 15,  15,  30, 0 };
struct bt_color white      = {255, 255, 255, 255};
struct bt_color final_clr  = {200, 200, 255, 255};

struct char_anim {
    float t;
    struct bt_color color;
};

static float char_start_ms(int idx) {
    return DELAY_MS + idx * STAGGER_MS;
}

static float char_duration_ms(void) {
    return CHAR_DUR_MS;
}

static void update_char(struct char_anim *ca, float elapsed, int idx) {
    float start = char_start_ms(idx);
    if (elapsed < start) {
        ca->t = 0.0f;
        ca->color = bg_color;
        return;
    }
    float raw = (elapsed - start) / char_duration_ms();
    ca->t = raw > 1.0f ? 1.0f : raw;

    if (ca->t <= 0.0f) {
        ca->color = bg_color;
    } else if (ca->t < 0.7f) {
        float p = bt_ease_out_cubic(ca->t / 0.7f);
        bt_color_lerp(&ca->color, bg_color, white, p);
    } else {
        float p = bt_ease_out_cubic((ca->t - 0.7f) / 0.3f);
        bt_color_lerp(&ca->color, white, final_clr, p);
    }
}

int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g))
        return 0;
    gfx_font_load();

    uint32_t W = gfx_width(&g);
    uint32_t H = gfx_height(&g);

    const char *text = "Bootie!";
    int text_len = strlen(text);
    float total_w = (float)gfx_text_width(text, FONT_SZ);
    int ty = ((int)H - FONT_SZ) / 2;

    float elapsed = 0.0f;
    int done = 0;
    int skip = 0;

    while (!skip) {
        while (gfx_checkkey(&g)) {
            gfx_getkey(&g);
            if (done) {
                skip = 1;
                break;
            } else {
                elapsed = DELAY_MS + text_len * STAGGER_MS + CHAR_DUR_MS;
                done = 1;
            }
        }
        if (skip) break;

        if (!done) {
            elapsed += FIXED_DT;
            if (elapsed >= DELAY_MS + text_len * STAGGER_MS + CHAR_DUR_MS) {
                done = 1;
            }
        }

        gfx_backbuffer_begin(&g);
        fill_rect(&g, 0, 0, W, H, bg_color.r, bg_color.g, bg_color.b);

        if (elapsed > DELAY_MS) {
            struct char_anim ca;
            char ch[2] = {0};
            char prefix[32];
            int tx = ((int)W - (int)total_w) / 2;

            for (int i = 0; i < text_len; i++) {
                update_char(&ca, elapsed, i);
                ch[0] = text[i];
                memcpy(prefix, text, i);
                prefix[i] = '\0';
                int cx = tx + gfx_text_width(prefix, FONT_SZ);
                draw_str(&g, cx, ty, ch,
                         ca.color.r, ca.color.g, ca.color.b, FONT_SZ);
            }
        }

        if (done) {
            const char *hint = "Press any key to continue";
            int hw = gfx_text_width(hint, 16);
            int hx = ((int)W - hw) / 2;
            draw_str(&g, hx, ty + 72, hint, 150, 150, 180, 16);
        }

        gfx_backbuffer_end(&g);
        gfx_delay_ms(&g, FIXED_DT);
    }

    gfx_close(&g);
    return 0;
}
