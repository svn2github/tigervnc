/* Copyright (C) 2002-2005 RealVNC Ltd.  All Rights Reserved.
 * Copyright 2011 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <FL/fl_draw.H>
#include <FL/fl_ask.H>

#include <rfb/CMsgWriter.h>
#include <rfb/LogWriter.h>

// FLTK can pull in the X11 headers on some systems
#ifndef XK_VoidSymbol
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <rfb/keysymdef.h>
#endif

#ifndef XF86XK_ModeLock
#include <rfb/XF86keysym.h>
#endif

#include "Viewport.h"
#include "CConn.h"
#include "OptionsDialog.h"
#include "i18n.h"
#include "fltk_layout.h"
#include "parameters.h"
#include "keysym2ucs.h"

using namespace rfb;

extern void exit_vncviewer();
extern void about_vncviewer();

static rfb::LogWriter vlog("Viewport");

// Menu constants

enum { ID_EXIT, ID_FULLSCREEN, ID_CTRL, ID_ALT, ID_MENUKEY, ID_CTRLALTDEL,
       ID_REFRESH, ID_OPTIONS, ID_INFO, ID_ABOUT, ID_DISMISS };

Viewport::Viewport(int w, int h, const rfb::PixelFormat& serverPF, CConn* cc_)
  : Fl_Widget(0, 0, w, h), cc(cc_), frameBuffer(NULL), pixelTrans(NULL),
    lastPointerPos(0, 0), lastButtonMask(0)
{
// FLTK STR #2599 must be fixed for proper dead keys support
#ifdef HAVE_FLTK_DEAD_KEYS
  set_simple_keyboard();
#endif

// FLTK STR #2636 gives us the ability to monitor clipboard changes
#ifdef HAVE_FLTK_CLIPBOARD
  Fl::add_clipboard_notify(handleClipboardChange, this);
#endif

  frameBuffer = new ManagedPixelBuffer(getPreferredPF(), w, h);
  assert(frameBuffer);

  setServerPF(serverPF);

  contextMenu = new Fl_Menu_Button(0, 0, 0, 0);
  initContextMenu();

  setMenuKey();

  OptionsDialog::addCallback(handleOptions, this);
}


Viewport::~Viewport()
{
  // Unregister all timeouts in case they get a change tro trigger
  // again later when this object is already gone.
  Fl::remove_timeout(handleUpdateTimeout, this);
  Fl::remove_timeout(handleColourMap, this);
  Fl::remove_timeout(handlePointerTimeout, this);

#ifdef HAVE_FLTK_CLIPBOARD
  Fl::remove_clipboard_notify(handleClipboardChange);
#endif

  OptionsDialog::removeCallback(handleOptions);

  delete frameBuffer;

  if (pixelTrans)
    delete pixelTrans;

  // FLTK automatically deletes all child widgets, so we shouldn't touch
  // them ourselves here
}


void Viewport::setServerPF(const rfb::PixelFormat& pf)
{
  if (pixelTrans)
    delete pixelTrans;
  pixelTrans = NULL;

  if (pf.equal(getPreferredPF()))
    return;

  pixelTrans = new PixelTransformer();
  pixelTrans->init(pf, &colourMap, getPreferredPF());
}


const rfb::PixelFormat &Viewport::getPreferredPF()
{
  static PixelFormat prefPF(32, 24, false, true, 255, 255, 255, 0, 8, 16);

  return prefPF;
}


// setColourMapEntries() changes some of the entries in the colourmap.
// Unfortunately these messages are often sent one at a time, so we delay the
// settings taking effect by 100ms.  This is because recalculating the internal
// translation table can be expensive.
void Viewport::setColourMapEntries(int firstColour, int nColours,
                                   rdr::U16* rgbs)
{
  for (int i = 0; i < nColours; i++)
    colourMap.set(firstColour+i, rgbs[i*3], rgbs[i*3+1], rgbs[i*3+2]);

  if (!Fl::has_timeout(handleColourMap, this))
    Fl::add_timeout(0.100, handleColourMap, this);
}


// Copy the areas of the framebuffer that have been changed (damaged)
// to the displayed window.

void Viewport::updateWindow()
{
  Rect r;

  Fl::remove_timeout(handleUpdateTimeout, this);

  r = damage.get_bounding_rect();
  Fl_Widget::damage(FL_DAMAGE_USER1, r.tl.x + x(), r.tl.y + y(), r.width(), r.height());

  damage.clear();
}


void Viewport::draw()
{
  int X, Y, W, H;

  int pixel_bytes, stride_bytes;
  const uchar *buf_start;

  // Check what actually needs updating
  fl_clip_box(x(), y(), w(), h(), X, Y, W, H);
  if ((W == 0) || (H == 0))
    return;

  pixel_bytes = frameBuffer->getPF().bpp/8;
  stride_bytes = pixel_bytes * frameBuffer->getStride();
  buf_start = frameBuffer->data +
              pixel_bytes * (X - x()) +
              stride_bytes * (Y - y());

  // FIXME: Check how efficient this thing really is
  fl_draw_image(buf_start, X, Y, W, H, pixel_bytes, stride_bytes);
}


int Viewport::handle(int event)
{
  char buffer[1024];
  int ret;
  int buttonMask, wheelMask;
  DownMap::const_iterator iter;

  switch (event) {
  case FL_PASTE:
    // This is documented as to ASCII, but actually does to 8859-1
    ret = fl_utf8toa(Fl::event_text(), Fl::event_length(), buffer, sizeof(buffer));
    if (ret >= sizeof(buffer)) {
      vlog.error(_("Clipboard buffer overflow!"));
      return 1;
    }
    vlog.debug("Sending clipboard data: '%s'", buffer);
    cc->writer()->clientCutText(buffer, ret);
    return 1;
  case FL_ENTER:
    // Yes, we would like some pointer events please!
    return 1;
  case FL_PUSH:
  case FL_RELEASE:
  case FL_DRAG:
  case FL_MOVE:
  case FL_MOUSEWHEEL:
    buttonMask = 0;
    if (Fl::event_button1())
      buttonMask |= 1;
    if (Fl::event_button2())
      buttonMask |= 2;
    if (Fl::event_button3())
      buttonMask |= 4;

    if (event == FL_MOUSEWHEEL) {
      wheelMask = 0;
      if (Fl::event_dy() < 0)
        wheelMask |= 8;
      if (Fl::event_dy() > 0)
        wheelMask |= 16;
      if (Fl::event_dx() < 0)
        wheelMask |= 32;
      if (Fl::event_dx() > 0)
        wheelMask |= 64;

      // A quick press of the wheel "button", followed by a immediate
      // release below
      handlePointerEvent(Point(Fl::event_x() - x(), Fl::event_y() - y()),
                         buttonMask | wheelMask);
    } 

    handlePointerEvent(Point(Fl::event_x() - x(), Fl::event_y() - y()), buttonMask);
    return 1;

  case FL_FOCUS:
    // Yes, we would like some focus please!
    return 1;

  case FL_UNFOCUS:
    // Release all keys that were pressed as that generally makes most
    // sense (e.g. Alt+Tab where we only see the Alt press)
    for (iter = downKeySym.begin();iter != downKeySym.end();++iter) {
      vlog.debug("Key released: 0x%04x => 0x%04x", iter->first, iter->second);
      cc->writer()->keyEvent(iter->second, false);
    }
    downKeySym.clear();
    return 1;

  case FL_KEYDOWN:
    if (menuKeyCode && (Fl::event_key() == menuKeyCode)) {
      popupContextMenu();
      return 1;
    }

    handleKeyEvent(Fl::event_key(), Fl::event_original_key(),
                   Fl::event_text(), true);
    return 1;

  case FL_KEYUP:
    if (menuKeyCode && (Fl::event_key() == menuKeyCode))
      return 1;

    handleKeyEvent(Fl::event_key(), Fl::event_original_key(),
                   Fl::event_text(), false);
    return 1;
  }

  return Fl_Widget::handle(event);
}


void Viewport::handleUpdateTimeout(void *data)
{
  Viewport *self = (Viewport *)data;

  assert(self);

  self->updateWindow();
}


void Viewport::handleColourMap(void *data)
{
  Viewport *self = (Viewport *)data;

  assert(self);

  if (self->pixelTrans != NULL)
    self->pixelTrans->setColourMapEntries(0, 0);

  self->Fl_Widget::damage(FL_DAMAGE_ALL);
}


void Viewport::handleClipboardChange(int source, void *data)
{
  Viewport *self = (Viewport *)data;

  assert(self);

  if (!sendPrimary && (source == 0))
    return;

  Fl::paste(*self, source);
}


void Viewport::handlePointerEvent(const rfb::Point& pos, int buttonMask)
{
  if (!viewOnly) {
    if (pointerEventInterval == 0 || buttonMask != lastButtonMask) {
      cc->writer()->pointerEvent(pos, buttonMask);
    } else {
      if (!Fl::has_timeout(handlePointerTimeout, this))
        Fl::add_timeout((double)pointerEventInterval/1000.0,
                        handlePointerTimeout, this);
    }
    lastPointerPos = pos;
    lastButtonMask = buttonMask;
  }
}


void Viewport::handlePointerTimeout(void *data)
{
  Viewport *self = (Viewport *)data;

  assert(self);

  self->cc->writer()->pointerEvent(self->lastPointerPos, self->lastButtonMask);
}


rdr::U32 Viewport::translateKeyEvent(int keyCode, int origKeyCode, const char *keyText)
{
  unsigned ucs;

  // First check for function keys
  if ((keyCode > FL_F) && (keyCode <= FL_F_Last))
    return XK_F1 + (keyCode - FL_F - 1);

  // Numpad numbers
  if ((keyCode >= (FL_KP + '0')) && (keyCode <= (FL_KP + '9')))
    return XK_KP_0 + (keyCode - (FL_KP + '0'));

  // FLTK does some special remapping of numpad keys when numlock is off
  if ((origKeyCode >= FL_KP) && (origKeyCode <= FL_KP_Last)) {
    switch (keyCode) {
    case FL_F+1:
      return XK_KP_F1;
    case FL_F+2:
      return XK_KP_F2;
    case FL_F+3:
      return XK_KP_F3;
    case FL_F+4:
      return XK_KP_F4;
    case FL_Home:
      return XK_KP_Home;
    case FL_Left:
      return XK_KP_Left;
    case FL_Up:
      return XK_KP_Up;
    case FL_Right:
      return XK_KP_Right;
    case FL_Down:
      return XK_KP_Down;
    case FL_Page_Up:
      return XK_KP_Page_Up;
    case FL_Page_Down:
      return XK_KP_Page_Down;
    case FL_End:
      return XK_KP_End;
    case FL_Insert:
      return XK_KP_Insert;
    case FL_Delete:
      return XK_KP_Delete;
    }
  }

  // Then other special keys
  switch (keyCode) {
  case FL_BackSpace:
    return XK_BackSpace;
  case FL_Tab:
    return XK_Tab;
  case FL_Enter:
    return XK_Return;
  case FL_Pause:
    return XK_Pause;
  case FL_Scroll_Lock:
    return XK_Scroll_Lock;
  case FL_Escape:
    return XK_Escape;
  case FL_Home:
    return XK_Home;
  case FL_Left:
    return XK_Left;
  case FL_Up:
    return XK_Up;
  case FL_Right:
    return XK_Right;
  case FL_Down:
    return XK_Down;
  case FL_Page_Up:
    return XK_Page_Up;
  case FL_Page_Down:
    return XK_Page_Down;
  case FL_End:
    return XK_End;
  case FL_Print:
    return XK_Print;
  case FL_Insert:
    return XK_Insert;
  case FL_Menu:
    return XK_Menu;
  case FL_Help:
    return XK_Help;
  case FL_Num_Lock:
    return XK_Num_Lock;
  case FL_Shift_L:
    return XK_Shift_L;
  case FL_Shift_R:
    return XK_Shift_R;
  case FL_Control_L:
    return XK_Control_L;
  case FL_Control_R:
    return XK_Control_R;
  case FL_Caps_Lock:
    return XK_Caps_Lock;
  case FL_Meta_L:
    return XK_Super_L;
  case FL_Meta_R:
    return XK_Super_R;
  case FL_Alt_L:
    return XK_Alt_L;
  case FL_Alt_R:
    return XK_Alt_R;
  case FL_Delete:
    return XK_Delete;
  case FL_KP_Enter:
    return XK_KP_Enter;
  case FL_KP + '=':
    return XK_KP_Equal;
  case FL_KP + '*':
    return XK_KP_Multiply;
  case FL_KP + '+':
    return XK_KP_Add;
  case FL_KP + ',':
    return XK_KP_Separator;
  case FL_KP + '-':
    return XK_KP_Subtract;
  case FL_KP + '.':
    return XK_KP_Decimal;
  case FL_KP + '/':
    return XK_KP_Divide;
#ifdef HAVE_FLTK_MEDIAKEYS
  case FL_Volume_Down:
    return XF86XK_AudioLowerVolume;
  case FL_Volume_Mute:
    return XF86XK_AudioMute;
  case FL_Volume_Up:
    return XF86XK_AudioRaiseVolume;
  case FL_Media_Play:
    return XF86XK_AudioPlay;
  case FL_Media_Stop:
    return XF86XK_AudioStop;
  case FL_Media_Prev:
    return XF86XK_AudioPrev;
  case FL_Media_Next:
    return XF86XK_AudioNext;
  case FL_Home_Page:
    return XF86XK_HomePage;
  case FL_Mail:
    return XF86XK_Mail;
  case FL_Search:
    return XF86XK_Search;
  case FL_Back:
    return XF86XK_Back;
  case FL_Forward:
    return XF86XK_Forward;
  case FL_Stop:
    return XF86XK_Stop;
  case FL_Refresh:
    return XF86XK_Refresh;
  case FL_Sleep:
    return XF86XK_Sleep;
  case FL_Favorites:
    return XF86XK_Favorites;
#endif
  case XK_ISO_Level3_Shift:
    // FLTK tends to let this one leak through on X11...
    return XK_ISO_Level3_Shift;
  case XK_Multi_key:
    // Same for this...
    return XK_Multi_key;
  }

  // Unknown special key?
  if (keyText[0] == '\0') {
    vlog.error(_("Unknown FLTK key code %d (0x%04x)"), keyCode, keyCode);
    return XK_VoidSymbol;
  }

  // Look up the symbol the key produces and translate that from Unicode
  // to a X11 keysym.
  if (fl_utf_nb_char((const unsigned char*)keyText, strlen(keyText)) != 1) {
    vlog.error(_("Multiple characters given for key code %d (0x%04x): '%s'"),
               keyCode, keyCode, keyText);
    return XK_VoidSymbol;
  }

  ucs = fl_utf8decode(keyText, NULL, NULL);
  return ucs2keysym(ucs);
}


void Viewport::handleKeyEvent(int keyCode, int origKeyCode, const char *keyText, bool down)
{
  rdr::U32 keySym;

  if (viewOnly)
    return;

  // Because of the way keyboards work, we cannot expect to have the same
  // symbol on release as when pressed. This breaks the VNC protocol however,
  // so we need to keep track of what keysym a key _code_ generated on press
  // and send the same on release.
  if (!down) {
    DownMap::iterator iter;

    iter = downKeySym.find(origKeyCode);
    if (iter == downKeySym.end()) {
      vlog.error(_("Unexpected release of FLTK key code %d (0x%04x)"),
                 origKeyCode, origKeyCode);
      return;
    }

    vlog.debug("Key released: 0x%04x => 0x%04x", origKeyCode, iter->second);

    cc->writer()->keyEvent(iter->second, false);

    downKeySym.erase(iter);

    return;
  }

  keySym = translateKeyEvent(keyCode, origKeyCode, keyText);
  if (keySym == XK_VoidSymbol)
    return;

  vlog.debug("Key pressed: 0x%04x (0x%04x) '%s' => 0x%04x",
             origKeyCode, keyCode, keyText, keySym);

  downKeySym[origKeyCode] = keySym;
  cc->writer()->keyEvent(keySym, down);
}


void Viewport::initContextMenu()
{
  contextMenu->clear();

  contextMenu->add(_("Exit viewer"), 0, NULL, (void*)ID_EXIT, FL_MENU_DIVIDER);

#ifdef HAVE_FLTK_FULLSCREEN
  contextMenu->add(_("Full screen"), 0, NULL, (void*)ID_FULLSCREEN, FL_MENU_DIVIDER);
#endif

  contextMenu->add(_("Ctrl"), 0, NULL, (void*)ID_CTRL, FL_MENU_TOGGLE);
  contextMenu->add(_("Alt"), 0, NULL, (void*)ID_ALT, FL_MENU_TOGGLE);

  if (menuKeyCode) {
    char sendMenuKey[64];
    snprintf(sendMenuKey, 64, _("Send %s"), (const char *)menuKey);
    contextMenu->add(sendMenuKey, 0, NULL, (void*)ID_MENUKEY, 0);
    contextMenu->add("Secret shortcut menu key", menuKeyCode, NULL, (void*)ID_MENUKEY, FL_MENU_INVISIBLE);
  }

  contextMenu->add(_("Send Ctrl-Alt-Del"), 0, NULL, (void*)ID_CTRLALTDEL, FL_MENU_DIVIDER);

  contextMenu->add(_("Refresh screen"), 0, NULL, (void*)ID_REFRESH, FL_MENU_DIVIDER);

  contextMenu->add(_("Options..."), 0, NULL, (void*)ID_OPTIONS, 0);
  contextMenu->add(_("Connection info..."), 0, NULL, (void*)ID_INFO, 0);
  contextMenu->add(_("About TigerVNC viewer..."), 0, NULL, (void*)ID_ABOUT, FL_MENU_DIVIDER);

  contextMenu->add(_("Dismiss menu"), 0, NULL, (void*)ID_DISMISS, 0);
}


void Viewport::popupContextMenu()
{
  const Fl_Menu_Item *m;
  char buffer[1024];

  contextMenu->position(Fl::event_x(), Fl::event_y());

  m = contextMenu->popup();
  if (m == NULL)
    return;

  switch (m->argument()) {
  case ID_EXIT:
    exit_vncviewer();
    break;
#ifdef HAVE_FLTK_FULLSCREEN
  case ID_FULLSCREEN:
    if (window()->fullscreen_active())
      window()->fullscreen_off();
    else
      window()->fullscreen();
    break;
#endif
  case ID_CTRL:
    if (!viewOnly)
      cc->writer()->keyEvent(XK_Control_L, m->value());
    break;
  case ID_ALT:
    if (!viewOnly)
      cc->writer()->keyEvent(XK_Alt_L, m->value());
    break;
  case ID_MENUKEY:
    if (!viewOnly) {
      handleKeyEvent(menuKeyCode, menuKeyCode, "", true);
      handleKeyEvent(menuKeyCode, menuKeyCode, "", false);
    }
    break;
  case ID_CTRLALTDEL:
    if (!viewOnly) {
      cc->writer()->keyEvent(XK_Control_L, true);
      cc->writer()->keyEvent(XK_Alt_L, true);
      cc->writer()->keyEvent(XK_Delete, true);
      cc->writer()->keyEvent(XK_Delete, false);
      cc->writer()->keyEvent(XK_Alt_L, false);
      cc->writer()->keyEvent(XK_Control_L, false);
    }
    break;
  case ID_REFRESH:
    cc->refreshFramebuffer();
    break;
  case ID_OPTIONS:
    OptionsDialog::showDialog();
    break;
  case ID_INFO:
    if (fltk_escape(cc->connectionInfo(), buffer, sizeof(buffer)) < sizeof(buffer)) {
      fl_message_title(_("VNC connection info"));
      fl_message(buffer);
    }
    break;
  case ID_ABOUT:
    about_vncviewer();
    break;
  case ID_DISMISS:
    // Don't need to do anything
    break;
  }
}


void Viewport::setMenuKey()
{
  const char *menuKeyStr;

  menuKeyCode = 0;

  menuKeyStr = menuKey;
  if (menuKeyStr[0] == 'F') {
    int num = atoi(menuKeyStr + 1);
    if ((num >= 1) && (num <= 12))
      menuKeyCode = FL_F + num;
  }

  // Need to repopulate the context menu as it contains references to
  // the menu key
  initContextMenu();
}


void Viewport::handleOptions(void *data)
{
  Viewport *self = (Viewport*)data;

  self->setMenuKey();
}
