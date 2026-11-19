#include <avr/io.h>
#include <util/delay.h>
#include <ssd1306/ssd1306.h>
#include <ssd1306/i2c.h>

// TODO: remove LED code
// summary: a block is positoned somehwere, moves around and bounces off of the walls


// don't change these
#define BLOCK_PAGES (1u)
#define BLOCK_COLUMNS (8u)
// these can be changed
#define BLOCK_VEL (1)
#define DELAY_MS 20

struct block_state {
    int16_t x, y, velx, vely;
} block;

void init_block() {
    block.velx = 1;
    block.vely = 1;
    block.x = 0;
    block.y = 0;
}

void update_block() {
    static const int8_t
        minx = 0, maxx = SSD1306_N_COLS - BLOCK_COLUMNS,
        miny = 0, maxy = (SSD1306_N_PAGES - BLOCK_PAGES) * 8;
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
void draw_block(uint8_t page) {
    if (block.y % 8 == 0) {
        // block is page-aligned
        uint8_t y = block.y / 8;
        ssd1306_fill_gddram(
            y, y + BLOCK_PAGES, block.x, block.x + BLOCK_COLUMNS, page
        );
    } else {
        // block is spread across two pages
        uint8_t rem = block.y % 8;
        uint8_t page1 = block.y / 8;
        uint8_t page2 = page1 + 1;
        uint8_t page1Val = page << rem;
        uint8_t page2Val = page >> (8 - rem);
        ssd1306_fill_gddram(
            page1, page1 + 1, block.x, block.x + BLOCK_COLUMNS, page1Val
        );
        ssd1306_fill_gddram(
            page2, page2 + 1, block.x, block.x + BLOCK_COLUMNS, page2Val
        );
    }
}

int main(void) {

    DDRB |= (1 << DDB1);

    uint8_t state = 0;

    i2c_init();
    ssd1306_init();
    init_block();
    
    // clear the whole display
    ssd1306_fill_gddram(0, SSD1306_N_PAGES, 0, SSD1306_N_COLS, 0);

    while (1) {
        if (state) {
            PORTB |= (1 << PB1);
        } else {
            PORTB &= ~(1 << PB1);
        }
        state = !state;
        draw_block(0);
        update_block();
        draw_block(255);
        _delay_ms(DELAY_MS);
    }

}