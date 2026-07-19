/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** TimestampOverlay.h
**
** Burn a localtime timestamp string into raw YUV video frames.
** Supports: NV12, NV21, YUV420, YVU420, YUYV, UYVY.
** H264/H265 compressed frames are silently skipped.
**
** -------------------------------------------------------------------------*/

#pragma once

#include <ctime>
#include <cstring>
#include <stdint.h>
#include <sys/time.h>
#include <linux/videodev2.h>

namespace TimestampOverlay {

// --- Configuration ----------------------------------------------------------

// Scale factor (1 = 8×8 px per glyph, 2 = 16×16 px per glyph, …)
static constexpr int SCALE  = 2;
static constexpr int CHAR_W = 8 * SCALE;
static constexpr int CHAR_H = 8 * SCALE;
// Pixel margin around the text block
static constexpr int MARGIN = 4;
// YUV luma values: near-black background, near-white foreground
static constexpr uint8_t BG_Y = 16;
static constexpr uint8_t FG_Y = 235;

// --- Bitmap font (8×8, MSB = leftmost column) --------------------------------
// Characters available: ' ' '-' '0'-'9' ':'
static constexpr char CHARSET[] = " -0123456789:";

static const uint8_t FONT[13][8] = {
    // ' '
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // '-'
    {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    // '0'
    {0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00},
    // '1'
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    // '2'
    {0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00},
    // '3'
    {0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00},
    // '4'
    {0x06, 0x0E, 0x1E, 0x36, 0x7F, 0x06, 0x06, 0x00},
    // '5'
    {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00},
    // '6'
    {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00},
    // '7'
    {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    // '8'
    {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    // '9'
    {0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00},
    // ':'
    {0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00},
};

// --- Internal helpers --------------------------------------------------------

static inline const uint8_t *getGlyph(char c)
{
    const char *p = strchr(CHARSET, c);
    return FONT[p ? static_cast<int>(p - CHARSET) : 0];
}

// setPixel helpers: write the Y (luma) byte for one pixel

static inline void setPixelPlanar(uint8_t *frame, int width, int height, int x, int y, uint8_t val)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        frame[y * width + x] = val;
}

static inline void setPixelYUYV(uint8_t *frame, int width, int height, int x, int y, uint8_t val)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        frame[y * width * 2 + x * 2] = val; // Y0 at byte 0, Y1 at byte 2, …
}

static inline void setPixelUYVY(uint8_t *frame, int width, int height, int x, int y, uint8_t val)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
        frame[y * width * 2 + x * 2 + 1] = val; // Y0 at byte 1, Y1 at byte 3, …
}

typedef void (*SetPixelFn)(uint8_t *, int, int, int, int, uint8_t);

static void drawText(uint8_t *frame, int width, int height,
                     int x0, int y0, const char *text, SetPixelFn setPixel)
{
    int len = static_cast<int>(strlen(text));
    int bgW = len * CHAR_W + MARGIN * 2;
    int bgH = CHAR_H + MARGIN * 2;

    // Dark background rectangle
    for (int dy = 0; dy < bgH; dy++)
        for (int dx = 0; dx < bgW; dx++)
            setPixel(frame, width, height, x0 - MARGIN + dx, y0 - MARGIN + dy, BG_Y);

    // Glyphs
    for (int i = 0; i < len; i++)
    {
        const uint8_t *glyph = getGlyph(text[i]);
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                uint8_t pval = (glyph[row] & (0x80u >> col)) ? FG_Y : BG_Y;
                for (int sy = 0; sy < SCALE; sy++)
                    for (int sx = 0; sx < SCALE; sx++)
                        setPixel(frame, width, height,
                                 x0 + i * CHAR_W + col * SCALE + sx,
                                 y0 + row * SCALE + sy,
                                 pval);
            }
        }
    }
}

// --- Public API --------------------------------------------------------------

/**
 * Burn a "YYYY-MM-DD HH:MM:SS" timestamp into @frame.
 *
 * @param frame     Raw frame buffer (modified in-place).
 * @param frameSize Size of the buffer in bytes.
 * @param width     Frame width in pixels.
 * @param height    Frame height in pixels.
 * @param format    V4L2 pixel format (V4L2_PIX_FMT_*).
 * @param tv        Frame timestamp (localtime is used).
 */
static void apply(char *frame, int frameSize, int width, int height, int format, const timeval &tv)
{
    if (!frame || width <= 0 || height <= 0)
        return;

    // Format the timestamp string
    struct tm tm_info;
    time_t sec = tv.tv_sec;
    localtime_r(&sec, &tm_info);
    char text[32];
    strftime(text, sizeof(text), "%Y-%m-%d %H:%M:%S", &tm_info);

    auto *buf = reinterpret_cast<uint8_t *>(frame);

    switch (format)
    {
    case V4L2_PIX_FMT_NV12:
    case V4L2_PIX_FMT_NV21:
    case V4L2_PIX_FMT_YUV420:
    case V4L2_PIX_FMT_YVU420:
        // Y plane is width×height bytes starting at offset 0
        if (frameSize < width * height)
            return;
        drawText(buf, width, height, MARGIN, MARGIN, text, setPixelPlanar);
        break;

    case V4L2_PIX_FMT_YUYV:
        if (frameSize < width * height * 2)
            return;
        drawText(buf, width, height, MARGIN, MARGIN, text, setPixelYUYV);
        break;

    case V4L2_PIX_FMT_UYVY:
        if (frameSize < width * height * 2)
            return;
        drawText(buf, width, height, MARGIN, MARGIN, text, setPixelUYVY);
        break;

    default:
        // Compressed or unsupported format — no overlay
        break;
    }
}

} // namespace TimestampOverlay
