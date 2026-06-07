#include <bootprog.h>
#include <gfx.h>
#include <stdint.h>

#define GRID_COLS 40
#define GRID_ROWS 30
#define CELL_SIZE 12
#define MAX_SNAKE 512
#define MAX_STARS 80

struct point {
  int x;
  int y;
};

struct star {
  int32_t x;
  int32_t y;
  int32_t z;
  int prev_px;
  int prev_py;
  int prev_size;
};

/* Forward declarations of helper functions */

static void place_food(struct point *food, const struct point *snake, int len);
static void draw_cell(struct gfx *ctx, int cx, int cy, uint8_t r, uint8_t g,
                      uint8_t b, int x_off, int y_off);
static void draw_border(struct gfx *ctx, int x_off, int y_off);
static void init_starfield(struct star *stars, int count);
static void update_starfield(struct gfx *ctx, struct star *stars, int count,
                             int x_off, int y_off, int grid_w, int grid_h,
                             int start_screen);

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

  uint32_t W = gfx_width(&g);
  uint32_t H = gfx_height(&g);

  /* Center of play area on screen */
  int grid_w = GRID_COLS * CELL_SIZE;
  int grid_h = GRID_ROWS * CELL_SIZE;
  int x_off = (W - grid_w) / 2;
  int y_off = (H - grid_h) / 2 + 10;

  /* Initialize starfield background */
  struct star stars[MAX_STARS];
  init_starfield(stars, MAX_STARS);

  /* Start screen */
  fill_rect(&g, 0, 0, W, H, 10, 10, 15);
  draw_border(&g, x_off, y_off);

  const char *title = "Bare-metal Snake";
  const char *prompt = "Press any key to start...";

  while (!gfx_checkkey(&g)) {
    update_starfield(&g, stars, MAX_STARS, x_off, y_off, grid_w, grid_h, 1);

    /* Redraw border and text on top of starfield to avoid clipping */
    draw_border(&g, x_off, y_off);
    draw_str(&g, (W - 16 * 6 * 2) / 2, y_off + grid_h / 3, title, 50, 220, 50,
             2);
    draw_str(&g, (W - 25 * 6) / 2, y_off + grid_h / 2, prompt, 200, 200, 200,
             1);

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
      /* 1. Animate background starfield outside play area */
      update_starfield(&g, stars, MAX_STARS, x_off, y_off, grid_w, grid_h, 0);

      /* 2. Clear play arena */
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

      char score_str[32];
      sprintf(score_str, "SCORE: %d", score);
      /* Clear score area at the top */
      fill_rect(&g, 0, 0, W, y_off - 4, 10, 10, 15);
      draw_str(&g, (W - 10 * 6) / 2, (y_off - 10) / 2, score_str, 240, 240, 255,
               1);

      /* Tick delay - Constant 40 FPS */
      gfx_delay_ms(&g, 25);
    }

    /* Game Over Screen */
    fill_rect(&g, x_off, y_off, grid_w, grid_h, 20, 20, 30);
    draw_border(&g, x_off, y_off);

    const char *go_title = "GAME OVER";
    char go_score[64];
    sprintf(go_score, "Final Score: %d", score);
    const char *restart_prompt = "Press SPACE to Restart, ESC to Exit";

    int exit_requested = 0;
    while (1) {
      /* Animate starfield in the background */
      update_starfield(&g, stars, MAX_STARS, x_off, y_off, grid_w, grid_h, 1);

      /* Redraw game over items on top of starfield */
      draw_border(&g, x_off, y_off);
      draw_str(&g, (W - 9 * 6 * 2) / 2, y_off + grid_h / 3, go_title, 220, 50,
               50, 2);
      draw_str(&g, (W - strlen(go_score) * 6) / 2, y_off + grid_h / 2, go_score,
               240, 240, 255, 1);
      draw_str(&g, (W - 35 * 6) / 2, y_off + grid_h * 2 / 3, restart_prompt,
               180, 180, 180, 1);

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

/* Initialize star positions and depth values */
static void init_starfield(struct star *stars, int count) {
  for (int i = 0; i < count; i++) {
    stars[i].x = ((int32_t)next_rand() % 2000) - 1000;
    stars[i].y = ((int32_t)next_rand() % 2000) - 1000;
    stars[i].z = ((int32_t)next_rand() % 999) + 1;
    stars[i].prev_px = -1;
    stars[i].prev_py = -1;
    stars[i].prev_size = 1;
  }
}

/* Update positions, project to 2D screen space, erase old pixels and draw stars
 */
static void update_starfield(struct gfx *ctx, struct star *stars, int count,
                             int x_off, int y_off, int grid_w, int grid_h,
                             int start_screen) {
  uint32_t W = gfx_width(ctx);
  uint32_t H = gfx_height(ctx);

  for (int i = 0; i < count; i++) {
    /* 1. Erase old star */
    if (stars[i].prev_px >= 0 && stars[i].prev_px + stars[i].prev_size <= (int)W &&
        stars[i].prev_py >= 0 && stars[i].prev_py + stars[i].prev_size <= (int)H) {
      if (stars[i].prev_size == 2) {
        put_pixel(ctx, stars[i].prev_px, stars[i].prev_py, 10, 10, 15);
        put_pixel(ctx, stars[i].prev_px + 1, stars[i].prev_py, 10, 10, 15);
        put_pixel(ctx, stars[i].prev_px, stars[i].prev_py + 1, 10, 10, 15);
        put_pixel(ctx, stars[i].prev_px + 1, stars[i].prev_py + 1, 10, 10, 15);
      } else {
        put_pixel(ctx, stars[i].prev_px, stars[i].prev_py, 10, 10, 15);
      }
    }

    /* 2. Move star closer */
    stars[i].z -= 20;
    if (stars[i].z <= 0) {
      stars[i].x = ((int32_t)next_rand() % 2000) - 1000;
      stars[i].y = ((int32_t)next_rand() % 2000) - 1000;
      stars[i].z = 1000;
      stars[i].prev_px = -1;
      stars[i].prev_py = -1;
      continue;
    }

    /* 3. Project coordinates */
    int px = (int)W / 2 + (stars[i].x * 256) / stars[i].z;
    int py = (int)H / 2 + (stars[i].y * 256) / stars[i].z;

    /* 4. Reset if off-screen (ensures stars stay dense and visible) */
    int max_size = (stars[i].z < 250) ? 2 : 1;
    if (px < 0 || px + max_size > (int)W || py < 0 || py + max_size > (int)H) {
      stars[i].x = ((int32_t)next_rand() % 2000) - 1000;
      stars[i].y = ((int32_t)next_rand() % 2000) - 1000;
      stars[i].z = 1000;
      stars[i].prev_px = -1;
      stars[i].prev_py = -1;
      continue;
    }

    /* 5. Draw if allowed */
    int in_arena = (px >= x_off && px < x_off + grid_w && py >= y_off &&
                    py < y_off + grid_h);

    /* On start screen, stars are visible everywhere. During gameplay, keep them
     * outside the arena */
    if (start_screen || !in_arena) {
      int brightness = 255 - (stars[i].z * 180) / 1000;
      if (brightness < 80)
        brightness = 80;
      if (brightness > 255)
        brightness = 255;

      /* Closer stars are drawn as 2x2 blocks for enhanced visibility and depth
       */
      int size = (stars[i].z < 250) ? 2 : 1;

      if (size == 2) {
        put_pixel(ctx, px, py, brightness, brightness, brightness);
        put_pixel(ctx, px + 1, py, brightness, brightness, brightness);
        put_pixel(ctx, px, py + 1, brightness, brightness, brightness);
        put_pixel(ctx, px + 1, py + 1, brightness, brightness, brightness);
      } else {
        put_pixel(ctx, px, py, brightness, brightness, brightness);
      }
      stars[i].prev_px = px;
      stars[i].prev_py = py;
      stars[i].prev_size = size;
    } else {
      stars[i].prev_px = -1;
      stars[i].prev_py = -1;
    }
  }
}
