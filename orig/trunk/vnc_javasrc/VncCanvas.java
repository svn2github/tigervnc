//
//  Copyright (C) 2001 HorizonLive.com, Inc.  All Rights Reserved.
//  Copyright (C) 2001 Const Kaplinsky.  All Rights Reserved.
//  Copyright (C) 2000 Tridia Corporation.  All Rights Reserved.
//  Copyright (C) 1999 AT&T Laboratories Cambridge.  All Rights Reserved.
//
//  This is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this software; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.lang.*;
import java.util.zip.*;


//
// VncCanvas is a subclass of Canvas which draws a VNC desktop on it.
//

class VncCanvas extends Canvas
  implements KeyListener, MouseListener, MouseMotionListener {

  VncViewer viewer;
  RfbProto rfb;
  ColorModel cm8, cm24;
  Color[] colors;
  int bytesPixel;

  Image rawPixelsImage;
  MemoryImageSource pixelsSource;
  byte[] pixels8;
  int[] pixels24;

  byte[] zlibBuf;
  int zlibBufLen = 0;
  Inflater zlibInflater;

  final static int tightZlibBufferSize = 512;
  Inflater[] tightInflaters;

  boolean listenersInstalled;

  VncCanvas(VncViewer v) throws IOException {
    viewer = v;
    rfb = viewer.rfb;

    cm8 = new DirectColorModel(8, 7, (7 << 3), (3 << 6));
    cm24 = new DirectColorModel(24, 0xFF0000, 0x00FF00, 0x0000FF);

    colors = new Color[256];
    for (int i = 0; i < 256; i++)
      colors[i] = new Color(cm8.getRGB(i));

    if (viewer.options.eightBitColors) {
      rfb.writeSetPixelFormat(8, 8, false, true, 7, 7, 3, 0, 3, 6);
      bytesPixel = 1;
    } else {
      rfb.writeSetPixelFormat(32, 24, true, true, 255, 255, 255, 16, 8, 0);
      bytesPixel = 4;
    }

    updateFramebufferSize();

    listenersInstalled = false;
    if (!viewer.options.viewOnly)
      enableInput(true);

    tightInflaters = new Inflater[4];
  }

  //
  // Start/stop receiving keyboard and mouse events.
  //

  public synchronized void enableInput(boolean enable) {
    if (enable && !listenersInstalled) {
      listenersInstalled = true;
      addKeyListener(this);
      addMouseListener(this);
      addMouseMotionListener(this);
    } else if (!enable && listenersInstalled) {
      listenersInstalled = false;
      removeKeyListener(this);
      removeMouseListener(this);
      removeMouseMotionListener(this);
    }
  }

  void updateFramebufferSize() {

    if (bytesPixel == 1) {
      pixels24 = null;
      pixels8 = new byte[rfb.framebufferWidth * rfb.framebufferHeight];

      pixelsSource =
	new MemoryImageSource(rfb.framebufferWidth, rfb.framebufferHeight,
			      cm8, pixels8, 0, rfb.framebufferWidth);
    } else {
      pixels8 = null;
      pixels24 = new int[rfb.framebufferWidth * rfb.framebufferHeight];

      pixelsSource =
	new MemoryImageSource(rfb.framebufferWidth, rfb.framebufferHeight,
			      cm24, pixels24, 0, rfb.framebufferWidth);
    }

    pixelsSource.setAnimated(true);
    rawPixelsImage = createImage(pixelsSource);

    if (viewer.inSeparateFrame) {
      if (viewer.desktopScrollPane != null)
	resizeDesktopFrame();
    } else {
      setSize(rfb.framebufferWidth, rfb.framebufferHeight);
    }
  }

  void resizeDesktopFrame() {
    setSize(rfb.framebufferWidth, rfb.framebufferHeight);

    // FIXME: Find a better way to determine correct size of a
    // ScrollPane.  -- const
    Insets insets = viewer.desktopScrollPane.getInsets();
    viewer.desktopScrollPane.setSize(rfb.framebufferWidth +
				     2 * Math.min(insets.left, insets.right),
				     rfb.framebufferHeight +
				     2 * Math.min(insets.top, insets.bottom));

    viewer.vncFrame.pack();
    viewer.desktopScrollPane.doLayout();
  }

  public Dimension getPreferredSize() {
    return new Dimension(rfb.framebufferWidth, rfb.framebufferHeight);
  }

  public Dimension getMinimumSize() {
    return new Dimension(rfb.framebufferWidth, rfb.framebufferHeight);
  }

  public Dimension getMaximumSize() {
    return new Dimension(rfb.framebufferWidth, rfb.framebufferHeight);
  }

  public void update(Graphics g) {
    paint(g);
  }

  public void paint(Graphics g) {
    g.drawImage(rawPixelsImage, 0, 0, this);
    if (showSoftCursor) {
      int x0 = cursorX - hotX, y0 = cursorY - hotY;
      Rectangle r = new Rectangle(x0, y0, cursorWidth, cursorHeight);
      if (r.intersects(g.getClipBounds())) {
	g.drawImage(softCursor, x0, y0, this);
      }
    }
  }

  //
  // processNormalProtocol() - executed by the rfbThread to deal with the
  // RFB socket.
  //

  public void processNormalProtocol() throws IOException {

    rfb.writeFramebufferUpdateRequest(0, 0, rfb.framebufferWidth,
				      rfb.framebufferHeight, false);

    //
    // main dispatch loop
    //

    while (true) {
      int msgType = rfb.readServerMessageType();

      switch (msgType) {
      case RfbProto.FramebufferUpdate:
	rfb.readFramebufferUpdate();

	for (int i = 0; i < rfb.updateNRects; i++) {
	  rfb.readFramebufferUpdateRectHdr();

	  if (rfb.updateRectEncoding == rfb.EncodingLastRect)
	    break;

	  if (rfb.updateRectEncoding == rfb.EncodingNewFBSize) {
	    rfb.setFramebufferSize(rfb.updateRectW, rfb.updateRectH);
	    updateFramebufferSize();
	    break;
	  }

	  if (rfb.updateRectEncoding == rfb.EncodingXCursor ||
	      rfb.updateRectEncoding == rfb.EncodingRichCursor) {
	    handleCursorShapeUpdate(rfb.updateRectEncoding,
				    rfb.updateRectX, rfb.updateRectY,
				    rfb.updateRectW, rfb.updateRectH);
	    continue;
	  }

	  switch (rfb.updateRectEncoding) {

	  case RfbProto.EncodingRaw:
	  {
	    drawRawRect(rfb.updateRectX, rfb.updateRectY,
			rfb.updateRectW, rfb.updateRectH);
	    break;
	  }

	  case RfbProto.EncodingCopyRect:
	  {
	    rfb.readCopyRect();
	    handleCopyRect();
	    break;
	  }

	  case RfbProto.EncodingRRE:
	  {
	    int rx = rfb.updateRectX, ry = rfb.updateRectY;
	    int rw = rfb.updateRectW, rh = rfb.updateRectH;
	    int nSubrects = rfb.is.readInt();
	    int pixel, x, y, w, h;

	    if (bytesPixel == 1) {
	      fillLargeArea(rx, ry, rw, rh, (byte)rfb.is.read());

	      for (int j = 0; j < nSubrects; j++) {
		pixel = rfb.is.read();
		x = rx + rfb.is.readUnsignedShort();
		y = ry + rfb.is.readUnsignedShort();
		w = rfb.is.readUnsignedShort();
		h = rfb.is.readUnsignedShort();

		fillSmallArea(x, y, w, h, (byte)pixel);
	      }
	    } else {		// 24-bit color
	      fillLargeArea(rx, ry, rw, rh, rfb.is.readInt());

	      for (int j = 0; j < nSubrects; j++) {
		pixel = rfb.is.readInt();
		x = rx + rfb.is.readUnsignedShort();
		y = ry + rfb.is.readUnsignedShort();
		w = rfb.is.readUnsignedShort();
		h = rfb.is.readUnsignedShort();

		fillSmallArea(x, y, w, h, pixel);
	      }
	    }

	    handleUpdatedPixels(rx, ry, rw, rh);
	    break;
	  }

	  case RfbProto.EncodingCoRRE:
	  {
	    int rx = rfb.updateRectX, ry = rfb.updateRectY;
	    int rw = rfb.updateRectW, rh = rfb.updateRectH;
	    int nSubrects = rfb.is.readInt();
	    int pixel, x, y, w, h;

	    if (bytesPixel == 1) {
	      fillLargeArea(rx, ry, rw, rh, (byte)rfb.is.read());

	      for (int j = 0; j < nSubrects; j++) {
		pixel = rfb.is.read();
		x = rx + rfb.is.read();
		y = ry + rfb.is.read();
		w = rfb.is.read();
		h = rfb.is.read();

		fillSmallArea(x, y, w, h, (byte)pixel);
	      }
	    } else {		// 24-bit color
	      fillLargeArea(rx, ry, rw, rh, rfb.is.readInt());

	      for (int j = 0; j < nSubrects; j++) {
		pixel = rfb.is.readInt();
		x = rx + rfb.is.read();
		y = ry + rfb.is.read();
		w = rfb.is.read();
		h = rfb.is.read();

		fillSmallArea(x, y, w, h, pixel);
	      }
	    }

	    handleUpdatedPixels(rx, ry, rw, rh);
	    break;
	  }

	  case RfbProto.EncodingHextile:
	  {
	    int rx = rfb.updateRectX, ry = rfb.updateRectY;
	    int rw = rfb.updateRectW, rh = rfb.updateRectH;
	    int bg = 0, fg = 0, sx, sy, sw, sh;

	    for (int ty = ry; ty < ry + rh; ty += 16) {

	      int th = 16;
	      if (ry + rh - ty < 16)
		th = ry + rh - ty;

	      for (int tx = rx; tx < rx + rw; tx += 16) {

		int tw = 16;
		if (rx + rw - tx < 16)
		  tw = rx + rw - tx;

		int subencoding = rfb.is.read();

		if ((subencoding & rfb.HextileRaw) != 0) {
		  if (bytesPixel == 1) {
		    for (int j = ty; j < ty + th; j++) {
		      rfb.is.readFully(pixels8, j*rfb.framebufferWidth+tx, tw);
		    }
		  } else {
		    byte[] buf = new byte[tw * 4];
		    int count, offset;
		    for (int j = ty; j < ty + th; j++) {
		      rfb.is.readFully(buf);
		      offset = j * rfb.framebufferWidth + tx;
		      for (count = 0; count < tw; count++) {
			pixels24[offset + count] =
			  (buf[count * 4 + 1] & 0xFF) << 16 |
			  (buf[count * 4 + 2] & 0xFF) << 8 |
			  (buf[count * 4 + 3] & 0xFF);
		      }
		    }
		  }
		  continue;
		}

		if (bytesPixel == 1) {
		  if ((subencoding & rfb.HextileBackgroundSpecified) != 0)
		    bg = rfb.is.read();

		  fillLargeArea(tx, ty, tw, th, (byte)bg);

		  if ((subencoding & rfb.HextileForegroundSpecified) != 0)
		    fg = rfb.is.read();
		} else {
		  if ((subencoding & rfb.HextileBackgroundSpecified) != 0)
		    bg = rfb.is.readInt();

		  fillLargeArea(tx, ty, tw, th, bg);

		  if ((subencoding & rfb.HextileForegroundSpecified) != 0)
		    fg = rfb.is.readInt();
		}

		// FIXME: Too many tests for bytesPixel?

		if ((subencoding & rfb.HextileAnySubrects) != 0) {

		  int nSubrects = rfb.is.read();

		  if ((subencoding & rfb.HextileSubrectsColoured) != 0) {

		    for (int j = 0; j < nSubrects; j++) {
		      fg = (bytesPixel == 1) ?
			rfb.is.read() : rfb.is.readInt();
		      int b1 = rfb.is.read();
		      int b2 = rfb.is.read();
		      sx = tx + (b1 >> 4);
		      sy = ty + (b1 & 0xf);
		      sw = (b2 >> 4) + 1;
		      sh = (b2 & 0xf) + 1;

		      if (bytesPixel == 1) {
			fillSmallArea(sx, sy, sw, sh, (byte)fg);
		      } else {
			fillSmallArea(sx, sy, sw, sh, fg);
		      }
		    }

		  } else {

		    for (int j = 0; j < nSubrects; j++) {
		      int b1 = rfb.is.read();
		      int b2 = rfb.is.read();
		      sx = tx + (b1 >> 4);
		      sy = ty + (b1 & 0xf);
		      sw = (b2 >> 4) + 1;
		      sh = (b2 & 0xf) + 1;

		      if (bytesPixel == 1) {
			fillSmallArea(sx, sy, sw, sh, (byte)fg);
		      } else {
			fillSmallArea(sx, sy, sw, sh, fg);
		      }
		    }

		  }
		}
	      }
	      handleUpdatedPixels(rx, ty, rw, th);
	    }
	    break;
	  }

	  case RfbProto.EncodingZlib:
	  {
	    int nBytes = rfb.is.readInt();

            if (zlibBuf == null || zlibBufLen < nBytes) {
              zlibBufLen = nBytes * 2;
              zlibBuf = new byte[zlibBufLen];
            }

            rfb.is.readFully(zlibBuf, 0, nBytes);

            if (zlibInflater == null) {
              zlibInflater = new Inflater();
            }
            zlibInflater.setInput(zlibBuf, 0, nBytes);

            drawZlibRect(rfb.updateRectX, rfb.updateRectY,
			 rfb.updateRectW, rfb.updateRectH);

	    break;
	  }

	  case RfbProto.EncodingTight:
	  {
	    drawTightRect(rfb.updateRectX, rfb.updateRectY,
			  rfb.updateRectW, rfb.updateRectH);

	    break;
	  }

	  default:
	    throw new IOException("Unknown RFB rectangle encoding " +
				  rfb.updateRectEncoding);
	  }

	}
	rfb.writeFramebufferUpdateRequest(0, 0, rfb.framebufferWidth,
					  rfb.framebufferHeight, true);
	break;

      case RfbProto.SetColourMapEntries:
	throw new IOException("Can't handle SetColourMapEntries message");

      case RfbProto.Bell:
        Toolkit.getDefaultToolkit().beep();
	break;

      case RfbProto.ServerCutText:
	String s = rfb.readServerCutText();
	viewer.clipboard.setCutText(s);
	break;

      default:
	throw new IOException("Unknown RFB message type " + msgType);
      }
    }
  }


  //
  // Draw a raw rectangle.
  //

  void drawRawRect(int x, int y, int w, int h) throws IOException {

    if (bytesPixel == 1) {
      for (int dy = y; dy < y + h; dy++) {
	rfb.is.readFully(pixels8, dy * rfb.framebufferWidth + x, w);
      }
    } else {
      byte[] buf = new byte[w * 4];
      int i, offset;
      for (int dy = y; dy < y + h; dy++) {
	rfb.is.readFully(buf);
	offset = dy * rfb.framebufferWidth + x;
	for (i = 0; i < w; i++) {
	  pixels24[offset + i] =
	    (buf[i * 4 + 1] & 0xFF) << 16 |
	    (buf[i * 4 + 2] & 0xFF) << 8 |
	    (buf[i * 4 + 3] & 0xFF);
	}
      }
    }

    handleUpdatedPixels(x, y, w, h);
  }


  //
  // Handle CopyRect rectangle (fast version).
  //

  void handleCopyRect() throws IOException {

    int sx = rfb.copyRectSrcX, sy = rfb.copyRectSrcY;
    int rx = rfb.updateRectX, ry = rfb.updateRectY;
    int rw = rfb.updateRectW, rh = rfb.updateRectH;

    int y0, y1, delta;
    if (sy > ry) {
      y0 = 0; y1 = rh; delta = 1;
    } else if (sy < ry) {
      y0 = rh - 1; y1 = -1; delta = -1;
    } else {
      handleCopyRectSlow();
      return;
    }

    Object array = pixels8;
    if (bytesPixel != 1)
      array = pixels24;

    for (int dy = y0; dy != y1; dy += delta) {
      System.arraycopy(array, (sy + dy) * rfb.framebufferWidth + sx,
                       array, (ry + dy) * rfb.framebufferWidth + rx, rw);
    }

    handleUpdatedPixels(rx, ry, rw, rh);
  }


  //
  // Handle CopyRect rectangle (slow version, but scanlines may overlap).
  //

  void handleCopyRectSlow() throws IOException {

    int sx = rfb.copyRectSrcX, sy = rfb.copyRectSrcY;
    int rx = rfb.updateRectX, ry = rfb.updateRectY;
    int rw = rfb.updateRectW, rh = rfb.updateRectH;

    int dx, dy;

    if (bytesPixel == 1) {
      if (sy * rfb.framebufferWidth + sx > ry * rfb.framebufferWidth + rx) {
	for (dy = 0; dy < rh; dy++) {
	  for (dx = 0; dx < rw; dx++) {
	    pixels8[(ry + dy) * rfb.framebufferWidth + (rx + dx)] =
	      pixels8[(sy + dy) * rfb.framebufferWidth + (sx + dx)];
	  }
	}
      } else {
	for (dy = rh - 1; dy >= 0; dy--) {
	  for (dx = rw - 1; dx >= 0; dx--) {
	    pixels8[(ry + dy) * rfb.framebufferWidth + (rx + dx)] =
	      pixels8[(sy + dy) * rfb.framebufferWidth + (sx + dx)];
	  }
	}
      }
    } else {
      if (sy * rfb.framebufferWidth + sx > ry * rfb.framebufferWidth + rx) {
	for (dy = 0; dy < rh; dy++) {
	  for (dx = 0; dx < rw; dx++) {
	    pixels24[(ry + dy) * rfb.framebufferWidth + (rx + dx)] =
	      pixels24[(sy + dy) * rfb.framebufferWidth + (sx + dx)];
	  }
	}
      } else {
	for (dy = rh - 1; dy >= 0; dy--) {
	  for (dx = rw - 1; dx >= 0; dx--) {
	    pixels24[(ry + dy) * rfb.framebufferWidth + (rx + dx)] =
	      pixels24[(sy + dy) * rfb.framebufferWidth + (sx + dx)];
	  }
	}
      }
    }

    handleUpdatedPixels(rx, ry, rw, rh);
  }


  //
  // Draw a zlib rectangle.
  //

  void drawZlibRect(int x, int y, int w, int h) throws IOException {

    try {
      if (bytesPixel == 1) {
	for (int dy = y; dy < y + h; dy++) {
	  zlibInflater.inflate(pixels8, dy * rfb.framebufferWidth + x, w);
	}
      } else {
	byte[] buf = new byte[w * 4];
	int i, offset;
	for (int dy = y; dy < y + h; dy++) {
	  zlibInflater.inflate(buf);
	  offset = dy * rfb.framebufferWidth + x;
	  for (i = 0; i < w; i++) {
	    pixels24[offset + i] =
	      (buf[i * 4 + 1] & 0xFF) << 16 |
	      (buf[i * 4 + 2] & 0xFF) << 8 |
	      (buf[i * 4 + 3] & 0xFF);
	  }
	}
      }
    }
    catch (DataFormatException dfe) {
      throw new IOException(dfe.toString());
    }

    handleUpdatedPixels(x, y, w, h);
  }


  //
  // Draw a tight rectangle.
  //

  void drawTightRect(int x, int y, int w, int h) throws IOException {

    int comp_ctl = rfb.is.readUnsignedByte();

    // Flush zlib streams if we are told by the server to do so.
    for (int stream_id = 0; stream_id < 4; stream_id++) {
      if ((comp_ctl & 1) != 0 && tightInflaters[stream_id] != null) {
	tightInflaters[stream_id] = null;
      }
      comp_ctl >>= 1;
    }

    // Check correctness of subencoding value.
    if (comp_ctl > rfb.TightMaxSubencoding) {
      throw new IOException("Incorrect tight subencoding: " + comp_ctl);
    }

    // Handle solid rectangles.
    if (comp_ctl == rfb.TightFill) {
      int bg = rfb.is.readUnsignedByte();

      fillLargeArea(x, y, w, h, (byte)bg);
      handleUpdatedPixels(x, y, w, h);
      return;
    }

    // Read filter id and parameters.
    int numColors = 0, rowSize = w;
    byte palette[] = new byte[2];
    if ((comp_ctl & rfb.TightExplicitFilter) != 0) {
      int filter_id = rfb.is.readUnsignedByte();
      if (filter_id == rfb.TightFilterPalette) {
	numColors = rfb.is.readUnsignedByte() + 1; // Must be 2.
	if (numColors != 2) {
	  throw new IOException("Incorrect tight palette size: " + numColors);
	}
	palette[0] = rfb.is.readByte();
	palette[1] = rfb.is.readByte();
	rowSize = (w + 7) / 8;
      } else if (filter_id != rfb.TightFilterCopy) {
	throw new IOException("Incorrect tight filter id: " + filter_id);
      }
    }

    // Read, optionally uncompress and decode data.
    int dataSize = h * rowSize;
    if (dataSize < rfb.TightMinToCompress) {
      if (numColors == 2) {
	byte[] monoData = new byte[dataSize];
	rfb.is.readFully(monoData, 0, dataSize);
	drawMonoData(x, y, w, h, monoData, palette);
      } else {
	for (int j = y; j < (y + h); j++) {
	  rfb.is.readFully(pixels8, j * rfb.framebufferWidth + x, w);
	}
      }
    } else {
      int zlibDataLen = rfb.readCompactLen();
      byte[] zlibData = new byte[zlibDataLen];
      rfb.is.readFully(zlibData, 0, zlibDataLen);
      int stream_id = comp_ctl & 0x03;
      if (tightInflaters[stream_id] == null) {
	tightInflaters[stream_id] = new Inflater();
      }
      Inflater myInflater = tightInflaters[stream_id];
      myInflater.setInput(zlibData, 0, zlibDataLen);
      try {
	if (numColors == 2) {
	  byte[] monoData = new byte[dataSize];
	  myInflater.inflate(monoData, 0, dataSize);
	  drawMonoData(x, y, w, h, monoData, palette);
	} else {
	  for (int j = y; j < (y + h); j++) {
	    myInflater.inflate(pixels8, j * rfb.framebufferWidth + x, w);
	  }
	}
      }
      catch(DataFormatException dfe) {
	throw new IOException(dfe.toString());
      }
    }

    handleUpdatedPixels(x, y, w, h);
  }


  //
  // Decode and draw 1bpp-encoded bi-color rectangle.
  //

  void drawMonoData(int x, int y, int w, int h,
		    byte[] src, byte[] palette)
    throws IOException {

    int dx, dy, n;
    int i = y * rfb.framebufferWidth + x;
    int rowBytes = (w + 7) / 8;
    byte b;

    for (dy = 0; dy < h; dy++) {
      for (dx = 0; dx < w / 8; dx++) {
	b = src[dy*rowBytes+dx];
	for (n = 7; n >= 0; n--)
	  pixels8[i++] = palette[b >> n & 1];
      }
      for (n = 7; n >= 8 - w % 8; n--) {
	pixels8[i++] = palette[src[dy*rowBytes+dx] >> n & 1];
      }
      i += (rfb.framebufferWidth - w);
    }
  }


  //
  // Display newly updated area of pixels.
  //

  synchronized void
  handleUpdatedPixels(int x, int y, int w, int h) {

    pixelsSource.newPixels(x, y, w, h);

    // Request repaint delayed by 20 milliseconds.
    repaint(20, x, y, w, h);
  }


  //
  // Emulate fillRect operation on the array of pixels.
  // Here are two versions -- for 8-bit and 24-bit color.
  // These versions are optimized for very small rectangles.
  //

  void fillSmallArea(int x, int y, int w, int h, byte pixel) {

    int offset = y * rfb.framebufferWidth + x;
    for (int dy = 0; dy < h; dy++) {
      for (int dx = 0; dx < w; dx++) {
	pixels8[offset++] = pixel;
      }
      offset += (rfb.framebufferWidth - w);
    }
  }

  void fillSmallArea(int x, int y, int w, int h, int pixel) {

    int offset = y * rfb.framebufferWidth + x;
    for (int dy = 0; dy < h; dy++) {
      for (int dx = 0; dx < w; dx++) {
	pixels24[offset++] = pixel;
      }
      offset += (rfb.framebufferWidth - w);
    }
  }


  //
  // Emulate fillRect operation on the array of pixels.
  // Here are two versions -- for 8-bit and 24-bit color.
  // These versions are optimized for rectangles that are large enough.
  //

  void fillLargeArea(int x, int y, int w, int h, byte pixel) {
    if (h * w == 0)
      return;

    int offset = y * rfb.framebufferWidth + x;
    for (int i = 0; i < w; i++) {
      pixels8[offset+i] = pixel;
    }

    for (int rows = h - 1; rows > 0; rows--) {
      System.arraycopy(pixels8, offset,
                       pixels8, offset + rfb.framebufferWidth, w);
      offset += rfb.framebufferWidth;
    }
  }

  void fillLargeArea(int x, int y, int w, int h, int pixel) {
    if (h * w == 0)
      return;

    int offset = y * rfb.framebufferWidth + x;
    for (int i = 0; i < w; i++) {
      pixels24[offset+i] = pixel;
    }

    for (int rows = h - 1; rows > 0; rows--) {
      System.arraycopy(pixels24, offset,
                       pixels24, offset + rfb.framebufferWidth, w);
      offset += rfb.framebufferWidth;
    }
  }


  //
  // Override the ImageObserver interface method.
  // FIXME: Maybe call repaint() from imageUpdate()?
  //

  public boolean imageUpdate(Image img, int infoflags,
                             int x, int y, int width, int height) {
    if ((infoflags & ALLBITS) == 0) {
      return true;
    } else {
      return false;
    }
  }
 

  //
  // Handle events.
  //

  public void keyPressed(KeyEvent evt) {
    processLocalKeyEvent(evt);
  }
  public void keyReleased(KeyEvent evt) {
    processLocalKeyEvent(evt);
  }
  public void keyTyped(KeyEvent evt) {
    evt.consume();
  }

  public void mousePressed(MouseEvent evt) {
    processLocalMouseEvent(evt, false);
  }
  public void mouseReleased(MouseEvent evt) {
    processLocalMouseEvent(evt, false);
  }
  public void mouseMoved(MouseEvent evt) {
    processLocalMouseEvent(evt, true);
  }
  public void mouseDragged(MouseEvent evt) {
    processLocalMouseEvent(evt, true);
  }

  public void processLocalKeyEvent(KeyEvent evt) {
    if (rfb != null && rfb.inNormalProtocol) {
      try {
        rfb.writeKeyEvent(evt);
      } catch (Exception e) {
	e.printStackTrace();
      }
    }
    // Don't ever pass keyboard events to AWT for default processing. 
    // Otherwise, pressing Tab would switch focus to ButtonPanel etc.
    evt.consume();
  }

  public void processLocalMouseEvent(MouseEvent evt, boolean moved) {
    if (rfb != null && rfb.inNormalProtocol) {
      if (moved) {
        softCursorMove(evt.getX(), evt.getY());
      }
      try {
        rfb.writePointerEvent(evt);
      } catch (Exception e) {
        e.printStackTrace();
      }
    }
  }


  //
  // Ignored events.
  //

  public void mouseClicked(MouseEvent evt) {}
  public void mouseEntered(MouseEvent evt) {}
  public void mouseExited(MouseEvent evt) {}


  //////////////////////////////////////////////////////////////////
  //
  // Handle cursor shape updates (XCursor and RichCursor encodings).
  //

  boolean showSoftCursor = false;

  int[] softCursorPixels;
  MemoryImageSource softCursorSource;
  Image softCursor;

  int cursorX = 0, cursorY = 0;
  int cursorWidth, cursorHeight;
  int hotX, hotY;

  //
  // Handle cursor shape update (XCursor and RichCursor encodings).
  //

  synchronized void
    handleCursorShapeUpdate(int encodingType,
			    int xhot, int yhot, int width, int height)
    throws IOException {

    int bytesPerRow = (width + 7) / 8;
    int bytesMaskData = bytesPerRow * height;

    softCursorFree();

    if (width * height == 0)
      return;

    // Ignore cursor shape data if requested by user.

    if (viewer.options.ignoreCursorUpdates) {
      if (encodingType == rfb.EncodingXCursor) {
	rfb.is.skipBytes(6 + bytesMaskData * 2);
      } else {
	// rfb.EncodingRichCursor
	rfb.is.skipBytes(width * height + bytesMaskData);
      }
      return;
    }

    // Decode cursor pixel data.

    softCursorPixels = new int[width * height];

    if (encodingType == rfb.EncodingXCursor) {

      // Read foreground and background colors of the cursor.
      byte[] rgb = new byte[6];
      rfb.is.readFully(rgb, 0, 6);
      int[] colors = { (0xFF000000 | rgb[3] << 16 | rgb[4] << 8 | rgb[5]),
		       (0xFF000000 | rgb[0] << 16 | rgb[1] << 8 | rgb[2]) };

      // Read pixel and mask data.
      byte[] pixBuf = new byte[bytesMaskData];
      rfb.is.readFully(pixBuf, 0, bytesMaskData);
      byte[] maskBuf = new byte[bytesMaskData];
      rfb.is.readFully(maskBuf, 0, bytesMaskData);

      // Decode pixel data into softCursorPixels[].
      byte pixByte, maskByte;
      int x, y, n, result;
      int i = 0;
      for (y = 0; y < height; y++) {
	for (x = 0; x < width / 8; x++) {
	  pixByte = pixBuf[y * bytesPerRow + x];
	  maskByte = maskBuf[y * bytesPerRow + x];
	  for (n = 7; n >= 0; n--) {
	    if ((maskByte >> n & 1) != 0) {
	      result = colors[pixByte >> n & 1];
	    } else {
	      result = 0;	// Transparent pixel
	    }
	    softCursorPixels[i++] = result;
	  }
	}
	for (n = 7; n >= 8 - width % 8; n--) {
	  if ((maskBuf[y * bytesPerRow + x] >> n & 1) != 0) {
	    result = colors[pixBuf[y * bytesPerRow + x] >> n & 1];
	  } else {
	    result = 0;		// Transparent pixel
	  }
	  softCursorPixels[i++] = result;
	}
      }

    } else {
      // encodingType == rfb.EncodingRichCursor

      // Read pixel and mask data.
      byte[] pixBuf = new byte[width * height * bytesPixel];
      rfb.is.readFully(pixBuf, 0, width * height * bytesPixel);
      byte[] maskBuf = new byte[bytesMaskData];
      rfb.is.readFully(maskBuf, 0, bytesMaskData);

      // Decode pixel data into softCursorPixels[].
      byte pixByte, maskByte;
      int x, y, n, result;
      int i = 0;
      for (y = 0; y < height; y++) {
	for (x = 0; x < width / 8; x++) {
	  maskByte = maskBuf[y * bytesPerRow + x];
	  for (n = 7; n >= 0; n--) {
	    if ((maskByte >> n & 1) != 0) {
	      if (bytesPixel == 1) {
		result = cm8.getRGB(pixBuf[i]);
	      } else {
		result = 0xFF000000 |
		  (pixBuf[i * 4 + 1] & 0xFF) << 16 |
		  (pixBuf[i * 4 + 2] & 0xFF) << 8 |
		  (pixBuf[i * 4 + 3] & 0xFF);
	      }
	    } else {
	      result = 0;	// Transparent pixel
	    }
	    softCursorPixels[i++] = result;
	  }
	}
	for (n = 7; n >= 8 - width % 8; n--) {
	  if ((maskBuf[y * bytesPerRow + x] >> n & 1) != 0) {
	    if (bytesPixel == 1) {
	      result = cm8.getRGB(pixBuf[i]);
	    } else {
	      result = 0xFF000000 |
		(pixBuf[i * 4 + 1] & 0xFF) << 16 |
		(pixBuf[i * 4 + 2] & 0xFF) << 8 |
		(pixBuf[i * 4 + 3] & 0xFF);
	    }
	  } else {
	    result = 0;		// Transparent pixel
	  }
	  softCursorPixels[i++] = result;
	}
      }

    }

    // Draw the cursor on an off-screen image.

    softCursorSource =
      new MemoryImageSource(width, height, softCursorPixels, 0, width);
    softCursor = createImage(softCursorSource);

    // Set remaining data associated with cursor.

    cursorWidth = width;
    cursorHeight = height;
    hotX = xhot;
    hotY = yhot;

    showSoftCursor = true;

    // Show the cursor.

    repaint(10, cursorX - hotX, cursorY - hotY, cursorWidth, cursorHeight);
  }

  //
  // softCursorMove(). Moves soft cursor into a particular location.
  //

  synchronized void softCursorMove(int x, int y) {
    if (showSoftCursor) {
      repaint(10, cursorX - hotX, cursorY - hotY, cursorWidth, cursorHeight);
      repaint(10, x - hotX, y - hotY, cursorWidth, cursorHeight);
    }

    cursorX = x;
    cursorY = y;
  }

  //
  // softCursorFree(). Remove soft cursor, dispose resources.
  //

  synchronized void softCursorFree() {
    if (showSoftCursor) {
      showSoftCursor = false;
      softCursor = null;
      softCursorSource = null;
      softCursorPixels = null;

      repaint(10, cursorX - hotX, cursorY - hotY, cursorWidth, cursorHeight);
    }
  }
}
