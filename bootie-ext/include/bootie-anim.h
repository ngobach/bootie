#ifndef BOOTIE_ANIM_H
#define BOOTIE_ANIM_H

#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Easing functions (math-free subset, safe for bare-metal)           */
/* ------------------------------------------------------------------ */
typedef float (*bt_ease_fn)(float t);

float bt_ease_linear(float t);
float bt_ease_in_quad(float t);
float bt_ease_out_quad(float t);
float bt_ease_in_out_quad(float t);
float bt_ease_in_cubic(float t);
float bt_ease_out_cubic(float t);
float bt_ease_in_out_cubic(float t);
float bt_ease_in_quart(float t);
float bt_ease_out_quart(float t);
float bt_ease_in_out_quart(float t);
float bt_ease_in_quint(float t);
float bt_ease_out_quint(float t);
float bt_ease_in_out_quint(float t);
float bt_ease_out_bounce(float t);
float bt_ease_in_bounce(float t);
float bt_ease_in_out_bounce(float t);

/* ------------------------------------------------------------------ */
/*  Color helpers                                                      */
/* ------------------------------------------------------------------ */
struct bt_color {
    uint8_t r, g, b, a;
};

/* Linear interpolation between two colors */
static inline void bt_color_lerp(struct bt_color *out,
                                  struct bt_color a, struct bt_color b,
                                  float t) {
    if (t <= 0.0f) { *out = a; return; }
    if (t >= 1.0f) { *out = b; return; }
    out->r = (uint8_t)((float)a.r + (float)(b.r - a.r) * t);
    out->g = (uint8_t)((float)a.g + (float)(b.g - a.g) * t);
    out->b = (uint8_t)((float)a.b + (float)(b.b - a.b) * t);
    out->a = (uint8_t)((float)a.a + (float)(b.a - a.a) * t);
}

/* ------------------------------------------------------------------ */
/*  Simple tween: drives a float from → to with easing over duration  */
/* ------------------------------------------------------------------ */
struct bt_tween {
    float elapsed;
    float duration;
    bt_ease_fn ease;
    float *target;
    float from;
    float to;
    uint8_t done;
};

static inline void bt_tween_start(struct bt_tween *t, float *target,
                                   float from, float to,
                                   float duration_ms, bt_ease_fn ease) {
    t->elapsed = 0.0f;
    t->duration = duration_ms;
    t->ease = ease;
    t->target = target;
    t->from = from;
    t->to = to;
    t->done = 0;
}

/* Advance tween by dt (ms). Returns 1 while running, 0 when done. */
static inline int bt_tween_tick(struct bt_tween *t, float dt) {
    if (t->done) return 0;
    t->elapsed += dt;
    if (t->elapsed >= t->duration) {
        *t->target = t->to;
        t->done = 1;
        return 0;
    }
    float p = t->elapsed / t->duration;
    float v = t->ease ? t->ease(p) : p;
    *t->target = t->from + (t->to - t->from) * v;
    return 1;
}

static inline float bt_tween_value(struct bt_tween *t) {
    return *t->target;
}

/* ------------------------------------------------------------------ */
/*  Timeline: tracks elapsed time.                                    */
/*  Use a fixed dt per frame (e.g. 16 ms for ~60 fps).                */
/* ------------------------------------------------------------------ */
struct bt_timeline {
    float elapsed;
};

static inline void bt_timeline_init(struct bt_timeline *tl) {
    tl->elapsed = 0.0f;
}

static inline void bt_timeline_advance(struct bt_timeline *tl, float dt) {
    tl->elapsed += dt;
}

/* ------------------------------------------------------------------ */
/*  Easing implementation                                              */
/* ------------------------------------------------------------------ */
float bt_ease_linear(float t) { return t; }
float bt_ease_in_quad(float t) { return t * t; }
float bt_ease_out_quad(float t) { return -t * (t - 2.0f); }
float bt_ease_in_out_quad(float t) {
    if (t < 0.5f) return 2.0f * t * t;
    return -2.0f * t * t + 4.0f * t - 1.0f;
}
float bt_ease_in_cubic(float t) { return t * t * t; }
float bt_ease_out_cubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}
float bt_ease_in_out_cubic(float t) {
    if (t < 0.5f) return 4.0f * t * t * t;
    float f = 2.0f * t - 2.0f;
    return 0.5f * f * f * f + 1.0f;
}
float bt_ease_in_quart(float t) { return t * t * t * t; }
float bt_ease_out_quart(float t) {
    float f = t - 1.0f;
    return -(f * f * f * f) + 1.0f;
}
float bt_ease_in_out_quart(float t) {
    if (t < 0.5f) return 8.0f * t * t * t * t;
    float f = t - 1.0f;
    return -8.0f * f * f * f * f + 1.0f;
}
float bt_ease_in_quint(float t) { return t * t * t * t * t; }
float bt_ease_out_quint(float t) {
    float f = t - 1.0f;
    return f * f * f * f * f + 1.0f;
}
float bt_ease_in_out_quint(float t) {
    if (t < 0.5f) return 16.0f * t * t * t * t * t;
    float f = 2.0f * t - 2.0f;
    return 0.5f * f * f * f * f * f + 1.0f;
}
float bt_ease_out_bounce(float t) {
    if (t < 4.0f/11.0f) return (121.0f * t * t) / 16.0f;
    if (t < 8.0f/11.0f) return (363.0f/40.0f * t * t) - (99.0f/10.0f * t) + 17.0f/5.0f;
    if (t < 9.0f/10.0f) return (4356.0f/361.0f * t * t) - (35442.0f/1805.0f * t) + 16061.0f/1805.0f;
    return (54.0f/5.0f * t * t) - (513.0f/25.0f * t) + 268.0f/25.0f;
}
float bt_ease_in_bounce(float t) { return 1.0f - bt_ease_out_bounce(1.0f - t); }
float bt_ease_in_out_bounce(float t) {
    if (t < 0.5f) return 0.5f * bt_ease_in_bounce(t * 2.0f);
    return 0.5f * bt_ease_out_bounce(t * 2.0f - 1.0f) + 0.5f;
}

#endif /* BOOTIE_ANIM_H */
