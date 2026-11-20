# ssd1306

A small C library for driving SSD1306 OLED displays from AVR microcontrollers over I2C.

The library is deliberately minimal. It does not try to wrap every feature of the SSD1306 (scrolling, for example, is not supported). It gives you a simple way to write to the display's graphics memory (GDDRAM), either from a buffer or by filling an area with a fixed value, and leaves the rest up to you.

Tested on the ATmega328P. It should work on other AVR microcontrollers with a built-in TWI peripheral, but I haven't tried any.

## Features

- Initialise the display
- Write a buffer to any rectangular area of GDDRAM
- Fill any rectangular area of GDDRAM with a single value
- Source buffers can live in RAM or in flash (PROGMEM)
- Built-in mini I2C driver using the hardware TWI peripheral
- Optional interrupt driven I2C, which puts the CPU to sleep while waiting for transactions to complete

## Requirements

- `avr-gcc` and [`avr-libc`](https://avrdudes.github.io/avr-libc/) (this is where headers like `avr/io.h` come from)
- A microcontroller with a TWI peripheral
- An SSD1306 display connected over I2C

No other dependencies.

## Getting started

Copy the library into your project (or add it as a git submodule), add the `include` directory to your include path and compile the sources in `src` along with your own.

`F_CPU` must be defined, since the I2C clock speed is calculated from it. The build will fail with an error if it isn't.

A minimal program looks like this:

```c
#include <avr/io.h>
#include <ssd1306/i2c.h>
#include <ssd1306/ssd1306.h>

int main(void) {
  if(i2c_init(I2C_CLOCK_SPEED_STANDARD) != I2C_R_OK) {
    // handle error
  }
  if (!ssd1306_init()) {
    // handle error
  }

  // clear the whole display
  if (!ssd1306_fill_gddram(0, SSD1306_N_PAGES, 0, SSD1306_N_COLS, 1)) {
    // handle error
  }

  while (1) {
  }
}
```

**IT IS HIGHLY RECOMMENDED TO SETUP THE WTD AS I2C TRANSACTIONS MAY BLOCK INDEFINITELY!**

## Usage

### Display memory layout

The SSD1306 splits its GDDRAM into pages. Each page is 8 pixels tall, and each byte in a page is a vertical strip of 8 pixels (one column). A 128x64 display has 8 pages of 128 columns.

Both functions below take a page range and a column range. The end indices are exclusive, so the whole display is pages `0` to `SSD1306_N_PAGES` and columns `0` to `SSD1306_N_COLS`.

### `ssd1306.h`

```c
bool ssd1306_init();
```

Initialises the display. Returns `true` on success. Call `i2c_init` first.

```c
bool ssd1306_write_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                          uint8_t colStartIdx, uint8_t colEndIdx,
                          const uint8_t *data, uint32_t n, uint8_t flags);
```

Writes `n` bytes from `data` to the given area. Data is laid out one page after another, with each page being `colEndIdx - colStartIdx` bytes wide. Returns `true` on success.

Pass `SSD1306_F_PGM` in `flags` if `data` is stored in flash (PROGMEM), otherwise pass `0`.

```c
bool ssd1306_fill_gddram(uint8_t pageStartIdx, uint8_t pageEndIdx,
                         uint8_t colStartIdx, uint8_t colEndIdx, uint8_t value);
```

Fills the given area with `value`. Use `0` to clear and `255` to light every pixel. Returns `true` on success.

### `i2c.h`

The I2C driver is used by the SSD1306 code, but you can use it directly if you need to talk to other devices on the bus.

```c
I2CRetCode i2c_init(uint32_t clock_speed);
I2CRetCode i2c_start();
void       i2c_stop();
I2CRetCode i2c_send_byte(uint8_t byte);
I2CRetCode i2c_send_bytes(const uint8_t *data, uint32_t n);
```

`clock_speed` is in Hz. `I2C_CLOCK_SPEED_STANDARD` (100 kHz) and `I2C_CLOCK_SPEED_FAST` (400 kHz) are provided. Functions return an `I2CRetCode`:

| Code | Meaning |
| --- | --- |
| `I2C_R_OK` | Success |
| `I2C_R_CLOCK_SPEED_IMPOSSIBLE` | The requested clock speed can't be reached with the current `F_CPU` |
| `I2C_R_FAILED` | The transaction failed |

### `util.h`

Two small UART helpers, used by the examples for debug output.

```c
void uart_init();
void uart_transmit(const char *message);
```

The default baud rate is 9600.

## Configuration

These can be defined at compile time (for example with `-DNAME=value`).

| Macro | Default | Description |
| --- | --- | --- |
| `F_CPU` | none, required | CPU clock frequency in Hz |
| `I2C_USE_INTERRUPT` | `0` | `0` busy waits on I2C transactions. `1` defines an ISR for the TWI interrupt and sleeps the CPU while waiting. If you set this to `1`, you must enable interrupts yourself (`sei()`) |
| `SSD1306_ADDRESS` | `0b0111100` (0x3C) | I2C address of the display |
| `SSD1306_N_PAGES` | `8` | Number of pages on the display |
| `SSD1306_N_COLS` | `128` | Number of columns on the display |
| `UART_BAUD` | `9600` | Baud rate used by `util.h` |

## Examples

Two examples are included in the `examples` directory. Both expect the display on the I2C bus and, optionally, an LED on `PB1`, which toggles every frame as a heartbeat.

### block

`block.c` draws an 8x8 block and bounces it around the display. It only sends the part of the screen around the block to the display each frame rather than the whole thing, which keeps the I2C traffic down. It also sets up the watchdog timer and uses the UART for error messages, so connect a serial adapter if you want to see them.

### text

`text.c` renders text on the display, cycling through the printable ASCII characters and wrapping at the end of each line. The glyphs are stored in flash and written to the display with `SSD1306_F_PGM`.

The font data was generated with [font_rasterizer](https://github.com/mrjoe3012/font_rasterizer), which rasterizes a font and outputs it as a C buffer that can go straight into the ATmega328's flash memory. The font in the example is DejaVu Sans. The copyright for the font belongs to its original creators, see the [DejaVu fonts license](https://dejavu-fonts.github.io/License.html).

## Building

A GitHub Action builds the library on every push and pull request to make sure everything still compiles. There are no prebuilt release artifacts, so you will need to build it as part of your own project.

## Contributing

Contributions are welcome. The usual workflow is fine:

1. Fork the repository
2. Create a branch for your change
3. Open a pull request

Please make sure the build action passes.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
