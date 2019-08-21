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

int ichosen=4;

void 
gp2x_display_screen_menu(void)
{
	int x,y,i;
	x=40; y=10;
	for(i=0;i<12;i++)
	{
		if(i==3||i==6||i==9)
		{
			x=40; y+=25;
		}

		Uint32 bg,fg;

		if(ichosen==i)
		{
			bg=0;
			fg=gp2x_sdl_rgb(255,255,255);
		}
		else
		{
			fg=0;
			bg=gp2x_sdl_rgb(255,255,255);
		}

		gp2x_sdl_fill_rectangle(x, y, 50, 50, bg, 0);
		gp2x_sdl_draw_rectangle(x, y, 50, 50, fg, 0);
		if(i<9)
			gp2x_sdl_put_char(x+12, y+12, fg, 0, 49+i, 1, 0);
		else if(i==9)
			gp2x_sdl_put_char(x+12, y+12, fg, 0, 67, 1, 0);
		else if(i==10)
			gp2x_sdl_put_char(x+12, y+12, fg, 0, 48, 1, 0);
		else
			gp2x_sdl_put_char(x+12, y+12, fg, 0, 69, 1, 0);
		x+=25;
	}
}

int 
gp2x_show_pad()
{
  	int end_menu  = 0; ichosen=4;
	gp2x_kbd_wait_no_button();
	Uint32 last_time=0;

	while (! end_menu)
  	{
    		gp2x_display_screen_menu();
    		gp2x_sdl_flip();

    		unsigned long new_pad;
    		new_pad=gp2x_joystick_read();

    		if ((new_pad & (GP2X_L|GP2X_R|GP2X_START)) ==
        		(GP2X_L|GP2X_R|GP2X_START)) {
      		gp2x_sdl_exit(0);
    		} else
		{
			if(new_pad&GP2X_LEFT)
				if(ichosen!=0&&ichosen!=3&&ichosen!=6)
					ichosen-=1;
			if(new_pad&GP2X_RIGHT)
				if(ichosen!=2&&ichosen!=5&&ichosen!=8)
					ichosen+=1;
			if(new_pad&GP2X_UP)
				if(ichosen!=0&&ichosen!=1&&ichosen!=2)
					ichosen-=3;
			if(new_pad&GP2X_DOWN)
				if(ichosen!=9&&ichosen!=10&&ichosen!=11)
					ichosen+=3;
			if(new_pad&GP2X_X) // chosen;
				end_menu = 1;
			if(new_pad&GP2X_START || new_pad&GP2X_A)
			{
				ichosen=-1;
				end_menu=1;
			}
		}
		if(SDL_GetTicks()-last_time<=150)
			SDL_Delay(150);
		else
			last_time=SDL_GetTicks();
	}
	gp2x_kbd_wait_no_button();
	return ichosen+INTEL_PD0L_KP1;
}

