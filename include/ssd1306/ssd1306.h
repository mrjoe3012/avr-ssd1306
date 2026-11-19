#ifndef SSD1306_H_
#define SSD1306_H_

#include <stdint.h>

#ifndef SSD1306_ADDRESS
#define SSD1306_ADDRESS   (0b0111100)
#endif

#ifndef SSD1306_N_PAGES
#define SSD1306_N_PAGES   (8u)
#endif

#ifndef SSD1306_N_COLS
#define SSD1306_N_COLS    (128u)
#endif

#define SSD1306_F_PGM     (1 << 0)

void ssd1306_init();
void ssd1306_write_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                          uint8_t colStartIdx, uint8_t colEndIdx,
                          const uint8_t *data, uint32_t n, uint8_t flags);
void ssd1306_fill_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                         uint8_t colStartIdx, uint8_t colEndIdx, uint8_t value);

#endif // SSD1306
