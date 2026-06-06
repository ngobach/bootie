#include <bootprog.h>
#include <stdint.h>
#include <gfx.h>

#define GRID_COLS 40
#define GRID_ROWS 30
#define CELL_SIZE 12
#define MAX_SNAKE 512

struct point {
    int x;
    int y;
};

/* Forward declarations of helper functions */
static void seed_rand(uint32_t s);
static uint32_t next_rand(void);
static void place_food(struct point *food, const struct point *snake, int len);
static void draw_cell(struct gfx *ctx, int cx, int cy, uint8_t r, uint8_t g, uint8_t b, int x_off, int y_off);
static void draw_border(struct gfx *ctx, int x_off, int y_off);

/* Main game execution loop (must be the first function defined in the C file) */
int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g)) {
        return 0;
    }

    uint32_t W = gfx_width(&g);
    uint32_t H = gfx_height(&g);

    /* Center of play area on screen */
    int grid_w = GRID_COLS * CELL_SIZE;
    int grid_h = GRID_ROWS * CELL_SIZE;
    int x_off  = (W - grid_w) / 2;
    int y_off  = (H - grid_h) / 2 + 10;

    /* Start screen */
    fill_rect(&g, 0, 0, W, H, 10, 10, 15);
    draw_border(&g, x_off, y_off);

    const char *title = "Bare-metal Snake";
    draw_str(&g, (W - 16 * 6 * 2) / 2, y_off + grid_h / 3, title, 50, 220, 50, 2);
    
    const char *prompt = "Press any key to start...";
    draw_str(&g, (W - 25 * 6) / 2, y_off + grid_h / 2, prompt, 200, 200, 200, 1);

    /* Generate a highly random seed based on keypress latency */
    uint32_t seed = 0;
    while (!gfx_checkkey(&g)) {
        seed++;
        gfx_delay_ms(&g, 1);
    }
    gfx_getkey(&g); /* consume the pressed key */

    /* Add the BIOS system tick counter to the seed for extra entropy */
    seed += *(volatile unsigned long *)0x46c;
    seed_rand(seed);

    /* Game State variables */
    struct point snake[MAX_SNAKE];
    int snake_len;
    struct point food;
    int dx, dy;     /* current moving direction */
    int ndx, ndy;   /* next direction input */
    int score;
    int game_over;
    int speed_delay;

    while (1) {
        /* Initialize/Reset game variables */
        snake_len = 4;
        snake[0].x = GRID_COLS / 2;     snake[0].y = GRID_ROWS / 2;
        snake[1].x = GRID_COLS / 2 - 1; snake[1].y = GRID_ROWS / 2;
        snake[2].x = GRID_COLS / 2 - 2; snake[2].y = GRID_ROWS / 2;
        snake[3].x = GRID_COLS / 2 - 3; snake[3].y = GRID_ROWS / 2;

        dx = 1;  dy = 0;
        ndx = 1; ndy = 0;
        score = 0;
        game_over = 0;
        speed_delay = 140;

        place_food(&food, snake, snake_len);

        /* Main Game Loop */
        while (!game_over) {
            /* Input processing */
            while (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { /* ESC to quit game */
                    gfx_close(&g);
                    return 0;
                }
                
                /* Handle controls (Arrow keys and WASD) */
                if ((key == 0x4800 || key == 'w' || key == 'W') && dy == 0) {
                    ndx = 0; ndy = -1;
                } else if ((key == 0x5000 || key == 's' || key == 'S') && dy == 0) {
                    ndx = 0; ndy = 1;
                } else if ((key == 0x4B00 || key == 'a' || key == 'A') && dx == 0) {
                    ndx = -1; ndy = 0;
                } else if ((key == 0x4D00 || key == 'd' || key == 'D') && dx == 0) {
                    ndx = 1; ndy = 0;
                }
            }

            /* Apply next direction */
            dx = ndx;
            dy = ndy;

            /* Compute new head position */
            struct point new_head;
            new_head.x = snake[0].x + dx;
            new_head.y = snake[0].y + dy;

            /* Check wall collision */
            if (new_head.x < 0 || new_head.x >= GRID_COLS || new_head.y < 0 || new_head.y >= GRID_ROWS) {
                game_over = 1;
                break;
            }

            /* Check self collision */
            for (int i = 0; i < snake_len; i++) {
                if (new_head.x == snake[i].x && new_head.y == snake[i].y) {
                    game_over = 1;
                    break;
                }
            }
            if (game_over) break;

            /* Check food consumption */
            if (new_head.x == food.x && new_head.y == food.y) {
                /* Grow the snake */
                if (snake_len < MAX_SNAKE) {
                    snake_len++;
                }
                /* Shift and insert new head */
                for (int i = snake_len - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }
                snake[0] = new_head;

                score += 10;
                place_food(&food, snake, snake_len);

                /* Speed up slightly */
                if (speed_delay > 60) {
                    speed_delay -= 2;
                }
            } else {
                /* Standard movement */
                for (int i = snake_len - 1; i > 0; i--) {
                    snake[i] = snake[i - 1];
                }
                snake[0] = new_head;
            }

            /* Draw the frame */
            /* 1. Clear play arena */
            fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);

            /* 2. Draw Food */
            draw_cell(&g, food.x, food.y, 220, 50, 50, x_off, y_off);

            /* 3. Draw Snake */
            for (int i = 0; i < snake_len; i++) {
                if (i == 0) {
                    /* Head: Yellow */
                    draw_cell(&g, snake[i].x, snake[i].y, 220, 220, 50, x_off, y_off);
                } else {
                    /* Body: Green */
                    draw_cell(&g, snake[i].x, snake[i].y, 50, 200, 50, x_off, y_off);
                }
            }

            /* 4. Draw Score */
            char score_str[32];
            sprintf(score_str, "SCORE: %d", score);
            /* Clear score area at the top */
            fill_rect(&g, 0, 0, W, y_off - 4, 10, 10, 15);
            draw_str(&g, (W - 10 * 6) / 2, (y_off - 10) / 2, score_str, 240, 240, 255, 1);

            /* Tick delay */
            gfx_delay_ms(&g, speed_delay);
        }

        /* Game Over Screen */
        fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);
        draw_border(&g, x_off, y_off);

        const char *go_title = "GAME OVER";
        draw_str(&g, (W - 9 * 6 * 2) / 2, y_off + grid_h / 3, go_title, 220, 50, 50, 2);

        char go_score[64];
        sprintf(go_score, "Final Score: %d", score);
        draw_str(&g, (W - strlen(go_score) * 6) / 2, y_off + grid_h / 2, go_score, 240, 240, 255, 1);

        const char *restart_prompt = "Press SPACE to Restart, ESC to Exit";
        draw_str(&g, (W - 35 * 6) / 2, y_off + grid_h * 2 / 3, restart_prompt, 180, 180, 180, 1);

        int exit_requested = 0;
        while (1) {
            if (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { /* ESC */
                    exit_requested = 1;
                    break;
                } else if (key == ' ') { /* SPACE */
                    break;
                }
            }
            gfx_delay_ms(&g, 20);
        }

        if (exit_requested) {
            break;
        }
    }

    gfx_close(&g);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Helper Functions                                                  */
/* ------------------------------------------------------------------ */

static uint32_t rand_seed = 12345;

static void seed_rand(uint32_t s) {
    rand_seed = s;
}

/* Linear Congruential Generator for simple PRNG */
static uint32_t next_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed / 65536) % 32768;
}

/* Find a coordinate that is not covered by the snake body */
static void place_food(struct point *food, const struct point *snake, int len) {
    int valid = 0;
    while (!valid) {
        food->x = next_rand() % GRID_COLS;
        food->y = next_rand() % GRID_ROWS;

        valid = 1;
        for (int i = 0; i < len; i++) {
            if (food->x == snake[i].x && food->y == snake[i].y) {
                valid = 0;
                break;
            }
        }
    }
}

/* Draw a grid cell with a tiny border/gap */
static void draw_cell(struct gfx *ctx, int cx, int cy, uint8_t r, uint8_t g, uint8_t b, int x_off, int y_off) {
    int sx = x_off + cx * CELL_SIZE;
    int sy = y_off + cy * CELL_SIZE;
    fill_rect(ctx, sx + 1, sy + 1, CELL_SIZE - 2, CELL_SIZE - 2, r, g, b);
}

/* Draw a double-line boundary frame for the grid */
static void draw_border(struct gfx *ctx, int x_off, int y_off) {
    int grid_w = GRID_COLS * CELL_SIZE;
    int grid_h = GRID_ROWS * CELL_SIZE;
    int border_t = 3;

    /* Outermost framing */
    fill_rect(ctx, x_off - border_t, y_off - border_t, grid_w + border_t * 2, border_t, 100, 100, 150);
    fill_rect(ctx, x_off - border_t, y_off + grid_h, grid_w + border_t * 2, border_t, 100, 100, 150);
    fill_rect(ctx, x_off - border_t, y_off - border_t, border_t, grid_h + border_t * 2, 100, 100, 150);
    fill_rect(ctx, x_off + grid_w, y_off - border_t, border_t, grid_h + border_t * 2, 100, 100, 150);
}
