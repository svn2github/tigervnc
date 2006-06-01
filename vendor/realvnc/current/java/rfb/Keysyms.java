/* Copyright (C) 2002-2003 RealVNC Ltd.  All Rights Reserved.
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
//
// Keysyms - defines X keysyms for non-character keys.  All keysyms
// corresponding to characters should be generated by calling
// UnicodeToKeysym.translate().
//

package rfb;

public class Keysyms {

  public static final int BackSpace = 0xFF08;
  public static final int Tab = 0xFF09;
  public static final int Linefeed = 0xFF0A;
  public static final int Clear = 0xFF0B;
  public static final int Return = 0xFF0D;
  public static final int Pause = 0xFF13;
  public static final int Scroll_Lock = 0xFF14;
  public static final int Sys_Req = 0xFF15;
  public static final int Escape = 0xFF1B;
  public static final int Delete = 0xFFFF;

  public static final int Home = 0xFF50;
  public static final int Left = 0xFF51;
  public static final int Up = 0xFF52;
  public static final int Right = 0xFF53;
  public static final int Down = 0xFF54;
  public static final int Prior = 0xFF55;
  public static final int Page_Up = 0xFF55;
  public static final int Next = 0xFF56;
  public static final int Page_Down = 0xFF56;
  public static final int End = 0xFF57;
  public static final int Begin = 0xFF58;

  public static final int Select = 0xFF60;
  public static final int Print = 0xFF61;
  public static final int Execute = 0xFF62;
  public static final int Insert = 0xFF63;
  public static final int Undo = 0xFF65;
  public static final int Redo = 0xFF66;
  public static final int Menu = 0xFF67;
  public static final int Find = 0xFF68;
  public static final int Cancel = 0xFF69;
  public static final int Help = 0xFF6A;
  public static final int Break = 0xFF6B;
  public static final int Mode_switch = 0xFF7E;
  public static final int script_switch = 0xFF7E;
  public static final int Num_Lock = 0xFF7F;

  public static final int F1 = 0xFFBE;
  public static final int F2 = 0xFFBF;
  public static final int F3 = 0xFFC0;
  public static final int F4 = 0xFFC1;
  public static final int F5 = 0xFFC2;
  public static final int F6 = 0xFFC3;
  public static final int F7 = 0xFFC4;
  public static final int F8 = 0xFFC5;
  public static final int F9 = 0xFFC6;
  public static final int F10 = 0xFFC7;
  public static final int F11 = 0xFFC8;
  public static final int F12 = 0xFFC9;

  public static final int Shift_L = 0xFFE1;
  public static final int Shift_R = 0xFFE2;
  public static final int Control_L = 0xFFE3;
  public static final int Control_R = 0xFFE4;
  public static final int Meta_L = 0xFFE7;
  public static final int Meta_R = 0xFFE8;
  public static final int Alt_L = 0xFFE9;
  public static final int Alt_R = 0xFFEA;
}
