//
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

//
// Options frame.
//
// This deals with all the options the user can play with.
// It sets the encodings array and some booleans.
//

import java.awt.*;

class optionsFrame extends Frame {

  static String[] names = {
    "Encoding",
    "Compression level",
    "Use CopyRect",
    "Mouse buttons 2 and 3",
    "Raw pixel drawing",
    "CopyRect",
    "Share desktop",
  };

  static String[][] values = {
    { "Raw", "RRE", "CoRRE", "Hextile", "Zlib", "Tight" },
    { "Default", "1", "2", "3", "4", "5", "6", "7", "8", "9" },
    { "Yes", "No" },
    { "Normal", "Reversed" },
    { "Fast", "Reliable" },
    { "Fast", "Reliable" },
    { "Yes", "No" },
  };

  final int encodingIndex = 0, compressLevelIndex = 1, useCopyRectIndex = 2,
    mouseButtonIndex = 3, rawPixelDrawingIndex = 4, copyRectFastIndex = 5,
    shareDesktopIndex = 6;

  Label[] labels = new Label[names.length];
  Choice[] choices = new Choice[names.length];
  Button dismiss;
  vncviewer v;


  //
  // The actual data which other classes look at:
  //

  int[] encodings = new int[10];
  int nEncodings;

  int compressLevel;

  boolean reverseMouseButtons2And3;

  boolean drawEachPixelForRawRects;

  boolean copyRectFast;

  boolean shareDesktop;


  //
  // Constructor.  Set up the labels and choices from the names and values
  // arrays.
  //

  optionsFrame(vncviewer v1) {
    super("TightVNC Options");

    v = v1;

    GridBagLayout gridbag = new GridBagLayout();
    setLayout(gridbag);

    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;

    for (int i = 0; i < names.length; i++) {
      labels[i] = new Label(names[i]);
      gbc.gridwidth = 1;
      gridbag.setConstraints(labels[i],gbc);
      add(labels[i]);

      choices[i] = new Choice();
      gbc.gridwidth = GridBagConstraints.REMAINDER;
      gridbag.setConstraints(choices[i],gbc);
      add(choices[i]);

      for (int j = 0; j < values[i].length; j++) {
	choices[i].addItem(values[i][j]);
      }
    }

    dismiss = new Button("Dismiss");
    gbc.gridwidth = GridBagConstraints.REMAINDER;
    gridbag.setConstraints(dismiss,gbc);
    add(dismiss);

    pack();

    // Set up defaults

    choices[encodingIndex].select("Hextile");
    choices[compressLevelIndex].select("Default");
    choices[useCopyRectIndex].select("Yes");
    choices[mouseButtonIndex].select("Normal");
    choices[rawPixelDrawingIndex].select("Fast");
    choices[copyRectFastIndex].select("Fast");
    choices[shareDesktopIndex].select("Yes");

    // But let them be overridden by parameters

    for (int i = 0; i < names.length; i++) {
      String s = v.readParameter(names[i], false);
      if (s != null) {
	for (int j = 0; j < values[i].length; j++) {
	  if (s.equalsIgnoreCase(values[i][j])) {
	    choices[i].select(j);
	  }
	}
      }
    }

    // Make the booleans and encodings array correspond to the state of the GUI

    setEncodings();
    setOtherOptions();
  }


  //
  // Disable shareDesktop option
  //

  void disableShareDesktop() {
    labels[shareDesktopIndex].disable();
    choices[shareDesktopIndex].disable();
  }

  //
  // setEncodings looks at the encoding, compression level and
  // copyRect choices and sets the encodings array appropriately. It
  // also calls the vncviewer's setEncodings method to send a message
  // to the RFB server if necessary.
  //

  void setEncodings() {
    nEncodings = 0;
    if (choices[useCopyRectIndex].getSelectedItem().equals("Yes")) {
      encodings[nEncodings++] = rfbProto.EncodingCopyRect;
    }

    int preferredEncoding = rfbProto.EncodingRaw;

    if (choices[encodingIndex].getSelectedItem().equals("RRE")) {
      preferredEncoding = rfbProto.EncodingRRE;
      labels[compressLevelIndex].disable();
      choices[compressLevelIndex].disable();
    } else if (choices[encodingIndex].getSelectedItem().equals("CoRRE")) {
      preferredEncoding = rfbProto.EncodingCoRRE;
      labels[compressLevelIndex].disable();
      choices[compressLevelIndex].disable();
    } else if (choices[encodingIndex].getSelectedItem().equals("Hextile")) {
      preferredEncoding = rfbProto.EncodingHextile;
      labels[compressLevelIndex].disable();
      choices[compressLevelIndex].disable();
    } else if (choices[encodingIndex].getSelectedItem().equals("Zlib")) {
      preferredEncoding = rfbProto.EncodingZlib;
      labels[compressLevelIndex].enable();
      choices[compressLevelIndex].enable();
    } else if (choices[encodingIndex].getSelectedItem().equals("Tight")) {
      preferredEncoding = rfbProto.EncodingTight;
      labels[compressLevelIndex].enable();
      choices[compressLevelIndex].enable();
    }

    encodings[nEncodings++] = preferredEncoding;
    if (preferredEncoding != rfbProto.EncodingHextile) {
      encodings[nEncodings++] = rfbProto.EncodingHextile;
    }
    if (preferredEncoding != rfbProto.EncodingTight) {
      encodings[nEncodings++] = rfbProto.EncodingTight;
    }
    if (preferredEncoding != rfbProto.EncodingZlib) {
      encodings[nEncodings++] = rfbProto.EncodingZlib;
    }
    if (preferredEncoding != rfbProto.EncodingCoRRE) {
      encodings[nEncodings++] = rfbProto.EncodingCoRRE;
    }
    if (preferredEncoding != rfbProto.EncodingRRE) {
      encodings[nEncodings++] = rfbProto.EncodingRRE;
    }

    if (preferredEncoding == rfbProto.EncodingTight ||
        preferredEncoding == rfbProto.EncodingZlib) {
      int level;
      try {
        level =
          Integer.parseInt(choices[compressLevelIndex].getSelectedItem());
      }
      catch (NumberFormatException e) {
        level = -1;
      }
      if (level >= 1 && level <= 9) {
        encodings[nEncodings++] = rfbProto.EncodingCompressLevel0 + level;
      }
    }

    v.setEncodings();
  }

  //
  // setOtherOptions looks at the "other" choices (ones which don't set the
  // encoding) and sets the boolean flags appropriately.
  //

  void setOtherOptions() {

    reverseMouseButtons2And3
      = choices[mouseButtonIndex].getSelectedItem().equals("Reversed");

    drawEachPixelForRawRects
      = choices[rawPixelDrawingIndex].getSelectedItem().equals("Reliable");

    copyRectFast
      = choices[copyRectFastIndex].getSelectedItem().equals("Fast");

    shareDesktop
      = choices[shareDesktopIndex].getSelectedItem().equals("Yes");
  }



  //
  // Respond to an action i.e. choice or button press
  //

  public boolean action(Event evt, Object arg) {

    if (evt.target == dismiss) {
      hide();
      return true;

    } else if ((evt.target == choices[encodingIndex]) ||
	       (evt.target == choices[compressLevelIndex]) ||
	       (evt.target == choices[useCopyRectIndex])) {

      setEncodings();
      return true;

    } else if ((evt.target == choices[mouseButtonIndex]) ||
	       (evt.target == choices[rawPixelDrawingIndex]) ||
	       (evt.target == choices[copyRectFastIndex]) ||
	       (evt.target == choices[shareDesktopIndex])) {

      setOtherOptions();
      return true;
    }
    return false;
  }
}
