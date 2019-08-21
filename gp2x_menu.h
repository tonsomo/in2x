/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

# ifndef _GP2X_MENU_H_
# define _GP2X_MENU_H_

# define GP2X_MENU_BORDER_COLOR     gp2x_sdl_rgb(0x80,0x80,0xF0)
# define GP2X_MENU_WARNING_COLOR    gp2x_sdl_rgb(0xFF,0x00,0x00)
# define GP2X_MENU_NOTE_COLOR       gp2x_sdl_rgb(0xFF,0xFF,0x00)
# define GP2X_MENU_BACKGROUND_COLOR gp2x_sdl_rgb(0x00,0x00,0x00)
# define GP2X_MENU_BLACK_COLOR      gp2x_sdl_rgb(0x00,0x00,0x00)
# define GP2X_MENU_AUTHOR_COLOR     gp2x_sdl_rgb(0x00,0x00,0xFF)

# define GP2X_MENU_TEXT_COLOR       gp2x_sdl_rgb(0x80,0x80,0x80)
# define GP2X_MENU_TEXT2_COLOR      gp2x_sdl_rgb(0xff,0xff,0xff)
# define GP2X_MENU_SEL_COLOR        gp2x_sdl_rgb(0x00,0xff,0xff)
# define GP2X_MENU_SEL2_COLOR       gp2x_sdl_rgb(0xFF,0x00,0x80)

# define GP2X_MENU_MIN_TIME         6000

  typedef struct menu_item_t {
    char *title;
  } menu_item_t;


   extern int gp2x_main_menu(int cl);

# endif
