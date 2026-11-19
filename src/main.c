#include <avr/io.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdio.h>
#include <util/delay.h>
#include <util/twi.h>
#include "output.h"

#include <ssd1306/ssd1306.h>
#include <ssd1306/util.h>
#include <ssd1306/i2c.h>

int main(void) {
  uart_init();
  uart_transmit("STARTING...\r\n");

  i2c_init();
  ssd1306_init();
  ssd1306_fill_gddram(0, SSD1306_N_PAGES, 0, SSD1306_N_COLS, 0);

  // debug led
  DDRB |= (2 << DDB1);
  uint8_t state = 0;

  uint8_t i = 0;

  uint8_t sx = 0;
  uint8_t sy = 0;
  uint8_t c = '1';

  while (1) {
    if (state) {
      PORTB &= ~(1 << PB1);
    } else {
      PORTB |= (1 << PB1);
    }
    state = !state;
    i++;

    if (i >= 5) {
      uint32_t idx = FONT_FONTNAME_CIDX(c);
      uint32_t off = pgm_read_dword(FONT_FONTNAME_OFFSETS + idx);
      uint8_t w = pgm_read_byte(FONT_FONTNAME_WIDTHS + idx);
      const uint8_t *fontDataP = FONT_FONTNAME_GBUF + off;
      if (sx + w > 128) {
        sy += FONT_FONTNAME_NUM_PAGES;
        sx = 0;
      }
      if (sy >= 8) {
        ssd1306_fill_gddram(0, SSD1306_N_PAGES, 0, SSD1306_N_COLS, 0);
        sy = 0;
        sx = 0;
      }
      ssd1306_write_gddram(
        sy, sy + FONT_FONTNAME_NUM_PAGES, sx, sx + w,
        fontDataP, w * FONT_FONTNAME_NUM_PAGES, SSD1306_F_PGM
      );

      sx += w;
      c++;
      if (c > 126) {
        c = 33;
      }
    }

    _delay_ms(500);
  }
}
