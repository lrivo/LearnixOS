#pragma once

#include <psf2.h>
#include <stdint.h>

/* Bootloader indipendent struct with framebuffer config. */
struct console_fb_info {
  void *fb_addr;
  uint32_t width;
  uint32_t heigth;
  uint32_t pitch;
};

/* Internal context of the terminal. */
struct console_ctx {
  /* framebuffer HW info taken from Limine. */
  uint32_t *fb;
  uint32_t width;
  uint32_t height;
  uint32_t pitch;

  /* font */
  psf2_hdr *font;      // psf2 font header
  void *glyph_data;    // start of the first printable glyph
  
  /* terminal state. */
  uint32_t cursor_x;
  uint32_t cursor_y;
  uint32_t cols;      // width / font_width
  uint32_t rows;      // heigth / font_height
  uint32_t fg_col;    // RGB foreground color
  uint32_t bg_col;    // RGB background color

};

void console_init(struct console_fb_info info);

void console_set_color(uint32_t bg, uint32_t fg);

void console_putchar(char c);

void console_putstr(const char *s);
