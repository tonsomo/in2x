/*
 *  Copyright (C) 2006 Jonn Blanchard (jonn.blanchard@gmail.com)
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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include "gp2x_sdl.h"
#include "gp2x.h"
#include "global.h"
#include "config.h"
#include "gp2x_kbd.h"
#include "gp2x_menu.h"
#include "gp2x_menu_kbd.h"

static void 
gp2x_display_screen_menu(void)
{
 	gp2x_sdl_blit_background();

 	gp2x_sdl_draw_rectangle(0,0,318,237,GP2X_MENU_BORDER_COLOR,0);
 	gp2x_sdl_draw_rectangle(1,1,314,233,GP2X_MENU_BORDER_COLOR,0);

    	gp2x_sdl_print(8, 8, "JZINTV", GP2X_MENU_SEL_COLOR);
    	gp2x_sdl_print(8, 16, "GP2X Version 1.0", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 24, "Original Work: Joe Zbiciak", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 32, "GP2X Port    : Jonn Blanchard", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 40, "With elements from the PSP Port By", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 48, "             : Ludovic Jacomme", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 56, "Keypad Graphic By", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 64, "             : DaveC", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 72, "Ideas and Testing By", GP2X_MENU_TEXT_COLOR);
    	gp2x_sdl_print(8, 80, "             : The GP2X community", GP2X_MENU_TEXT_COLOR);

    	gp2x_sdl_print(60, 110, "Press X to continue", GP2X_MENU_SEL_COLOR);
}

int 
gp2x_show_about()
{
  	int end_menu  = 0;

	while (! end_menu)
  	{
    		gp2x_display_screen_menu();
    		gp2x_sdl_flip();

    		unsigned long new_pad;
    		new_pad=gp2x_joystick_read();

    		if ((new_pad & (GP2X_L|GP2X_R|GP2X_START)) ==
        		(GP2X_L|GP2X_R|GP2X_START)) {
      		gp2x_sdl_exit(0);
    		} else if(new_pad&GP2X_X)
			end_menu=1;
	}
	gp2x_kbd_wait_no_button();
	return 0;
}

