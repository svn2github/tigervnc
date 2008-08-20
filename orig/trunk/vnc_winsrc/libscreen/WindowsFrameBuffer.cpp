#include "WindowsFrameBuffer.h"

WindowsFrameBuffer::WindowsFrameBuffer(void)
{
}

WindowsFrameBuffer::~WindowsFrameBuffer(void)
{
}

HPALETTE WindowsFrameBuffer::GetSystemPalette()
{
  int paletteSize, logSize;
  PLOGPALETTE logPalette;
  HDC dc;
  HWND focus;
  HPALETTE result;

  focus = GetFocus();
  dc = GetDC(focus);

  paletteSize = GetDeviceCaps(dc, SIZEPALETTE);
  logSize = sizeof(LOGPALETTE) + (paletteSize - 1) * sizeof(PALETTEENTRY);
  logPalette = (PLOGPALETTE) malloc(logSize);
  if (logPalette == NULL) {
    ReleaseDC(focus, dc);
    return NULL;
  }

  logPalette->palVersion = 0x0300;
  logPalette->palNumEntries = paletteSize;
  GetSystemPaletteEntries(dc, 0, paletteSize, logPalette->palPalEntry);
  result = CreatePalette(logPalette);

  free(logPalette);
  ReleaseDC(focus, dc);

  return result;
}
  
void WindowsFrameBuffer::CaptureScreenRect(Rect *aRect, HDC dstDC)
{
  HDC screenDC;
  screenDC = GetDC(0);

  //BitBlt(dstDC, aRect->left, aRect->right, aRect->right, aRect->bottom, screenDC, 0, 0, SRCCOPY);

  ReleaseDC(0, screenDC);
}

void WindowsFrameBuffer::SetPropertiesChanged()
{
  // Check for changing

  return;
}

void WindowsFrameBuffer::Update()
{
  SetPropertiesChanged();

  return;
}
