#include <bootie.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define BOARD_COLS 10
#define BOARD_ROWS 20
#define CELL_SIZE 18

struct point {
    int x;
    int y;
};

struct piece {
    int shape_idx;
    struct point blocks[4];
    int x;
    int y;
};

struct color {
    uint8_t r, g, b;
};

/* Global high score saved between runs in the same session */
static int session_high_score = 0;

/* The 7 classic Tetrimino shapes */
static const struct point SHAPES[7][4] = {
    { {-1, 0}, {0, 0}, {1, 0}, {2, 0} },   // 0: I
    { {0, 0}, {1, 0}, {0, 1}, {1, 1} },    // 1: O
    { {-1, 0}, {0, 0}, {1, 0}, {0, 1} },   // 2: T
    { {0, 0}, {1, 0}, {-1, 1}, {0, 1} },   // 3: S
    { {-1, 0}, {0, 0}, {0, 1}, {1, 1} },   // 4: Z
    { {-1, 0}, {0, 0}, {1, 0}, {1, 1} },   // 5: J
    { {-1, 0}, {0, 0}, {1, 0}, {-1, 1} }   // 6: L
};

static const struct color SHAPE_COLORS[8] = {
    {20, 20, 30},    // 0: Empty / Background
    {0, 220, 220},   // 1: I - Cyan
    {220, 220, 0},   // 2: O - Yellow
    {160, 0, 220},   // 3: T - Purple
    {0, 220, 0},     // 4: S - Green
    {220, 0, 0},     // 5: Z - Red
    {40, 80, 255},   // 6: J - Blue
    {220, 120, 0}    // 7: L - Orange
};

/* 7-Bag Randomizer structure to prevent dry spells of shapes */
struct bag {
    int pieces[7];
    int index;
};

static void init_bag(struct bag *b) {
    for (int i = 0; i < 7; i++) {
        b->pieces[i] = i;
    }
    b->index = 7;
}

static void shuffle_bag(struct bag *b) {
    for (int i = 6; i > 0; i--) {
        int j = next_rand() % (i + 1);
        int temp = b->pieces[i];
        b->pieces[i] = b->pieces[j];
        b->pieces[j] = temp;
    }
    b->index = 0;
}

static int draw_bag(struct bag *b) {
    if (b->index >= 7) {
        shuffle_bag(b);
    }
    return b->pieces[b->index++];
}



/* Core Tetris block drawing */
static void draw_block(struct gfx *g, int x, int y, int shape_idx, int is_ghost,
                       int x_off, int y_off) {
    if (x < 0 || x >= BOARD_COLS || y < 0 || y >= BOARD_ROWS) return;

    int px = x_off + x * CELL_SIZE;
    int py = y_off + y * CELL_SIZE;

    struct color c = SHAPE_COLORS[shape_idx];

    if (!is_ghost) {
        /* Standard Solid Block with Bevels */
        fill_rect(g, px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2, c.r, c.g, c.b);
        /* Top & Left highlight */
        fill_rect(g, px + 1, py + 1, CELL_SIZE - 2, 1, c.r + (255 - c.r)/2, c.g + (255 - c.g)/2, c.b + (255 - c.b)/2);
        fill_rect(g, px + 1, py + 1, 1, CELL_SIZE - 2, c.r + (255 - c.r)/2, c.g + (255 - c.g)/2, c.b + (255 - c.b)/2);
        /* Bottom & Right bevel */
        fill_rect(g, px + 1, py + CELL_SIZE - 2, CELL_SIZE - 2, 1, c.r/2, c.g/2, c.b/2);
        fill_rect(g, px + CELL_SIZE - 2, py + 1, 1, CELL_SIZE - 2, c.r/2, c.g/2, c.b/2);
    } else {
        /* Ghost Block (hollow border) */
        fill_rect(g, px + 1, py + 1, CELL_SIZE - 2, 1, c.r / 2, c.g / 2, c.b / 2);
        fill_rect(g, px + 1, py + CELL_SIZE - 2, CELL_SIZE - 2, 1, c.r / 2, c.g / 2, c.b / 2);
        fill_rect(g, px + 1, py + 1, 1, CELL_SIZE - 2, c.r / 2, c.g / 2, c.b / 2);
        fill_rect(g, px + CELL_SIZE - 2, py + 1, 1, CELL_SIZE - 2, c.r / 2, c.g / 2, c.b / 2);
    }
}

/* Playfield and border drawing */
static void draw_border(struct gfx *g, int x_off, int y_off, int w, int h) {
    fill_rect(g, x_off - 4, y_off - 4, w + 8, 4, 0, 120, 240); // Top
    fill_rect(g, x_off - 4, y_off + h, w + 8, 4, 0, 120, 240); // Bottom
    fill_rect(g, x_off - 4, y_off - 4, 4, h + 8, 0, 120, 240); // Left
    fill_rect(g, x_off + w, y_off - 4, 4, h + 8, 0, 120, 240); // Right
}

static void draw_next_box(struct gfx *g, int x_off, int y_off, int grid_w, int next_shape_idx) {
    int bx = x_off + grid_w + 20;
    int by = y_off + 20;
    int bw = 5 * CELL_SIZE;
    int bh = 5 * CELL_SIZE;

    draw_str(g, bx + (bw - 4 * 6) / 2, by - 14, "NEXT", 200, 200, 255, 1);

    // Box neon border
    fill_rect(g, bx - 2, by - 2, bw + 4, 2, 0, 120, 240);
    fill_rect(g, bx - 2, by + bh, bw + 4, 2, 0, 120, 240);
    fill_rect(g, bx - 2, by - 2, 2, bh + 4, 0, 120, 240);
    fill_rect(g, bx + bw, by - 2, 2, bh + 4, 0, 120, 240);

    // Box dark background
    fill_rect(g, bx, by, bw, bh, 20, 20, 30);

    if (next_shape_idx >= 0 && next_shape_idx < 7) {
        float cx = 2.0;
        float cy = 2.0;
        if (next_shape_idx == 0) { // I shape
            cx = 1.5;
            cy = 2.0;
        } else if (next_shape_idx == 1) { // O shape
            cx = 1.5;
            cy = 1.5;
        }

        for (int i = 0; i < 4; i++) {
            int px = bx + (int)((cx + SHAPES[next_shape_idx][i].x) * CELL_SIZE);
            int py = by + (int)((cy + SHAPES[next_shape_idx][i].y) * CELL_SIZE);

            struct color c = SHAPE_COLORS[next_shape_idx + 1];

            fill_rect(g, px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2, c.r, c.g, c.b);
            fill_rect(g, px + 1, py + 1, CELL_SIZE - 2, 1, c.r + (255 - c.r)/2, c.g + (255 - c.g)/2, c.b + (255 - c.b)/2);
            fill_rect(g, px + 1, py + 1, 1, CELL_SIZE - 2, c.r + (255 - c.r)/2, c.g + (255 - c.g)/2, c.b + (255 - c.b)/2);
            fill_rect(g, px + 1, py + CELL_SIZE - 2, CELL_SIZE - 2, 1, c.r/2, c.g/2, c.b/2);
            fill_rect(g, px + CELL_SIZE - 2, py + 1, 1, CELL_SIZE - 2, c.r/2, c.g/2, c.b/2);
        }
    }
}

static void draw_info_panel(struct gfx *g, int x_off, int y_off, int grid_w,
                            int score, int level, int lines, int high_score) {
    int px = x_off + grid_w + 20;
    char buf[32];

    // High Score
    draw_str(g, px, y_off + 130, "HIGH SCORE", 200, 200, 255, 1);
    fill_rect(g, px, y_off + 145, 120, 8, 10, 10, 15);
    sprintf(buf, "%d", high_score);
    draw_str(g, px, y_off + 145, buf, 255, 220, 50, 1);

    // Score
    draw_str(g, px, y_off + 185, "SCORE", 200, 200, 255, 1);
    fill_rect(g, px, y_off + 200, 120, 8, 10, 10, 15);
    sprintf(buf, "%d", score);
    draw_str(g, px, y_off + 200, buf, 255, 255, 255, 1);

    // Level
    draw_str(g, px, y_off + 240, "LEVEL", 200, 200, 255, 1);
    fill_rect(g, px, y_off + 255, 120, 8, 10, 10, 15);
    sprintf(buf, "%d", level);
    draw_str(g, px, y_off + 255, buf, 255, 255, 255, 1);

    // Lines
    draw_str(g, px, y_off + 295, "LINES", 200, 200, 255, 1);
    fill_rect(g, px, y_off + 310, 120, 8, 10, 10, 15);
    sprintf(buf, "%d", lines);
    draw_str(g, px, y_off + 310, buf, 255, 255, 255, 1);
}

/* Gameplay mechanics helpers */
static void spawn_piece(struct piece *p, int shape_idx) {
    p->shape_idx = shape_idx;
    for (int i = 0; i < 4; i++) {
        p->blocks[i] = SHAPES[shape_idx][i];
    }
    p->x = BOARD_COLS / 2 - 1;
    p->y = 0;
}

static void rotate_piece(struct piece *p, int clockwise) {
    if (p->shape_idx == 1) return; // O-piece does not rotate

    for (int i = 0; i < 4; i++) {
        int nx = clockwise ? -p->blocks[i].y : p->blocks[i].y;
        int ny = clockwise ? p->blocks[i].x : -p->blocks[i].x;
        p->blocks[i].x = nx;
        p->blocks[i].y = ny;
    }
}

static int check_collision(const struct piece *p, int dx, int dy,
                           const uint8_t board[BOARD_ROWS][BOARD_COLS]) {
    for (int i = 0; i < 4; i++) {
        int gx = p->x + p->blocks[i].x + dx;
        int gy = p->y + p->blocks[i].y + dy;

        if (gx < 0 || gx >= BOARD_COLS || gy >= BOARD_ROWS) {
            return 1;
        }
        if (gy >= 0 && board[gy][gx] != 0) {
            return 1;
        }
    }
    return 0;
}

static int try_rotate(struct piece *p, const uint8_t board[BOARD_ROWS][BOARD_COLS]) {
    struct piece backup = *p;
    rotate_piece(p, 1);

    if (!check_collision(p, 0, 0, board)) return 1;

    // Standard SRS wall kick approximations
    if (!check_collision(p, -1, 0, board)) { p->x -= 1; return 1; }
    if (!check_collision(p, 1, 0, board)) { p->x += 1; return 1; }
    if (!check_collision(p, 0, -1, board)) { p->y -= 1; return 1; }
    if (!check_collision(p, -2, 0, board)) { p->x -= 2; return 1; }
    if (!check_collision(p, 2, 0, board)) { p->x += 2; return 1; }

    *p = backup;
    return 0;
}

static void get_ghost_piece(const struct piece *p, struct piece *ghost,
                            const uint8_t board[BOARD_ROWS][BOARD_COLS]) {
    *ghost = *p;
    while (!check_collision(ghost, 0, 1, board)) {
        ghost->y++;
    }
}

static void merge_piece(const struct piece *p, uint8_t board[BOARD_ROWS][BOARD_COLS]) {
    for (int i = 0; i < 4; i++) {
        int gx = p->x + p->blocks[i].x;
        int gy = p->y + p->blocks[i].y;
        if (gx >= 0 && gx < BOARD_COLS && gy >= 0 && gy < BOARD_ROWS) {
            board[gy][gx] = p->shape_idx + 1;
        }
    }
}

static void animate_line_clear(struct gfx *g, uint8_t board[BOARD_ROWS][BOARD_COLS],
                               const int full_lines[BOARD_ROWS], int x_off, int y_off,
                               int grid_w, int grid_h, int score, int level, int lines,
                               int next_shape) {
    // We clear from center columns (4, 5) outward to edges (0, 9)
    for (int step = 0; step < 5; step++) {
        int col1 = 4 - step;
        int col2 = 5 + step;

        // Clear these columns in the full rows
        for (int r = 0; r < BOARD_ROWS; r++) {
            if (full_lines[r]) {
                board[r][col1] = 0;
                board[r][col2] = 0;
            }
        }

        // Redraw play arena
        fill_rect(g, x_off, y_off, grid_w, grid_h, 20, 20, 30);

        // Draw board blocks
        for (int r = 0; r < BOARD_ROWS; r++) {
            for (int c = 0; c < BOARD_COLS; c++) {
                if (board[r][c] > 0) {
                    draw_block(g, c, r, board[r][c], 0, x_off, y_off);
                }
            }
        }

        // Draw UI elements to keep layout updated during animation
        draw_border(g, x_off, y_off, grid_w, grid_h);
        draw_next_box(g, x_off, y_off, grid_w, next_shape);
        draw_info_panel(g, x_off, y_off, grid_w, score, level, lines, session_high_score);

        gfx_delay_ms(g, 60); // Delay for animation timing (5 * 60ms = 300ms total clear animation)
    }
}

static void shift_cleared_lines(uint8_t board[BOARD_ROWS][BOARD_COLS], const int full_lines[BOARD_ROWS]) {
    uint8_t temp_board[BOARD_ROWS][BOARD_COLS];
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            temp_board[r][c] = 0;
        }
    }
    int write_y = BOARD_ROWS - 1;
    for (int read_y = BOARD_ROWS - 1; read_y >= 0; read_y--) {
        if (!full_lines[read_y]) {
            for (int c = 0; c < BOARD_COLS; c++) {
                temp_board[write_y][c] = board[read_y][c];
            }
            write_y--;
        }
    }
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            board[r][c] = temp_board[r][c];
        }
    }
}

static int get_gravity_delay(int level) {
    if (level < 1) level = 1;
    if (level > 10) level = 10;
    int delays[11] = {800, 800, 700, 600, 500, 400, 300, 220, 150, 100, 70};
    return delays[level];
}

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

    int grid_w = BOARD_COLS * CELL_SIZE;
    int grid_h = BOARD_ROWS * CELL_SIZE;
    int x_off = (W - grid_w) / 2 - 30; // Shift left slightly to make room for NEXT box
    int y_off = (H - grid_h) / 2 + 10;

    /* --- Start Screen --- */
    fill_rect(&g, 0, 0, W, H, 10, 10, 15);
    draw_border(&g, x_off, y_off, grid_w, grid_h);

    const char *title = "Tetris";
    const char *prompt = "Press any key to start...";

    // Align title and prompt relative to the playfield border to keep it centered visually
    draw_str(&g, x_off + (grid_w - (int)strlen(title) * 6 * 2) / 2, y_off + grid_h / 3, title, 0, 220, 220, 2);
    draw_str(&g, x_off + (grid_w - 25 * 6) / 2, y_off + grid_h / 2, prompt, 200, 200, 200, 1);

    while (!gfx_checkkey(&g)) {
        gfx_delay_ms(&g, 25);
    }
    gfx_getkey(&g); // Consume starting key

    uint8_t board[BOARD_ROWS][BOARD_COLS];
    struct bag random_bag;

    while (1) {
        // Clear screen to background color on startup/restart
        fill_rect(&g, 0, 0, W, H, 10, 10, 15);
        draw_border(&g, x_off, y_off, grid_w, grid_h);

        /* Reset Game Variables */
        memset(board, 0, sizeof(board));
        init_bag(&random_bag);

        int score = 0;
        int level = 1;
        int lines = 0;
        int game_over = 0;

        struct piece active_piece;
        struct piece ghost_piece;
        int next_shape = draw_bag(&random_bag);

        spawn_piece(&active_piece, draw_bag(&random_bag));
        get_ghost_piece(&active_piece, &ghost_piece, board);

        int gravity_timer = 0;
        int lock_timer = 0;
        int line_scores[5] = {0, 100, 300, 500, 800};

        /* --- Main Game Loop --- */
        while (!game_over) {
            int tick_start_flag = 0;

            /* Handle Keyboard Input */
            while (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { // ESC to exit game completely
                    gfx_close(&g);
                    return 0;
                }

                if (key == 0x4B00 || key == 'a' || key == 'A') { // Left
                    if (!check_collision(&active_piece, -1, 0, board)) {
                        active_piece.x--;
                        get_ghost_piece(&active_piece, &ghost_piece, board);
                        lock_timer = 0; // reset lock delay on successful move
                    }
                } else if (key == 0x4D00 || key == 'd' || key == 'D') { // Right
                    if (!check_collision(&active_piece, 1, 0, board)) {
                        active_piece.x++;
                        get_ghost_piece(&active_piece, &ghost_piece, board);
                        lock_timer = 0;
                    }
                } else if (key == 0x4800 || key == 'w' || key == 'W' || key == 'k' || key == 'K') { // Rotate Clockwise
                    if (try_rotate(&active_piece, board)) {
                        get_ghost_piece(&active_piece, &ghost_piece, board);
                        lock_timer = 0;
                    }
                } else if (key == 0x5000 || key == 's' || key == 'S') { // Soft Drop
                    if (!check_collision(&active_piece, 0, 1, board)) {
                        active_piece.y++;
                        score++; // 1 point per soft drop grid line
                        gravity_timer = 0; // reset gravity on manual drop
                    }
                } else if (key == ' ' || key == 0x0D) { // Hard Drop (Space or Enter)
                    int drop_dist = 0;
                    while (!check_collision(&active_piece, 0, 1, board)) {
                        active_piece.y++;
                        drop_dist++;
                    }
                    score += drop_dist * 2; // 2 points per hard drop grid line
                    lock_timer = 500; // force immediate lock
                }
            }

            /* Apply Gravity */
            gravity_timer += 25;
            int delay_limit = get_gravity_delay(level);
            if (gravity_timer >= delay_limit) {
                gravity_timer = 0;
                if (!check_collision(&active_piece, 0, 1, board)) {
                    active_piece.y++;
                }
            }

            /* Handle Lock Delay */
            int on_ground = check_collision(&active_piece, 0, 1, board);
            if (on_ground) {
                lock_timer += 25;
                if (lock_timer >= 500) {
                    lock_timer = 0;
                    merge_piece(&active_piece, board);

                    // Check for full lines
                    int full_lines[BOARD_ROWS];
                    int cleared = 0;
                    for (int r = 0; r < BOARD_ROWS; r++) {
                        int full = 1;
                        for (int c = 0; c < BOARD_COLS; c++) {
                            if (board[r][c] == 0) {
                                full = 0;
                                break;
                            }
                        }
                        if (full) {
                            full_lines[r] = 1;
                            cleared++;
                        } else {
                            full_lines[r] = 0;
                        }
                    }

                    if (cleared > 0) {
                        // Play block dissolving animation from center outward
                        animate_line_clear(&g, board, full_lines, x_off, y_off, grid_w, grid_h,
                                           score, level, lines, next_shape);
                        // Shift blocks down
                        shift_cleared_lines(board, full_lines);
                        score += line_scores[cleared] * level;
                        lines += cleared;
                        level = lines / 10 + 1;
                    }

                    if (score > session_high_score) {
                        session_high_score = score;
                    }

                    // Spawn next piece
                    spawn_piece(&active_piece, next_shape);
                    next_shape = draw_bag(&random_bag);
                    get_ghost_piece(&active_piece, &ghost_piece, board);

                    // If newly spawned piece collides immediately, game is over
                    if (check_collision(&active_piece, 0, 0, board)) {
                        game_over = 1;
                    }
                }
            } else {
                lock_timer = 0;
            }

            /* --- Rendering Frame --- */
            gfx_backbuffer_begin(&g);
            // Clear play arena
            fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);

            // 3. Draw Ghost Piece (hollow preview)
            for (int i = 0; i < 4; i++) {
                int gx = ghost_piece.x + ghost_piece.blocks[i].x;
                int gy = ghost_piece.y + ghost_piece.blocks[i].y;
                if (gy >= 0) {
                    draw_block(&g, gx, gy, active_piece.shape_idx + 1, 1, x_off, y_off);
                }
            }

            // 4. Draw Active Piece
            for (int i = 0; i < 4; i++) {
                int gx = active_piece.x + active_piece.blocks[i].x;
                int gy = active_piece.y + active_piece.blocks[i].y;
                if (gy >= 0) {
                    draw_block(&g, gx, gy, active_piece.shape_idx + 1, 0, x_off, y_off);
                }
            }

            // 5. Draw Frozen Board Blocks
            for (int r = 0; r < BOARD_ROWS; r++) {
                for (int c = 0; c < BOARD_COLS; c++) {
                    if (board[r][c] > 0) {
                        draw_block(&g, c, r, board[r][c], 0, x_off, y_off);
                    }
                }
            }

            // 6. Draw Borders & Side Panel Panels
            draw_border(&g, x_off, y_off, grid_w, grid_h);
            draw_next_box(&g, x_off, y_off, grid_w, next_shape);
            draw_info_panel(&g, x_off, y_off, grid_w, score, level, lines, session_high_score);
            gfx_backbuffer_end(&g);

            gfx_delay_ms(&g, 25); // ~40 FPS tick rate
        }

        /* --- Game Over Screen --- */
        fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);
        draw_border(&g, x_off, y_off, grid_w, grid_h);

        const char *go_title = "GAME OVER";
        char go_score[64];
        sprintf(go_score, "Score: %d", score);
        const char *restart_prompt = "SPACE to Restart, ESC to Exit";

        draw_str(&g, x_off + (grid_w - 9 * 6 * 2) / 2, y_off + grid_h / 3, go_title, 220, 50, 50, 2);
        draw_str(&g, x_off + (grid_w - strlen(go_score) * 6) / 2, y_off + grid_h / 2, go_score, 255, 255, 255, 1);
        draw_str(&g, x_off + (grid_w - 29 * 6) / 2, y_off + 2 * grid_h / 3, restart_prompt, 200, 200, 200, 1);

        int exit_requested = 0;
        while (1) {
            if (gfx_checkkey(&g)) {
                int key = gfx_getkey(&g);
                if (key == 27) { // ESC to exit
                    exit_requested = 1;
                    break;
                }
                if (key == ' ') { // Space to restart
                    break;
                }
            }
            gfx_delay_ms(&g, 25);
        }

        if (exit_requested) {
            break;
        }
    }

    gfx_close(&g);
    return 0;
}
