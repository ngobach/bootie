#include <bootie.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define BRICK_COLS    10
#define BRICK_ROWS    6
#define BRICK_W       52
#define BRICK_H       18
#define BRICK_GAP     3
#define PADDLE_W      64
#define PADDLE_H      10
#define BALL_SZ       8
#define PADDLE_SPEED  10
#define BALL_SPEED    4
#define MAX_LIVES     3

static const struct color {
    uint8_t r, g, b;
} BRICK_COLORS[BRICK_ROWS] = {
    {220, 50, 50},    // 0: Red
    {220, 120, 0},    // 1: Orange
    {220, 220, 0},    // 2: Yellow
    {0, 220, 0},      // 3: Green
    {0, 120, 220},    // 4: Blue
    {160, 0, 220}     // 5: Purple
};

static int abs_val(int a) {
    return a < 0 ? -a : a;
}

int gmain(int argc, char *argv[], int flags) {
    (void)argc;
    (void)argv;
    (void)flags;

    struct gfx g;
    if (!gfx_init(&g)) return 0;
    gfx_font_load();

    uint32_t W = gfx_width(&g);
    uint32_t H = gfx_height(&g);

#define BORDER_T  3
#define WND_PAD   8

    int grid_w = BRICK_COLS * (BRICK_W + BRICK_GAP) - BRICK_GAP;
    int grid_h = BRICK_ROWS * (BRICK_H + BRICK_GAP) - BRICK_GAP;
    int x_off = (W - grid_w) / 2;
    int y_off = 45;

    int wnd_l = x_off - WND_PAD;
    int wnd_t = y_off - WND_PAD;
    int wnd_r = x_off + grid_w + WND_PAD;
    int paddle_y = H - 50;
    int wnd_b = paddle_y + PADDLE_H / 2 + WND_PAD;

    int paddle_x;

    /* --- Start Screen --- */
    fill_rect(&g, 0, 0, W, H, 10, 10, 15);
    fill_rect(&g, wnd_l, wnd_t, wnd_r - wnd_l, wnd_b - wnd_t, 20, 20, 30);
    fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T,
              wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
    fill_rect(&g, wnd_l - BORDER_T, wnd_b,
              wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
    fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T, BORDER_T,
              wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);
    fill_rect(&g, wnd_r, wnd_t - BORDER_T, BORDER_T,
              wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);
    const char *title = "BREAKOUT";
    const char *ctrl = "LEFT/RIGHT or A/D to move";
    const char *prompt = "Press any key to start...";
    draw_str(&g, (wnd_l + wnd_r - gfx_text_width(title, 28)) / 2, wnd_t + (wnd_b - wnd_t) / 3, title, 255, 255, 255, 28);
    draw_str(&g, (wnd_l + wnd_r - gfx_text_width(ctrl, 16)) / 2, wnd_t + (wnd_b - wnd_t) / 2 - 20, ctrl, 200, 200, 200, 16);
    draw_str(&g, (wnd_l + wnd_r - gfx_text_width(prompt, 16)) / 2, wnd_t + (wnd_b - wnd_t) / 2 + 24, prompt, 200, 200, 200, 16);
    while (!gfx_checkkey(&g)) gfx_delay_ms(&g, 25);
    gfx_getkey(&g);

    while (1) {
        /* Block grid: 0 = empty, 1..BRICK_ROWS = row color index */
        uint8_t bricks[BRICK_ROWS][BRICK_COLS];
        for (int r = 0; r < BRICK_ROWS; r++)
            for (int c = 0; c < BRICK_COLS; c++)
                bricks[r][c] = r + 1;

        paddle_x = (wnd_l + wnd_r) / 2 - PADDLE_W / 2;
        int ball_x = (wnd_l + wnd_r) / 2;
        int ball_y = paddle_y - PADDLE_H / 2 - BALL_SZ / 2;
        int ball_vx = BALL_SPEED, ball_vy = -BALL_SPEED;
        if (next_rand() % 2) ball_vx = -ball_vx;

        int score = 0;
        int lives = MAX_LIVES;
        int serving = 1;
        int game_over = 0;

        while (!game_over) {
            struct bt_fps fps;
            bt_fps_init(&fps, 60);

            /* Input */
            while (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { gfx_close(&g); return 0; }
                if (key == ' ' || key == 0x0D) {
                    if (serving) {
                        serving = 0;
                        ball_vy = -BALL_SPEED;
                        if (next_rand() % 2) ball_vx = -BALL_SPEED;
                        else ball_vx = BALL_SPEED;
                    }
                }
                if (key == 0x4B00 || key == 'a' || key == 'A') paddle_x -= PADDLE_SPEED;
                if (key == 0x4D00 || key == 'd' || key == 'D') paddle_x += PADDLE_SPEED;
            }

            if (paddle_x < wnd_l) paddle_x = wnd_l;
            if (paddle_x + PADDLE_W > wnd_r) paddle_x = wnd_r - PADDLE_W;

            /* Ball follows paddle while serving */
            if (serving) {
                ball_x = paddle_x + PADDLE_W / 2;
            }

            if (!serving) {
                /* Move ball */
                ball_x += ball_vx;
                ball_y += ball_vy;

                /* Wall bounces */
                if (ball_x - BALL_SZ / 2 <= wnd_l) { ball_x = wnd_l + BALL_SZ / 2 + 1; ball_vx = -ball_vx; }
                if (ball_x + BALL_SZ / 2 >= wnd_r) { ball_x = wnd_r - BALL_SZ / 2 - 1; ball_vx = -ball_vx; }
                if (ball_y - BALL_SZ / 2 <= wnd_t) { ball_y = wnd_t + BALL_SZ / 2 + 1; ball_vy = -ball_vy; }

                /* Paddle collision */
                if (ball_vy > 0 &&
                    ball_y + BALL_SZ / 2 >= paddle_y - PADDLE_H / 2 &&
                    ball_y + BALL_SZ / 2 <= paddle_y + PADDLE_H / 2 &&
                    ball_x + BALL_SZ / 2 > paddle_x &&
                    ball_x - BALL_SZ / 2 < paddle_x + PADDLE_W) {
                    int hit = (ball_x - paddle_x) * 100 / PADDLE_W;
                    ball_vx = (hit - 50) * BALL_SPEED / 25;
                    if (abs_val(ball_vx) < 1) ball_vx = (next_rand() % 2) ? 1 : -1;
                    ball_vy = -BALL_SPEED;
                    ball_y = paddle_y - PADDLE_H / 2 - BALL_SZ / 2 - 1;
                }

                /* Brick collision (AABB with 3x3 neighborhood) */
                {
                    int ball_l = ball_x - BALL_SZ / 2;
                    int ball_r = ball_x + BALL_SZ / 2;
                    int ball_t = ball_y - BALL_SZ / 2;
                    int ball_b = ball_y + BALL_SZ / 2;

                    /* Early out: ball outside brick grid */
                    if (ball_b > y_off && ball_t < y_off + grid_h &&
                        ball_r > x_off && ball_l < x_off + grid_w) {
                        int bc = (ball_x - x_off) / (BRICK_W + BRICK_GAP);
                        int br = (ball_y - y_off) / (BRICK_H + BRICK_GAP);
                        int hit = 0;

                        for (int dr = -1; dr <= 1 && !hit; dr++) {
                            for (int dc = -1; dc <= 1 && !hit; dc++) {
                                int r = br + dr;
                                int c = bc + dc;
                                if (r < 0 || r >= BRICK_ROWS || c < 0 || c >= BRICK_COLS)
                                    continue;
                                if (!bricks[r][c])
                                    continue;

                                int bx = x_off + c * (BRICK_W + BRICK_GAP);
                                int by = y_off + r * (BRICK_H + BRICK_GAP);
                                int brick_r = bx + BRICK_W;
                                int brick_b = by + BRICK_H;

                                /* AABB overlap check */
                                if (ball_r <= bx || ball_l >= brick_r) continue;
                                if (ball_b <= by || ball_t >= brick_b) continue;

                                /* Determine bounce face from overlap depth */
                                int ol = ball_r - bx;
                                int or_ = brick_r - ball_l;
                                int ot = ball_b - by;
                                int ob = brick_b - ball_t;

                                int min = ol;
                                if (or_ < min) min = or_;
                                if (ot < min) min = ot;
                                if (ob < min) min = ob;

                                if (min == ol || min == or_)
                                    ball_vx = -ball_vx;
                                else
                                    ball_vy = -ball_vy;

                                bricks[r][c] = 0;
                                score += 10;
                                hit = 1;
                            }
                        }

                        if (hit) {
                            int all_clear = 1;
                            for (int r = 0; r < BRICK_ROWS && all_clear; r++)
                                for (int c = 0; c < BRICK_COLS && all_clear; c++)
                                    if (bricks[r][c]) all_clear = 0;
                            if (all_clear)
                                game_over = 1;
                        }
                    }
                }

                /* Ball fell off bottom */
                if (ball_y + BALL_SZ / 2 >= wnd_b) {
                    lives--;
                    if (lives <= 0) {
                        game_over = 1;
                    } else {
                        /* Reset ball */
                        serving = 1;
                        ball_x = (wnd_l + wnd_r) / 2;
                        ball_y = paddle_y - PADDLE_H / 2 - BALL_SZ / 2;
                        ball_vx = 0;
                        ball_vy = 0;
                        paddle_x = (wnd_l + wnd_r) / 2 - PADDLE_W / 2;
                    }
                }
            }

            /* --- Rendering --- */
            gfx_backbuffer_begin(&g);

            /* Clear window area */
            fill_rect(&g, wnd_l, wnd_t, wnd_r - wnd_l, wnd_b - wnd_t, 20, 20, 30);

            /* Draw border around window (like snake.c) */
            fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T,
                      wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
            fill_rect(&g, wnd_l - BORDER_T, wnd_b,
                      wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
            fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T, BORDER_T,
                      wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);
            fill_rect(&g, wnd_r, wnd_t - BORDER_T, BORDER_T,
                      wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);

            /* Draw bricks */
            for (int r = 0; r < BRICK_ROWS; r++) {
                for (int c = 0; c < BRICK_COLS; c++) {
                    if (!bricks[r][c]) continue;
                    int bx = x_off + c * (BRICK_W + BRICK_GAP);
                    int by = y_off + r * (BRICK_H + BRICK_GAP);
                    const struct color *col = &BRICK_COLORS[r];
                    fill_rect(&g, bx + 1, by + 1, BRICK_W - 2, BRICK_H - 2, col->r, col->g, col->b);
                    fill_rect(&g, bx + 1, by + 1, BRICK_W - 2, 1,
                              col->r + (255 - col->r) / 2, col->g + (255 - col->g) / 2, col->b + (255 - col->b) / 2);
                    fill_rect(&g, bx + 1, by + 1, 1, BRICK_H - 2,
                              col->r + (255 - col->r) / 2, col->g + (255 - col->g) / 2, col->b + (255 - col->b) / 2);
                    fill_rect(&g, bx + 1, by + BRICK_H - 2, BRICK_W - 2, 1, col->r / 2, col->g / 2, col->b / 2);
                    fill_rect(&g, bx + BRICK_W - 2, by + 1, 1, BRICK_H - 2, col->r / 2, col->g / 2, col->b / 2);
                }
            }

            /* Draw paddle */
            int py = paddle_y - PADDLE_H / 2;
            fill_rect(&g, paddle_x, py, PADDLE_W, PADDLE_H, 100, 180, 255);
            fill_rect(&g, paddle_x, py, PADDLE_W, 1, 180, 220, 255);
            fill_rect(&g, paddle_x, py, 1, PADDLE_H, 180, 220, 255);
            fill_rect(&g, paddle_x, py + PADDLE_H - 1, PADDLE_W, 1, 40, 80, 140);
            fill_rect(&g, paddle_x + PADDLE_W - 1, py, 1, PADDLE_H, 40, 80, 140);

            /* Draw ball */
            fill_rect(&g, ball_x - BALL_SZ / 2, ball_y - BALL_SZ / 2, BALL_SZ, BALL_SZ, 255, 255, 255);

            /* Score & Lives */
            /* Clear and draw title bar above the window */
            fill_rect(&g, 0, 0, W, wnd_t - BORDER_T, 10, 10, 15);
            int top_label_y = (wnd_t - 20) / 2;
    draw_strf(&g, 10, top_label_y, 200, 200, 255, 16, "SCORE: %d", score);
    draw_strf(&g, W - 100, top_label_y, 200, 200, 255, 16, "LIVES: %d", lives);

            if (serving) {
                draw_str(&g, (wnd_l + wnd_r - gfx_text_width("SPACE to launch", 16)) / 2, paddle_y - 30, "SPACE to launch", 200, 200, 200, 16);
            }

            gfx_backbuffer_end(&g);
            bt_fps_wait(&fps);
        }

        /* --- Game Over Screen --- */
        fill_rect(&g, 0, 0, W, H, 10, 10, 15);
        fill_rect(&g, wnd_l, wnd_t, wnd_r - wnd_l, wnd_b - wnd_t, 20, 20, 30);
        fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T,
                  wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
        fill_rect(&g, wnd_l - BORDER_T, wnd_b,
                  wnd_r - wnd_l + BORDER_T * 2, BORDER_T, 100, 100, 150);
        fill_rect(&g, wnd_l - BORDER_T, wnd_t - BORDER_T, BORDER_T,
                  wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);
        fill_rect(&g, wnd_r, wnd_t - BORDER_T, BORDER_T,
                  wnd_b - wnd_t + BORDER_T * 2, 100, 100, 150);

        const char *go_title = "GAME OVER";
        const char *restart = "SPACE to Play Again, ESC to Exit";

        draw_str(&g, (wnd_l + wnd_r - gfx_text_width(go_title, 28)) / 2, wnd_t + (wnd_b - wnd_t) / 3, go_title, 220, 50, 50, 28);
        if (lives <= 0)
            draw_strf(&g, (wnd_l + wnd_r - gfx_text_width("Score: 99999", 16)) / 2, wnd_t + (wnd_b - wnd_t) / 2,
                      255, 255, 255, 16, "Score: %d", score);
        else
            draw_strf(&g, (wnd_l + wnd_r - gfx_text_width("You Win! Score: 99999", 16)) / 2, wnd_t + (wnd_b - wnd_t) / 2,
                      255, 255, 255, 16, "You Win! Score: %d", score);
        draw_str(&g, (wnd_l + wnd_r - gfx_text_width(restart, 16)) / 2, wnd_t + (wnd_b - wnd_t) * 2 / 3, restart, 200, 200, 200, 16);

        int exit_req = 0;
        while (1) {
            if (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { exit_req = 1; break; }
                if (key == ' ') { break; }
            }
            gfx_delay_ms(&g, 25);
        }
        if (exit_req) break;
    }

    gfx_close(&g);
    return 0;
}
