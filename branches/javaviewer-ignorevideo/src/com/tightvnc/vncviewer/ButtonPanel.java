//
//  Copyright (C) 2001,2002 HorizonLive.com, Inc.  All Rights Reserved.
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

//
// ButtonPanel class implements panel with four buttons in the
// VNCViewer desktop window.
//

package com.tightvnc.vncviewer;

import java.awt.*;
import java.awt.event.*;
import java.io.*;

class ButtonPanel extends Panel implements ActionListener {

  VncViewer viewer;
  Button disconnectButton;
  Button optionsButton;
  Button recordButton;
  Button clipboardButton;
  Button ctrlAltDelButton;
  Button refreshButton;
  Button selectButton;
  Button videoFreezeButton;

  final String enableVideoFreezeLabel = "Ignore Video";
  final String disableVideoFreezeLabel = "Enable Video";
  final String selectEnterLabel = "Select Video Area";
  final String selectLeaveLabel = "Hide Selection";

  ButtonPanel(VncViewer v) {
    viewer = v;

    setLayout(new FlowLayout(FlowLayout.LEFT, 0, 0));
    disconnectButton = new Button("Disconnect");
    disconnectButton.setEnabled(false);
    add(disconnectButton);
    disconnectButton.addActionListener(this);
    optionsButton = new Button("Options");
    add(optionsButton);
    optionsButton.addActionListener(this);
    clipboardButton = new Button("Clipboard");
    clipboardButton.setEnabled(false);
    add(clipboardButton);
    clipboardButton.addActionListener(this);
    if (viewer.rec != null) {
      recordButton = new Button("Record");
      add(recordButton);
      recordButton.addActionListener(this);
    }
    ctrlAltDelButton = new Button("Send Ctrl-Alt-Del");
    ctrlAltDelButton.setEnabled(false);
    add(ctrlAltDelButton);
    ctrlAltDelButton.addActionListener(this);
    refreshButton = new Button("Refresh");
    refreshButton.setEnabled(false);
    add(refreshButton);
    refreshButton.addActionListener(this);
  }

  /**
   * Add video selection button to the ButtonPanel.
   */
  public void addSelectButton() {
    selectButton = new Button(selectEnterLabel);
    selectButton.setEnabled(false);
    add(selectButton);
    selectButton.addActionListener(this);
  }

  /**
   * Add video ignore button to the ButtonPanel.
   */
  public void addVideoFreezeButton() {
    videoFreezeButton = new Button(enableVideoFreezeLabel);
    add(videoFreezeButton);
    videoFreezeButton.addActionListener(this);
  }

  //
  // Enable buttons on successful connection.
  //

  public void enableButtons() {
    disconnectButton.setEnabled(true);
    clipboardButton.setEnabled(true);
    refreshButton.setEnabled(true);
    if (selectButton != null) {
      selectButton.setEnabled(true);
    }
  }

  //
  // Disable all buttons on disconnect.
  //

  public void disableButtonsOnDisconnect() {
    remove(disconnectButton);
    disconnectButton = new Button("Hide desktop");
    disconnectButton.setEnabled(true);
    add(disconnectButton, 0);
    disconnectButton.addActionListener(this);

    optionsButton.setEnabled(false);
    clipboardButton.setEnabled(false);
    ctrlAltDelButton.setEnabled(false);
    refreshButton.setEnabled(false);
    if (selectButton != null) {
      selectButton.setEnabled(false);
    }
  }

  //
  // Enable/disable controls that should not be available in view-only
  // mode.
  //

  public void enableRemoteAccessControls(boolean enable) {
    ctrlAltDelButton.setEnabled(enable);
  }

  //
  // Event processing.
  //

  public void actionPerformed(ActionEvent evt) {

    viewer.moveFocusToDesktop();

    if (evt.getSource() == disconnectButton) {
      viewer.disconnect();

    } else if (evt.getSource() == optionsButton) {
      viewer.options.setVisible(!viewer.options.isVisible());

    } else if (evt.getSource() == recordButton) {
      viewer.rec.setVisible(!viewer.rec.isVisible());

    } else if (evt.getSource() == clipboardButton) {
      viewer.clipboard.setVisible(!viewer.clipboard.isVisible());
    } else if (evt.getSource() == videoFreezeButton) {

      //
      // Send video freeze message to server and change caption of button
      //

      //
      // TODO: Move this code to another place.
      //

      boolean sendOk = true;
      boolean currentFreezeState =
              videoFreezeButton.getLabel().equals(disableVideoFreezeLabel);
      try {
        viewer.rfb.trySendVideoFreeze(!currentFreezeState);
      } catch (IOException ex) {
        sendOk = false;
        ex.printStackTrace();
      }
      if (sendOk) {
        if (!currentFreezeState) {
            videoFreezeButton.setLabel(disableVideoFreezeLabel);
        } else {
            videoFreezeButton.setLabel(enableVideoFreezeLabel);
        }
      }
    } else if (evt.getSource() == ctrlAltDelButton) {
      try {
        final int modifiers = InputEvent.CTRL_MASK | InputEvent.ALT_MASK;

        KeyEvent ctrlAltDelEvent =
          new KeyEvent(this, KeyEvent.KEY_PRESSED, 0, modifiers, 127);
        viewer.rfb.writeKeyEvent(ctrlAltDelEvent);

        ctrlAltDelEvent =
          new KeyEvent(this, KeyEvent.KEY_RELEASED, 0, modifiers, 127);
        viewer.rfb.writeKeyEvent(ctrlAltDelEvent);

      } catch (IOException e) {
        e.printStackTrace();
      }
    } else if (evt.getSource() == refreshButton) {
      try {
	RfbProto rfb = viewer.rfb;
	rfb.writeFramebufferUpdateRequest(0, 0, rfb.framebufferWidth,
					  rfb.framebufferHeight, false);
      } catch (IOException e) {
        e.printStackTrace();
      }
    } else if (selectButton != null && evt.getSource() == selectButton) {
      if (viewer.vc != null) {
        boolean isSelecting = viewer.vc.isInSelectionMode();
        if (!isSelecting) {
          selectButton.setLabel(selectLeaveLabel);
          viewer.vc.enableSelection(true);
        } else {
          selectButton.setLabel(selectEnterLabel);
          viewer.vc.enableSelection(false);
        }
      }
    }
  }
}

