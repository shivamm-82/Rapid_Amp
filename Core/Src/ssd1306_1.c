#include "ssd1306_1.h"

/* ═══════════════════════════════════════════════════════════
 *  5×7 font  (32 ' ' … 126 '~')
 *  Each entry is 5 bytes = 5 columns, bits 0‥6 = rows 0‥6
 * ═══════════════════════════════════════════════════════════ */
static const uint8_t SSD1306_Font5x7[][SSD1306_FONT_CHAR_WIDTH] = {
    {0x00,0x00,0x00,0x00,0x00}, /* ' ' */
    {0x00,0x00,0x5F,0x00,0x00}, /* '!' */
    {0x00,0x07,0x00,0x07,0x00}, /* '"' */
    {0x14,0x7F,0x14,0x7F,0x14}, /* '#' */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* '$' */
    {0x23,0x13,0x08,0x64,0x62}, /* '%' */
    {0x36,0x49,0x55,0x22,0x50}, /* '&' */
    {0x00,0x05,0x03,0x00,0x00}, /* ''' */
    {0x00,0x1C,0x22,0x41,0x00}, /* '(' */
    {0x00,0x41,0x22,0x1C,0x00}, /* ')' */
    {0x08,0x2A,0x1C,0x2A,0x08}, /* '*' */
    {0x08,0x08,0x3E,0x08,0x08}, /* '+' */
    {0x00,0x50,0x30,0x00,0x00}, /* ',' */
    {0x08,0x08,0x08,0x08,0x08}, /* '-' */
    {0x00,0x60,0x60,0x00,0x00}, /* '.' */
    {0x20,0x10,0x08,0x04,0x02}, /* '/' */
    {0x3E,0x51,0x49,0x45,0x3E}, /* '0' */
    {0x00,0x42,0x7F,0x40,0x00}, /* '1' */
    {0x42,0x61,0x51,0x49,0x46}, /* '2' */
    {0x21,0x41,0x45,0x4B,0x31}, /* '3' */
    {0x18,0x14,0x12,0x7F,0x10}, /* '4' */
    {0x27,0x45,0x45,0x45,0x39}, /* '5' */
    {0x3C,0x4A,0x49,0x49,0x30}, /* '6' */
    {0x01,0x71,0x09,0x05,0x03}, /* '7' */
    {0x36,0x49,0x49,0x49,0x36}, /* '8' */
    {0x06,0x49,0x49,0x29,0x1E}, /* '9' */
    {0x00,0x36,0x36,0x00,0x00}, /* ':' */
    {0x00,0x56,0x36,0x00,0x00}, /* ';' */
    {0x08,0x14,0x22,0x41,0x00}, /* '<' */
    {0x14,0x14,0x14,0x14,0x14}, /* '=' */
    {0x00,0x41,0x22,0x14,0x08}, /* '>' */
    {0x02,0x01,0x51,0x09,0x06}, /* '?' */
    {0x32,0x49,0x79,0x41,0x3E}, /* '@' */
    {0x7E,0x11,0x11,0x11,0x7E}, /* 'A' */
    {0x7F,0x49,0x49,0x49,0x36}, /* 'B' */
    {0x3E,0x41,0x41,0x41,0x22}, /* 'C' */
    {0x7F,0x41,0x41,0x22,0x1C}, /* 'D' */
    {0x7F,0x49,0x49,0x49,0x41}, /* 'E' */
    {0x7F,0x09,0x09,0x09,0x01}, /* 'F' */
    {0x3E,0x41,0x49,0x49,0x7A}, /* 'G' */
    {0x7F,0x08,0x08,0x08,0x7F}, /* 'H' */
    {0x00,0x41,0x7F,0x41,0x00}, /* 'I' */
    {0x20,0x40,0x41,0x3F,0x01}, /* 'J' */
    {0x7F,0x08,0x14,0x22,0x41}, /* 'K' */
    {0x7F,0x40,0x40,0x40,0x40}, /* 'L' */
    {0x7F,0x02,0x0C,0x02,0x7F}, /* 'M' */
    {0x7F,0x04,0x08,0x10,0x7F}, /* 'N' */
    {0x3E,0x41,0x41,0x41,0x3E}, /* 'O' */
    {0x7F,0x09,0x09,0x09,0x06}, /* 'P' */
    {0x3E,0x41,0x51,0x21,0x5E}, /* 'Q' */
    {0x7F,0x09,0x19,0x29,0x46}, /* 'R' */
    {0x46,0x49,0x49,0x49,0x31}, /* 'S' */
    {0x01,0x01,0x7F,0x01,0x01}, /* 'T' */
    {0x3F,0x40,0x40,0x40,0x3F}, /* 'U' */
    {0x1F,0x20,0x40,0x20,0x1F}, /* 'V' */
    {0x3F,0x40,0x38,0x40,0x3F}, /* 'W' */
    {0x63,0x14,0x08,0x14,0x63}, /* 'X' */
    {0x07,0x08,0x70,0x08,0x07}, /* 'Y' */
    {0x61,0x51,0x49,0x45,0x43}, /* 'Z' */
    {0x00,0x7F,0x41,0x41,0x00}, /* '[' */
    {0x02,0x04,0x08,0x10,0x20}, /* '\' */
    {0x00,0x41,0x41,0x7F,0x00}, /* ']' */
    {0x04,0x02,0x01,0x02,0x04}, /* '^' */
    {0x40,0x40,0x40,0x40,0x40}, /* '_' */
    {0x00,0x01,0x02,0x04,0x00}, /* '`' */
    {0x20,0x54,0x54,0x54,0x78}, /* 'a' */
    {0x7F,0x48,0x44,0x44,0x38}, /* 'b' */
    {0x38,0x44,0x44,0x44,0x20}, /* 'c' */
    {0x38,0x44,0x44,0x48,0x7F}, /* 'd' */
    {0x38,0x54,0x54,0x54,0x18}, /* 'e' */
    {0x08,0x7E,0x09,0x01,0x02}, /* 'f' */
    {0x0C,0x52,0x52,0x52,0x3E}, /* 'g' */
    {0x7F,0x08,0x04,0x04,0x78}, /* 'h' */
    {0x00,0x44,0x7D,0x40,0x00}, /* 'i' */
    {0x20,0x40,0x44,0x3D,0x00}, /* 'j' */
    {0x7F,0x10,0x28,0x44,0x00}, /* 'k' */
    {0x00,0x41,0x7F,0x40,0x00}, /* 'l' */
    {0x7C,0x04,0x18,0x04,0x78}, /* 'm' */
    {0x7C,0x08,0x04,0x04,0x78}, /* 'n' */
    {0x38,0x44,0x44,0x44,0x38}, /* 'o' */
    {0x7C,0x14,0x14,0x14,0x08}, /* 'p' */
    {0x08,0x14,0x14,0x14,0x7C}, /* 'q' */
    {0x7C,0x08,0x04,0x04,0x08}, /* 'r' */
    {0x48,0x54,0x54,0x54,0x20}, /* 's' */
    {0x04,0x3F,0x44,0x40,0x20}, /* 't' */
    {0x3C,0x40,0x40,0x40,0x7C}, /* 'u' */
    {0x1C,0x20,0x40,0x20,0x1C}, /* 'v' */
    {0x3C,0x40,0x30,0x40,0x3C}, /* 'w' */
    {0x44,0x28,0x10,0x28,0x44}, /* 'x' */
    {0x0C,0x50,0x50,0x50,0x3C}, /* 'y' */
    {0x44,0x64,0x54,0x4C,0x44}, /* 'z' */
    {0x00,0x08,0x36,0x41,0x00}, /* '{' */
    {0x00,0x00,0x7F,0x00,0x00}, /* '|' */
    {0x00,0x41,0x36,0x08,0x00}, /* '}' */
    {0x08,0x08,0x2A,0x1C,0x08}, /* '~' */
};

/* ═══════════════════════════════════════════════════════════
 *  Private state
 * ═══════════════════════════════════════════════════════════ */
static I2C_HandleTypeDef *s_hi2c  = NULL;
static uint8_t            s_fb[SSD1306_BUF_SZ];   /* 1024 bytes */
static uint8_t            s_tx[1 + SSD1306_PHY_WIDTH]; /* chunk buf */

/* ── low-level helpers ──────────────────────────────────── */
static SSD1306_Status send_cmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    return (HAL_I2C_Master_Transmit(s_hi2c, SSD1306_I2C_ADDR,
                                    buf, 2, SSD1306_I2C_TIMEOUT) == HAL_OK)
           ? SSD1306_OK : SSD1306_ERR;
}

static SSD1306_Status send_cmds(const uint8_t *p, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
        if (send_cmd(p[i]) != SSD1306_OK) return SSD1306_ERR;
    return SSD1306_OK;
}

/* ═══════════════════════════════════════════════════════════
 *  Init
 * ═══════════════════════════════════════════════════════════ */
SSD1306_Status SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;
    HAL_Delay(10);

    static const uint8_t init_seq[] = {
        0xAE,        /* display off                  */
        0xD5, 0x80,  /* clock div / osc freq         */
        0xA8, 0x3F,  /* multiplex ratio: 63 (64 rows)*/
        0xD3, 0x00,  /* display offset: 0            */
        0x40,        /* start line: 0                */
        0x8D, 0x14,  /* charge pump ON               */
        0x20, 0x00,  /* horizontal addressing mode   */
        0xA1,        /* segment remap (normal)       */
        0xC8,        /* COM scan: top→bottom         */
        0xDA, 0x12,  /* COM pins config (64-row)     */
        0x81, 0xCF,  /* contrast                     */
        0xD9, 0xF1,  /* pre-charge period            */
        0xDB, 0x40,  /* VCOMH deselect level         */
        0xA4,        /* display RAM content          */
        0xA6,        /* normal (non-inverted)        */
        0xAF,        /* display ON                   */
    };

    SSD1306_Status st = send_cmds(init_seq, sizeof(init_seq));
    if (st != SSD1306_OK) return st;

    SSD1306_Clear(SSD1306_BLACK);
    return SSD1306_Update();
}

/* ═══════════════════════════════════════════════════════════
 *  Clear framebuffer
 * ═══════════════════════════════════════════════════════════ */
void SSD1306_Clear(SSD1306_Color color)
{
    memset(s_fb, (color == SSD1306_WHITE) ? 0xFF : 0x00, SSD1306_BUF_SZ);
}

/* ═══════════════════════════════════════════════════════════
 *  DrawPixel  — logical 90°CW space (x: 0‥63, y: 0‥127)
 *
 *  90° CW transform:
 *    phys_x = (PHY_HEIGHT - 1) - log_y   →  (63 - y)
 *    phys_y = log_x
 * ═══════════════════════════════════════════════════════════ */
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_Color color)
{
    // Logical canvas: 64 wide x 128 tall (portrait)
    // Physical display: 128 wide x 64 tall (landscape)
    // 90°CW transform: phys_x = y, phys_y = (LOG_WIDTH-1) - x
    if (x >= SSD1306_LOG_WIDTH || y >= SSD1306_LOG_HEIGHT) return;

    uint8_t  phys_x = y;
    uint8_t  phys_y = (SSD1306_LOG_WIDTH - 1u) - x;

    uint16_t index  = (phys_y / 8u) * SSD1306_PHY_WIDTH + phys_x;
    uint8_t  bit    = phys_y % 8u;

    if (color == SSD1306_WHITE)
        s_fb[index] |=  (uint8_t)(1u << bit);
    else
        s_fb[index] &= (uint8_t)~(1u << bit);
}

/* ═══════════════════════════════════════════════════════════
 *  DrawChar  — returns next X position
 *  Logical canvas: x 0‥63, y 0‥127
 *  Characters grow downward (+Y), stride rightward (+X per col)
 * ═══════════════════════════════════════════════════════════ */
// DrawChar — back to landscape mode (col→X, row→Y)
uint8_t SSD1306_DrawChar(uint8_t x, uint8_t y, char c,
                          SSD1306_Color color, uint8_t scale)
{
    if (c < SSD1306_FONT_FIRST_CHAR || c > SSD1306_FONT_LAST_CHAR) c = '?';

    const uint8_t *glyph =
        SSD1306_Font5x7[(uint8_t)(c - SSD1306_FONT_FIRST_CHAR)];

    for (uint8_t col = 0; col < SSD1306_FONT_CHAR_WIDTH; col++) {
        uint8_t col_data = glyph[col];
        for (uint8_t row = 0; row < SSD1306_FONT_CHAR_HEIGHT; row++) {
            SSD1306_Color px = (col_data & (1u << row))
                               ? color
                               : (SSD1306_Color)(color ^ 1u);
            for (uint8_t sx = 0; sx < scale; sx++)
                for (uint8_t sy = 0; sy < scale; sy++)
                    SSD1306_DrawPixel(
                        (uint8_t)(x + col * scale + sx),  // col → X
                        (uint8_t)(y + row * scale + sy),  // row → Y
                        px);
        }
    }

    /* gap column */
    for (uint8_t row = 0; row < SSD1306_FONT_CHAR_HEIGHT * scale; row++)
        for (uint8_t sx = 0; sx < scale; sx++)
            SSD1306_DrawPixel(
                (uint8_t)(x + SSD1306_FONT_CHAR_WIDTH * scale + sx),
                (uint8_t)(y + row),
                (SSD1306_Color)(color ^ 1u));

    return (uint8_t)(x + SSD1306_FONT_CHAR_STRIDE * scale);  // returns next X
}
/* ═══════════════════════════════════════════════════════════
 *  DrawString  — wraps on Y axis (down the portrait screen)
 *  x = column (0‥63), y = row start (0‥127)
 * ═══════════════════════════════════════════════════════════ */
void SSD1306_DrawString(uint8_t x, uint8_t y, const char *str,
                         SSD1306_Color color, uint8_t scale)
{
    if (!str) return;
    uint8_t cx = x;

    while (*str) {
        // wrap to next line when hitting right edge
        if (cx + SSD1306_FONT_CHAR_STRIDE * scale > SSD1306_LOG_WIDTH) {
            cx  = 0;
            y  += SSD1306_FONT_CHAR_HEIGHT * scale + 1;
        }
        if (y + SSD1306_FONT_CHAR_HEIGHT * scale > SSD1306_LOG_HEIGHT) break;

        cx = SSD1306_DrawChar(cx, y, *str++, color, scale);
    }
}

/* ═══════════════════════════════════════════════════════════
 *  Update — flush framebuffer to display, page by page
 * ═══════════════════════════════════════════════════════════ */
SSD1306_Status SSD1306_Update(void)
{
    const uint8_t addr_cmds[] = {
        SSD1306_CMD_COLUMNADDR, 0x00, SSD1306_PHY_WIDTH  - 1u,
        SSD1306_CMD_PAGEADDR,   0x00, SSD1306_PHY_PAGES  - 1u,
    };
    SSD1306_Status st = send_cmds(addr_cmds, sizeof(addr_cmds));
    if (st != SSD1306_OK) return st;

    for (uint8_t page = 0; page < SSD1306_PHY_PAGES; page++) {
        s_tx[0] = 0x40;
        memcpy(&s_tx[1], &s_fb[page * SSD1306_PHY_WIDTH], SSD1306_PHY_WIDTH);
        if (HAL_I2C_Master_Transmit(s_hi2c, SSD1306_I2C_ADDR,
                                    s_tx,
                                    (uint16_t)(1u + SSD1306_PHY_WIDTH),
                                    SSD1306_I2C_TIMEOUT) != HAL_OK)
            return SSD1306_ERR;
    }
    return SSD1306_OK;
}
