#include <drivers/console.h>
#include <stdint.h>
#include <lib/string.h>

/* Linker symbols for the PSF2 font. */
extern char _binary_default8x16_psfu_start[];
extern char _binary_default8x16_psfu_end[];

/* Internal console context. */
static struct console_ctx ctx;

/* Returns the framebuffer address for the given coordinates. */
static inline uint32_t *fb_get_pixel(uint32_t x, uint32_t y) {
  return &ctx.fb[(y * ctx.pitch) + x];
}

/* Draws the given ASCII char on screen. */
static inline void draw_char(unsigned char c, uint32_t x, uint32_t y) {
  uint8_t *glyph = (uint8_t*)ctx.glyph_data + (c * ctx.font->bytesperglyph);

  for (uint32_t gy = 0; gy < ctx.font->height; gy++) {
    uint8_t row = glyph[gy];
    for (uint32_t gx = 0; gx < ctx.font->width; gx++) {
      if (row & 0b10000000 >> gx) {
        *fb_get_pixel(x + gx, y + gy) = ctx.fg_col;
      }
    }
  }
}

static void console_scroll() {
  ctx.cursor_x = 0;
  ctx.cursor_y = 0;
  memset(ctx.fb, 0, ctx.width * ctx.height * ctx.pitch);
}

void console_init(struct console_fb_info info) {
  ctx.fb = (uint32_t*)info.fb_addr;
  ctx.height = info.heigth;
  ctx.width = info.width;
  ctx.pitch = info.pitch / 4;
  
  ctx.font = (psf2_hdr*)_binary_default8x16_psfu_start;
  ctx.glyph_data = (void*)ctx.font + ctx.font->headersize;

  ctx.cursor_x = 0;
  ctx.cursor_y = 0;
  ctx.fg_col = 0x00FF00;
}

void console_set_color(uint32_t bg, uint32_t fg) {
  ctx.bg_col = bg;
  ctx.fg_col = fg;
}

void console_putchar(char c) {
  switch (c) {
    case '\n': {
      ctx.cursor_x = 0;
      ctx.cursor_y += ctx.font->height;
      if (ctx.cursor_y > ctx.height) {
        console_scroll();
      }
      break;
    }
    default: {
      if (ctx.cursor_x >= ctx.width) {
        ctx.cursor_x = 0;
        ctx.cursor_y += ctx.font->height;
      }
      if (ctx.cursor_y >= ctx.height) {
        console_scroll();
      }
      draw_char(c, ctx.cursor_x, ctx.cursor_y);
      ctx.cursor_x += ctx.font->width;
      break;
    }
  }
}

void console_putstr(const char *s) {
  while (*s != 0) {
    console_putchar(*s++);
  }
}
