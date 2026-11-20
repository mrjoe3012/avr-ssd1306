/**
 * @file block.c
 * @author Joseph Agrane
 * @brief An example using the SSD1306 with the atmega328p. Connect I2C bus to
 * display and optionally an LED to PB1
 *
 */
#include <avr/io.h>
#include <ssd1306/i2c.h>
#include <ssd1306/ssd1306.h>
#include <ssd1306/util.h>
#include <string.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// don't change these
#define BLOCK_PAGES (1u)
#define BLOCK_COLUMNS (8u)
// these can be changed
#define BLOCK_VEL (1)
#define DELAY_MS (32)

char buffer[128];

#define HANDLE_ERROR(...)                                                      \
  {                                                                            \
    snprintf(buffer, sizeof(buffer), __VA_ARGS__);                             \
    uart_transmit(buffer);                                                     \
    while (1)                                                                  \
      ;                                                                        \
  }

struct block_state {
  int16_t x, y, velx, vely;
} block;

uint8_t framebuf[SSD1306_N_PAGES * SSD1306_N_COLS];

void init_block() {
  block.velx = BLOCK_VEL;
  block.vely = BLOCK_VEL;
  block.x = 0;
  block.y = 0;
}

void update_block() {
  static const int8_t minx = 0, maxx = SSD1306_N_COLS - BLOCK_COLUMNS, miny = 0,
                      maxy = (SSD1306_N_PAGES - BLOCK_PAGES) * 8;
  block.x += block.velx;
  block.y += block.vely;
  if (block.x < minx) {
    block.x = minx;
    block.velx = BLOCK_VEL;
  } else if (block.x > maxx) {
    block.x = maxx;
    block.velx = -BLOCK_VEL;
  }
  if (block.y < miny) {
    block.y = miny;
    block.vely = BLOCK_VEL;
  } else if (block.y > maxy) {
    block.y = maxy;
    block.vely = -BLOCK_VEL;
  }
}

// can pass 0 for page to clear block, or 255 to fill it
void draw_block(uint8_t page_val) {
  if (block.y % 8 == 0) {
    // block is page-aligned
    uint8_t y = block.y / 8;
    memset(framebuf + y * SSD1306_N_COLS + block.x, page_val,
           BLOCK_PAGES * BLOCK_COLUMNS);
  } else {
    // block is spread across two pages
    uint8_t rem = block.y % 8;
    uint8_t page = block.y / 8;
    uint8_t page1Val = page_val << rem;
    uint8_t page2Val = page_val >> (8 - rem);
    memset(framebuf + page * SSD1306_N_COLS + block.x, page1Val,
           BLOCK_PAGES * BLOCK_COLUMNS);
    memset(framebuf + (page + 1) * SSD1306_N_COLS + block.x, page2Val,
           BLOCK_PAGES * BLOCK_COLUMNS);
  }
}

ISR(WDT_vect) {
    uart_transmit("MCU TIMED OUT. RESETTING...\r\n");
}

int main(void) {

  // setup wdt reset
  wdt_reset();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDE) | (1 << WDIE) | (1 << WDP3) | (1 << WDP0);
  sei();

  DDRB |= (1 << DDB1);

  uint8_t state = 0;

  i2c_init(I2C_CLOCK_SPEED_STANDARD);
  uart_init();
  if (!ssd1306_init()) {
    HANDLE_ERROR("Unable to initialise ssd1306.\r\n");
  }
  init_block();

  // clear the whole display
  if (!ssd1306_fill_gddram(0, SSD1306_N_PAGES, 0, SSD1306_N_COLS, 0)) {
    HANDLE_ERROR("Unable to perform initial display clear on ssd1q306.\r\n");
  }

  while (1) {
    wdt_reset();
    if (state) {
      PORTB |= (1 << PB1);
    } else {
      PORTB &= ~(1 << PB1);
    }
    state = !state;
    draw_block(0);
    int16_t x1 = block.x, y1 = block.y / 8;
    update_block();
    int16_t x2 = block.x, y2 = block.y / 8;
    int16_t minx, maxx, miny, maxy;
    if (x1 < x2) {
      minx = x1;
      maxx = x2;
    } else {
      minx = x2;
      maxx = x1;
    }
    if (y1 < y2) {
      miny = y1;
      maxy = y2;
    } else {
      miny = y2;
      maxy = y1;
    }
    draw_block(255);
    // update the display only near the block
    for (uint8_t page = miny; page < maxy + 2 && page < SSD1306_N_PAGES;
         page++) {
      if (!ssd1306_write_gddram(page, page + 1, minx, maxx + BLOCK_COLUMNS,
                                framebuf + page * SSD1306_N_COLS + minx,
                                maxx - minx + BLOCK_COLUMNS, 0)) {
        uart_transmit("Failed to write to gddram!\r\n");
      }
    }
    _delay_ms(DELAY_MS);
  }
}