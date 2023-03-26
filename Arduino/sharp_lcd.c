/*
 * st7302.c
 *
 *  Created on: Jan 27, 2023
 *      Author: larry
 */
#include <stdint.h>
#include <string.h>
#include "Arduino.h"
#include "sharp_lcd.h"

static uint8_t u8CSPin, u8DISPPin;
static uint8_t u8Cache[(LCD_WIDTH * LCD_HEIGHT)/(2*8)]; // 64 lines = 1k bytes
static uint8_t cursor_x, cursor_y;
const uint8_t ucMirror[256] =
     {0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240,
      8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248,
      4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244,
      12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252,
      2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
      10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250,
      6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246,
      14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254,
      1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241,
      9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
      5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245,
      13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253,
      3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243,
      11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251,
      7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
      15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255};

void sharp_init(int iSpeed, uint8_t u8CS, uint8_t u8DISP)
{
   u8CSPin = u8CS; u8DISPPin = u8DISP;
   SPI_begin(iSpeed, 0);
   pinMode(u8CSPin, OUTPUT);
   pinMode(u8DISPPin, OUTPUT);
   digitalWrite(u8DISPPin, 1); // turn on the LCD
} /* sharp_init() */

void sharp_writeLine(uint8_t *pData, int iLine)
{
static uint8_t u8VCOM = 0; // internal VCOM signal toggle
uint8_t u8 = 0x80; // start command

   if (iLine < 0 || iLine >= LCD_HEIGHT)
        return;
   digitalWrite(u8CSPin, 1); // activate CS
   u8 = (u8 | u8VCOM) & 0xc0;
   u8VCOM += 8; // toggle VCOM every 8 lines (0x40)
   SPI_write(&u8, 1); // start+vcom
   u8 = ucMirror[iLine+1]; // flip bit direction
   SPI_write(&u8, 1); // destination line number
   for (int i=0; i<LCD_WIDTH/8; i++) {
	   u8 = ~pData[i];
       SPI_write(&u8, 1); // inverted
   }
   u8 = 0;
   SPI_write(&u8, 1); // stop byte
   digitalWrite(u8CSPin, 0); // de-activate CS
} /* sharp_writeLine() */

void sharp_writeBuffer(int iStartLine)
{
int i;
   for (i=0; i<LCD_HEIGHT/2; i++) {
      sharp_writeLine(&u8Cache[i*(LCD_WIDTH/8)], i+iStartLine);
   }
} /* sharp_writeBuffer() */

void sharp_fill(uint8_t u8Pattern)
{
  memset(u8Cache, u8Pattern, sizeof(u8Cache));
} /* sharp_fill() */

//
// Draw a string of characters in a custom font
// A back buffer must be defined
//
void sharp_print(const GFXfont *pFont, int x, int y, char *szMsg, uint8_t u8Color)
{
	int i, end_y, dx, dy, tx, ty, iBitOff;
	unsigned int c;
	uint8_t *s, *d, bits, ucMask, uc;
	GFXfont font;
	GFXglyph glyph, *pGlyph;

	    if (x == -1)
	        x = cursor_x;
	    if (y == -1)
	        y = cursor_y;
	   // in case of running on Harvard architecture, get copy of data from FLASH
	   memcpy(&font, pFont, sizeof(font));
	   pGlyph = &glyph;

	   i = 0;
	   while (szMsg[i] && x < LCD_WIDTH)
	   {
	      c = szMsg[i++];
	      if (c < font.first || c > font.last) // undefined character
	         continue; // skip it
	      c -= font.first; // first char of font defined
	      memcpy(&glyph, &font.glyph[c], sizeof(glyph));
	      dx = x + pGlyph->xOffset; // offset from character UL to start drawing
	      dy = y + pGlyph->yOffset;
	      s = font.bitmap + pGlyph->bitmapOffset; // start of bitmap data
	      // Bitmap drawing loop. Image is MSB first and each pixel is packed next
	      // to the next (continuing on to the next character line)
	      iBitOff = 0; // bitmap offset (in bits)
	      bits = uc = 0; // bits left in this font byte
	      end_y = dy + pGlyph->height;
	      if (dy < 0) { // skip these lines
	          iBitOff += (pGlyph->width * (-dy));
	          dy = 0;
	      }
	      for (ty=dy; ty<end_y && ty < LCD_HEIGHT; ty++) {
	          d = &u8Cache[(dx/8) + (ty * (LCD_WIDTH/8))];
		  ucMask = 0x80>>(dx & 7); // destination bit number for this starting point
	          for (tx=dx; tx<(dx+pGlyph->width); tx++) {
	            if (bits == 0) { // need to read more font data
	               uc = s[iBitOff>>3]; // get more font bitmap data
	               bits = 8 - (iBitOff & 7); // we might not be on a byte boundary
	               iBitOff += bits; // because of a clipped line
	               uc <<= (8-bits);
	            } // if we ran out of bits
	            if (tx < LCD_WIDTH) { // foreground pixel
	                if (uc & 0x80) {
	                   if (u8Color == 1)
	                      d[0] |= ucMask;
	                   else
	                      d[0] &= ~ucMask;
	                } else {
	                    if (u8Color == 1)
	                       d[0] &= ~ucMask;
	                    else
	                       d[0] |= ucMask;
	                }
	            }
	            bits--; // next bit
	            uc <<= 1;
		    ucMask >>= 1;
		    if (ucMask == 0) { // next destination byte
                       ucMask = 0x80;
		       d++;
		    }
	         } // for x
	      } // for y
	      x += pGlyph->xAdvance; // width of this character
	   } // while drawing characters
	   cursor_x = x;
	   cursor_y = y;
} /* sharp_print() */



