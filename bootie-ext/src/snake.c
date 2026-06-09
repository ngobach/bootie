#include <bootie.h>
#include <bootie-gfx.h>
#include <stdint.h>

#define GRID_COLS 40
#define GRID_ROWS 30
#define CELL_SIZE 12
#define MAX_SNAKE 512
struct point {
  int x;
  int y;
};

static int session_high_score;

/* Forward declarations of helper functions */

static void place_food(struct point *food, const struct point *snake, int len);
static void draw_cell(struct gfx *ctx, int cx, int cy, uint8_t r, uint8_t g,
                      uint8_t b, int x_off, int y_off);
static void draw_border(struct gfx *ctx, int x_off, int y_off);

/* Main game execution loop (must be the first function defined in the C file)
 */
int gmain(int argc, char *argv[], int flags) {
  (void)argc;
  (void)argv;
  (void)flags;

  struct gfx g;
  if (!gfx_init(&g)) {
    return 0;
  }
  gfx_font_load();

  uint32_t W = gfx_width(&g);
  uint32_t H = gfx_height(&g);

  /* Center of play area on screen */
  int grid_w = GRID_COLS * CELL_SIZE;
  int grid_h = GRID_ROWS * CELL_SIZE;
  int x_off = (W - grid_w) / 2;
  int y_off = (H - grid_h) / 2 + 10;

  /* Start screen */
  fill_rect(&g, 0, 0, W, H, 10, 10, 15);
  draw_border(&g, x_off, y_off);

  const char *title = "Snake";
  const char *prompt = "Press any key to start...";
  draw_str(&g, (W - gfx_text_width(title, SCALE_PX(2))) / 2, y_off + grid_h / 3,
           title, 50, 220, 50, 2);
  draw_str(&g, (W - gfx_text_width(prompt, SCALE_PX(1))) / 2, y_off + grid_h / 2, prompt, 200, 200, 200, 1);

  while (!gfx_checkkey(&g)) {
    gfx_delay_ms(&g, 25);
  }
  gfx_getkey(&g); /* consume the pressed key */

  /* Game State variables */
  struct point snake[MAX_SNAKE];
  int snake_len;
  struct point food;
  int dx, dy;   /* current moving direction */
  int ndx, ndy; /* next direction input */
  int score;
  int game_over;
  int speed_delay;

  while (1) {
    /* Initialize/Reset game variables */
    snake_len = 4;
    snake[0].x = GRID_COLS / 2;
    snake[0].y = GRID_ROWS / 2;
    snake[1].x = GRID_COLS / 2 - 1;
    snake[1].y = GRID_ROWS / 2;
    snake[2].x = GRID_COLS / 2 - 2;
    snake[2].y = GRID_ROWS / 2;
    snake[3].x = GRID_COLS / 2 - 3;
    snake[3].y = GRID_ROWS / 2;

    dx = 1;
    dy = 0;
    ndx = 1;
    ndy = 0;
    score = 0;
    game_over = 0;
    speed_delay = 140;

    place_food(&food, snake, snake_len);

    /* Main Game Loop */
    int logic_accumulator = 0;
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
          ndx = 0;
          ndy = -1;
        } else if ((key == 0x5000 || key == 's' || key == 'S') && dy == 0) {
          ndx = 0;
          ndy = 1;
        } else if ((key == 0x4B00 || key == 'a' || key == 'A') && dx == 0) {
          ndx = -1;
          ndy = 0;
        } else if ((key == 0x4D00 || key == 'd' || key == 'D') && dx == 0) {
          ndx = 1;
          ndy = 0;
        }
      }

      logic_accumulator += 25;
      if (logic_accumulator >= speed_delay) {
        logic_accumulator = 0;

        /* Apply next direction */
        dx = ndx;
        dy = ndy;

        /* Compute new head position */
        struct point new_head;
        new_head.x = snake[0].x + dx;
        new_head.y = snake[0].y + dy;

        /* Check wall collision */
        if (new_head.x < 0 || new_head.x >= GRID_COLS || new_head.y < 0 ||
            new_head.y >= GRID_ROWS) {
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
        if (game_over)
          break;

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
          if (score > session_high_score)
              session_high_score = score;
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
      }

      /* Draw the frame */
      gfx_backbuffer_begin(&g);
      /* 1. Clear play arena */
      fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);

      /* 3. Draw Food */
      draw_cell(&g, food.x, food.y, 220, 50, 50, x_off, y_off);

      /* 4. Draw Snake */
      for (int i = 0; i < snake_len; i++) {
        if (i == 0) {
          /* Head: Yellow */
          draw_cell(&g, snake[i].x, snake[i].y, 220, 220, 50, x_off, y_off);
        } else {
          /* Body: Green */
          draw_cell(&g, snake[i].x, snake[i].y, 50, 200, 50, x_off, y_off);
        }
      }

      /* 5. Draw Border & Score */
      draw_border(&g, x_off, y_off);

      /* Clear score area at the top */
      fill_rect(&g, 0, 0, W, y_off - 4, 10, 10, 15);
      if (session_high_score > 0)
          draw_strf_centered(&g, W / 2, (y_off - 10) / 2, 240, 240, 255, 1,
                    "SCORE: %d    HI: %d", score, session_high_score);
      else
          draw_strf_centered(&g, W / 2, (y_off - 10) / 2, 240, 240, 255, 1,
                    "SCORE: %d", score);
      gfx_backbuffer_end(&g);

      /* Tick delay - Constant 40 FPS */
      gfx_delay_ms(&g, 25);
    }

    /* Game Over Screen */
    fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);
    draw_border(&g, x_off, y_off);

    const char *go_title = "GAME OVER";
    const char *restart_prompt = "Press SPACE to Restart, ESC to Exit";

    int exit_requested = 0;
    while (1) {
      gfx_backbuffer_begin(&g);
      fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);
      draw_border(&g, x_off, y_off);
      draw_str(&g, (W - gfx_text_width(go_title, SCALE_PX(2))) / 2, y_off + grid_h / 3, go_title, 220, 50,
               50, 2);
      if (score >= session_high_score && score > 0)
          draw_strf_centered(&g, W / 2, y_off + grid_h / 2, 240, 240, 255, 1,
                    "New High Score: %d!", score);
      else
          draw_strf_centered(&g, W / 2, y_off + grid_h / 2, 240, 240, 255, 1,
                    "Score: %d    High Score: %d", score, session_high_score);
      draw_str(&g, (W - gfx_text_width(restart_prompt, SCALE_PX(1))) / 2, y_off + grid_h * 2 / 3, restart_prompt,
               180, 180, 180, 1);
      gfx_backbuffer_end(&g);

      if (gfx_checkkey(&g)) {
        int key = gfx_getkey(&g);
        if (key == 27) { /* ESC */
          exit_requested = 1;
          break;
        } else if (key == ' ') { /* SPACE */
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

/* ------------------------------------------------------------------ */
/*  Helper Functions                                                  */
/* ------------------------------------------------------------------ */

/* Find a coordinate that is not covered by the snake body */
static void place_food(struct point *food, const struct point *snake, int len) {
  int valid = 0;
  int attempts = 0;
  while (attempts < 200) {
    food->x = next_rand() % GRID_COLS;
    food->y = next_rand() % GRID_ROWS;

    valid = 1;
    for (int i = 0; i < len; i++) {
      if (food->x == snake[i].x && food->y == snake[i].y) {
        valid = 0;
        break;
      }
    }
    if (valid) {
      return;
    }
    attempts++;
  }

  /* Fallback: Linear scan starting from a random cell */
  int start_cell = next_rand() % (GRID_COLS * GRID_ROWS);
  for (int offset = 0; offset < GRID_COLS * GRID_ROWS; offset++) {
    int cell = (start_cell + offset) % (GRID_COLS * GRID_ROWS);
    int cx = cell % GRID_COLS;
    int cy = cell / GRID_COLS;

    valid = 1;
    for (int i = 0; i < len; i++) {
      if (cx == snake[i].x && cy == snake[i].y) {
        valid = 0;
        break;
      }
    }
    if (valid) {
      food->x = cx;
      food->y = cy;
      return;
    }
  }
}

/* Draw a grid cell with a tiny border/gap */
static void draw_cell(struct gfx *ctx, int cx, int cy, uint8_t r, uint8_t g,
                      uint8_t b, int x_off, int y_off) {
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
  fill_rect(ctx, x_off - border_t, y_off - border_t, grid_w + border_t * 2,
            border_t, 100, 100, 150);
  fill_rect(ctx, x_off - border_t, y_off + grid_h, grid_w + border_t * 2,
            border_t, 100, 100, 150);
  fill_rect(ctx, x_off - border_t, y_off - border_t, border_t,
            grid_h + border_t * 2, 100, 100, 150);
  fill_rect(ctx, x_off + grid_w, y_off - border_t, border_t,
            grid_h + border_t * 2, 100, 100, 150);
}


