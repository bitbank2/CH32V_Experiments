/*
 * st7302.h
 *
 *  Created on: Jan 27, 2023
 *      Author: larry
 */

#ifndef USER_SHARP_H_
#define USER_SHARP_H_

#define LCD_WIDTH 128
#define LCD_HEIGHT 128

// Proportional font data taken from Adafruit_GFX library
/// Font data stored PER GLYPH
#if !defined( _ADAFRUIT_GFX_H ) && !defined( _GFXFONT_H_ )
#define _GFXFONT_H_
typedef struct {
  uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
  uint8_t width;         ///< Bitmap dimensions in pixels
  uint8_t height;        ///< Bitmap dimensions in pixels
  uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
  int8_t xOffset;        ///< X dist from cursor pos to UL corner
  int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
  GFXglyph *glyph;  ///< Glyph array
  uint16_t first;    ///< ASCII extents (first char)
  uint16_t last;     ///< ASCII extents (last char)
  uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;
#endif // _ADAFRUIT_GFX_H

void sharp_init(int iSpeed, uint8_t u8CS, uint8_t u8DISP);
void sharp_fill(uint8_t u8Pattern);
void sharp_print(const GFXfont *pFont, int x, int y, char *szText, uint8_t u8Color);
void sharp_writeLine(uint8_t *pData, int iLen);
void sharp_writeBuffer(int iStartLine);

#endif /* USER_SHARP_H_ */
