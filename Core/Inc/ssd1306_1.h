#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f1xx_hal.h"  /* change to match your HAL header */
#include <stdint.h>
#include <string.h>

/* ── Physical display ─────────────────────────────────── */
#define SSD1306_PHY_WIDTH    128
#define SSD1306_PHY_HEIGHT    64
#define SSD1306_PHY_PAGES      8   /* 64 / 8 */
#define SSD1306_BUF_SZ       (SSD1306_PHY_WIDTH * SSD1306_PHY_PAGES)  /* 1024 */

/* ── Logical canvas after 90° CW rotation ────────────── */
/*    X: 0‥63   Y: 0‥127                                  */
#define SSD1306_LOG_WIDTH     64
#define SSD1306_LOG_HEIGHT   128

/* ── I2C ──────────────────────────────────────────────── */
#define SSD1306_I2C_ADDR     (0x3C << 1)
#define SSD1306_I2C_TIMEOUT   100

/* ── Font ─────────────────────────────────────────────── */
#define SSD1306_FONT_FIRST_CHAR  0x20
#define SSD1306_FONT_LAST_CHAR   0x7E
#define SSD1306_FONT_CHAR_WIDTH     7
#define SSD1306_FONT_CHAR_HEIGHT    12   /* actual pixel rows used */
#define SSD1306_FONT_CHAR_STRIDE    6   /* 5px glyph + 1px gap   */

/* ── Commands ─────────────────────────────────────────── */
#define SSD1306_CMD_COLUMNADDR  0x21
#define SSD1306_CMD_PAGEADDR    0x22

typedef enum { SSD1306_BLACK = 0, SSD1306_WHITE = 1 } SSD1306_Color;
typedef enum { SSD1306_OK = 0,    SSD1306_ERR  = 1 } SSD1306_Status;

/* ── Public API ───────────────────────────────────────── */
SSD1306_Status SSD1306_Init   (I2C_HandleTypeDef *hi2c);
void           SSD1306_Clear  (SSD1306_Color color);
SSD1306_Status SSD1306_Update (void);

/* Coordinate space: logical 64×128 (portrait) */
void     SSD1306_DrawPixel  (uint8_t x, uint8_t y, SSD1306_Color color);
uint8_t  SSD1306_DrawChar   (uint8_t x, uint8_t y, char c,
                              SSD1306_Color color, uint8_t scale);
void     SSD1306_DrawString (uint8_t x, uint8_t y, const char *str,
                              SSD1306_Color color, uint8_t scale);

#endif /* SSD1306_H */
