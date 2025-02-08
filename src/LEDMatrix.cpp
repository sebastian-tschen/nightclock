/*
  LEDMatrix V5 class by Aaron Liddiment (c) 2016
  modified:  Juergen Skrotzky (JorgenVikingGod@gmail.com)
  date:      2016/04/27

  Inspiration for some of the Matrix functions from Stefan Petrick

  FastLED v3.1 library by Daniel Garcia and Mark Kriegsmann.
  Written & tested on a Teensy 3.1 using Arduino V1.6.3 & teensyduino V1.22
*/

#include <FastLED.h>
#include <LEDMatrix.h>
 #include <pgmspace.h>
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))

cLEDMatrixBase::cLEDMatrixBase()
{
}

struct CRGB* cLEDMatrixBase::operator[](int n)
{
  return(&m_LED[n]);
}

struct CRGB& cLEDMatrixBase::operator()(int16_t x, int16_t y)
{
  if ( (x >= 0) && (x < m_Width) && (y >= 0) && (y < m_Height))
    return(m_LED[mXY(x, y)]);
  else
    return(m_OutOfBounds);
}

struct CRGB& cLEDMatrixBase::operator()(int16_t i)
{
  if ((i >=0) && (i < (m_Width * m_Height)))
    return(m_LED[i]);
  else
    return(m_OutOfBounds);
}

void cLEDMatrixBase::HorizontalMirror(bool FullHeight)
{
  int ty, y, x, xx;

  if (FullHeight)
    ty = m_Height - 1;
  else
    ty = (m_Height / 2);
  for (y=ty; y>=0; --y)
  {
    for (x=(m_Width/2)-1,xx=((m_Width+1)/2); x>=0; --x,++xx)
      m_LED[mXY(xx, y)] = m_LED[mXY(x, y)];
  }
}


void cLEDMatrixBase::VerticalMirror()
{
  int y, yy, x;

  for (y=(m_Height/2)-1,yy=((m_Height+1)/2); y>=0; --y,++yy)
  {
    for (x=m_Width-1; x>=0; --x)
      m_LED[mXY(x, yy)] = m_LED[mXY(x, y)];
  }
}


void cLEDMatrixBase::QuadrantMirror()
{
  HorizontalMirror(false);
  VerticalMirror();
}


void cLEDMatrixBase::QuadrantRotateMirror()
{
  int MaxXY, MidXY, x, y, src;

  if (m_Width > m_Height)
    MaxXY = m_Height;
  else
    MaxXY = m_Width;
  MidXY = (MaxXY / 2);
  MaxXY--;
  for (x=MidXY-(MaxXY%2); x>=0; --x)
  {
    for (y=MidXY-(MaxXY%2); y>=0; --y)
    {
      src = mXY(x, y);
      m_LED[mXY(MidXY + y, MidXY - (MaxXY % 2) - x)] = m_LED[src];
      m_LED[mXY(MaxXY - x, MaxXY - y)] = m_LED[src];
      m_LED[mXY(MidXY - (MaxXY % 2) - y, MidXY + x)] = m_LED[src];
    }
  }
}


void cLEDMatrixBase::TriangleTopMirror(bool FullHeight)
{
  int MaxXY, x, y;

  if (m_Width > m_Height)
    MaxXY = m_Height - 1;
  else
    MaxXY = m_Width - 1;
  if (! FullHeight)
    MaxXY /= 2;
  for (y=1; y<=MaxXY; ++y)
  {
    for (x=0; x<y; ++x)
      m_LED[mXY(y,x)] = m_LED[mXY(x,y)];
  }
}


void cLEDMatrixBase::TriangleBottomMirror(bool FullHeight)
{
  int MaxXY, x, y, xx, yy;

  if (m_Width > m_Height)
    MaxXY = m_Height - 1;
  else
    MaxXY = m_Width - 1;
  if (! FullHeight)
    MaxXY /= 2;
  for (y=0,xx=MaxXY; y<MaxXY; y++,xx--)
  {
    for (x=MaxXY-y-1,yy=y+1; x>=0; --x,++yy)
      m_LED[mXY(xx, yy)] = m_LED[mXY(x, y)];
  }
}


void cLEDMatrixBase::QuadrantTopTriangleMirror()
{
  TriangleTopMirror(false);
  QuadrantMirror();
}


void cLEDMatrixBase::QuadrantBottomTriangleMirror()
{
  TriangleBottomMirror(false);
  QuadrantMirror();
}

void cLEDMatrixBase::DrawPixel(int16_t x, int16_t y, CRGB Col) {
  DrawLine(x, y, x, y, Col);
}

void cLEDMatrixBase::setFont(const GFXfont *f) {
  if(f) {          // Font struct pointer passed in?
    if(!gfxFont) { // And no current font struct?
      // Switching from classic to new font behavior.
      // Move cursor pos down 6 pixels so it's on baseline.
      cursor_y += 6;
    }
  } else if(gfxFont) { // NULL passed.  Current font struct defined?
    // Switching from new to classic font behavior.
    // Move cursor pos up 6 pixels so it's at top-left of char.
    cursor_y -= 6;
  }
  gfxFont = (GFXfont *)f;
}


// Draw a character
void cLEDMatrixBase::DrawChar(int16_t x, int16_t y, unsigned char c,
 CRGB color, CRGB bg, uint8_t size) {


  // Character is assumed previously filtered by write() to eliminate
  // newlines, returns, non-printable characters, etc.  Calling drawChar()
  // directly with 'bad' characters of font may cause mayhem!

  c -= pgm_read_byte(&gfxFont->first);
  GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
  uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

  uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
  uint8_t  w  = pgm_read_byte(&glyph->width),
            h  = pgm_read_byte(&glyph->height),
            xa = pgm_read_byte(&glyph->xAdvance);
  int8_t   xo = pgm_read_byte(&glyph->xOffset),
            yo = pgm_read_byte(&glyph->yOffset);
  uint8_t  xx, yy, bits, bit = 0;
  int16_t  xo16, yo16;

  if(size > 1) {
    xo16 = xo;
    yo16 = yo;
  }

  // Todo: Add character clipping here

  // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
  // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
  // has typically been used with the 'classic' font to overwrite old
  // screen contents with new data.  This ONLY works because the
  // characters are a uniform size; it's not a sensible thing to do with
  // proportionally-spaced fonts with glyphs of varying sizes (and that
  // may overlap).  To replace previously-drawn text when using a custom
  // font, use the getTextBounds() function to determine the smallest
  // rectangle encompassing a string, erase the area with fillRect(),
  // then draw new text.  This WILL infortunately 'blink' the text, but
  // is unavoidable.  Drawing 'background' pixels will NOT fix this,
  // only creates a new set of problems.  Have an idea to work around
  // this (a canvas object type for MCUs that can afford the RAM and
  // displays supporting setAddrWindow() and pushColors()), but haven't
  // implemented this yet.

  for(yy=0; yy<h; yy++) {
    for(xx=0; xx<w; xx++) {
      if(!(bit++ & 7)) {
        bits = pgm_read_byte(&bitmap[bo++]);
      }
      if(bits & 0x80) {
        if(size == 1) {
          DrawPixel(x+xo+xx, y+yo+yy, color);
        } else {
          DrawFilledRectangle(x+(xo16+xx)*size, y+(yo16+yy)*size, size, size, color);
        }
      }
      bits <<= 1;
    }
  }
}

void cLEDMatrixBase::DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col)
{
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;
  if (abs(dx) >= abs(dy))
  {
    int32_t y = ((int32_t)y0 << 16) + 32768;
    // Support a single dot line without diving by 0 and crashing below
    if (!dx) {
      (*this)(x0, (y >> 16)) = Col;
    } else {
      int32_t f = ((int32_t)dy << 16) / (int32_t)abs(dx);
      if (dx >= 0)
      {
        for (; x0<=x1; ++x0,y+=f)
          (*this)(x0, (y >> 16)) = Col;
      }
      else
      {
        for (; x0>=x1; --x0,y+=f)
          (*this)(x0, (y >> 16)) = Col;
      }
    }
  }
  else
  {
    int32_t f = ((int32_t)dx << 16) / (int32_t)abs(dy);
    int32_t x = ((int32_t)x0 << 16) + 32768;
    if (dy >= 0)
    {
      for (; y0<=y1; ++y0,x+=f)
        (*this)((x >> 16), y0) = Col;
    }
    else
    {
      for (; y0>=y1; --y0,x+=f)
        (*this)((x >> 16), y0) = Col;
    }
  }
}


void cLEDMatrixBase::DrawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col)
{
  DrawLine(x0, y0, x0, y1, Col);
  DrawLine(x0, y1, x1, y1, Col);
  DrawLine(x1, y1, x1, y0, Col);
  DrawLine(x1, y0, x0, y0, Col);
}


void cLEDMatrixBase::DrawCircle(int16_t xc, int16_t yc, uint16_t r, CRGB Col)
{
  int16_t x = -r;
  int16_t y = 0;
  int16_t e = 2 - (2 * r);
  do
  {
    (*this)(xc + x, yc - y) = Col;
    (*this)(xc - x, yc + y) = Col;
    (*this)(xc + y, yc + x) = Col;
    (*this)(xc - y, yc - x) = Col;
    int16_t _e = e;
    if (_e <= y)
      e += (++y * 2) + 1;
    if ((_e > x) || (e > y))
      e += (++x * 2) + 1;
  }
  while (x < 0);
}


void cLEDMatrixBase::DrawFilledRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, CRGB Col)
{
  int16_t y = min(y0, y1);
  for (int16_t c=abs(y1-y0); c>=0; --c,++y)
    DrawLine(x0, y, x1, y, Col);
}


void cLEDMatrixBase::DrawFilledCircle(int16_t xc, int16_t yc, uint16_t r, CRGB Col)
{
  int16_t x = r;
  int16_t y = 0;
  int16_t e = 1 - x;
  while (x >= y)
  {
    DrawLine(xc + x, yc + y, xc - x, yc + y, Col);
    DrawLine(xc + y, yc + x, xc - y, yc + x, Col);
    DrawLine(xc - x, yc - y, xc + x, yc - y, Col);
    DrawLine(xc - y, yc - x, xc + y, yc - x, Col);
    ++y;
    if (e >= 0)
    {
      --x;
      e += 2 * ((y - x) + 1);
    }
    else
      e += (2 * y) + 1;
  }
}
