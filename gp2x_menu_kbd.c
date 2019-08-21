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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include "gp2x.h"

#include "config.h"
#include "gp2x_sdl.h"
#include "gp2x_kbd.h"
#include "gp2x_menu.h"
#include "gp2x_fmgr.h"
#include "gp2x_menu_kbd.h"
#include "gp2x_danzeff.h"

# define MENU_KBD_UP           0
# define MENU_KBD_UPLEFT       1
# define MENU_KBD_LEFT         2
# define MENU_KBD_DOWNLEFT     3
# define MENU_KBD_DOWN         4
# define MENU_KBD_DOWNRIGHT    5
# define MENU_KBD_RIGHT        6
# define MENU_KBD_UPRIGHT      7
# define MENU_KBD_RTRIGGER   	 8
# define MENU_KBD_LTRIGGER     9
# define MENU_KBD_A           10
# define MENU_KBD_B           11
# define MENU_KBD_X           12
# define MENU_KBD_Y           13

# define MENU_KBD_LOAD        14
# define MENU_KBD_SAVE        15
# define MENU_KBD_RESET       16
# define MENU_KBD_BACK        17

# define MAX_MENU_KBD_ITEM (MENU_KBD_BACK + 1)

  static menu_item_t menu_list[] =
  { 
    "Up         :",
    "Up Left    :",
    "Left       :",
    "Down Left  :",
    "Down       :",
    "Down Right :",
    "Right      :",
    "Up Right   :",

    "LTrigger :",
    "RTrigger :",

    "A        :",
    "B        :",
    "X        :",
    "Y        :",

    "Load Keyboard",
    "Save Keyboard",
    "Reset Keyboard",

    "Back to Menu" 
  };

  static int cur_menu_id = MENU_KBD_LOAD;

  static int loc_kbd_mapping[ KBD_ALL_BUTTONS ];

static int
gp2x_kbd_menu_id_to_key_id(int menu_id)
{
  int kbd_id = 0;

  switch ( menu_id ) 
  {
    case MENU_KBD_UP        : kbd_id = KBD_UP;        break;
    case MENU_KBD_DOWN      : kbd_id = KBD_DOWN;      break;
    case MENU_KBD_LEFT      : kbd_id = KBD_LEFT;      break;
    case MENU_KBD_RIGHT     : kbd_id = KBD_RIGHT;     break;
    case MENU_KBD_UPLEFT    : kbd_id = KBD_UPLEFT;    break;
    case MENU_KBD_DOWNLEFT  : kbd_id = KBD_DOWNLEFT;  break;
    case MENU_KBD_UPRIGHT   : kbd_id = KBD_UPRIGHT;   break;
    case MENU_KBD_DOWNRIGHT : kbd_id = KBD_DOWNRIGHT; break;
    case MENU_KBD_Y         : kbd_id = KBD_Y;  		break;
    case MENU_KBD_X	    : kbd_id = KBD_X;	     	break;
    case MENU_KBD_A    	    : kbd_id = KBD_A;		break;
    case MENU_KBD_B	    : kbd_id = KBD_B;    	break;
    case MENU_KBD_LTRIGGER  : kbd_id = KBD_LTRIGGER;  break;
    case MENU_KBD_RTRIGGER  : kbd_id = KBD_RTRIGGER;  break;
  }
  return kbd_id;
}

static void 
gp2x_display_screen_kbd_menu(void)
{
  char buffer[32];
  char *scan;
  int menu_id = 0;
  int kbd_id  = 0;
  int intel_key = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  gp2x_sdl_blit_background();

  gp2x_sdl_fill_rectangle(0,0,318,237,GP2X_MENU_BACKGROUND_COLOR,0);
  gp2x_sdl_draw_rectangle(0,0,318,237,GP2X_MENU_BORDER_COLOR,0);
  gp2x_sdl_draw_rectangle(1,1,314,233,GP2X_MENU_BORDER_COLOR,0);

  gp2x_sdl_fill_print(  2, 0, " LTrigger: EXIT ", 
                     GP2X_MENU_WARNING_COLOR,  GP2X_MENU_BACKGROUND_COLOR);

  gp2x_sdl_fill_print(  2, 115, " B: Cancel  A/X: Select SELECT: Back ", 
                     GP2X_MENU_BORDER_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  
  x      = 6;
  y      = 8;
  y_step = 6;
  
  for (menu_id = 0; menu_id < MAX_MENU_KBD_ITEM; menu_id++) 
  {
    if (cur_menu_id == menu_id) color = GP2X_MENU_SEL_COLOR;
    else                        color = GP2X_MENU_TEXT_COLOR;
    gp2x_sdl_fill_print(x, y, menu_list[menu_id].title, color, GP2X_MENU_BACKGROUND_COLOR);

    if (menu_id == MENU_KBD_RESET) 
    {
      y += y_step;
    } else
    if ((menu_id >= MENU_KBD_UP       ) && 
        (menu_id <= MENU_KBD_Y)) 
    {
      kbd_id  = gp2x_kbd_menu_id_to_key_id(menu_id);
      intel_key = loc_kbd_mapping[kbd_id];
      if ((intel_key >= 0) && (intel_key < INTEL_MAX_KEY)) {
        strcpy(buffer, gp2x_intel_key_to_name[intel_key].name);
      } else 
      if (intel_key == -1) {
        sprintf(buffer, "UNASSIGNED");
      } else
      {
        sprintf(buffer, "KEY %d", intel_key);
      }
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);

      if (menu_id == MENU_KBD_Y) 
      {
        y += y_step;
      }
    }

    y += y_step;
  }

  gp2x_menu_display_save_name();
}

static void
gp2x_keyboard_menu_reset_kbd(void)
{
  gp2x_display_screen_kbd_menu();
  gp2x_sdl_fill_print(74, 110, "RESET KEYBOARD !", 
                     GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  gp2x_sdl_flip();
  gp2x_kbd_reset_mapping();
  sleep(1);

  memcpy(loc_kbd_mapping, gp2x_kbd_mapping, sizeof(gp2x_kbd_mapping));
}

static void
gp2x_keyboard_menu_load()
{
  int ret;

  ret = gp2x_fmgr_menu(FMGR_FORMAT_KBD);
  if (ret ==  1) /* load OK */
  {
    gp2x_display_screen_kbd_menu();
    gp2x_sdl_fill_print(74, 110, "File loaded !", 
                       GP2X_MENU_NOTE_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
  else 
  if (ret == -1) /* Load Error */
  {
    gp2x_display_screen_kbd_menu();
    gp2x_sdl_fill_print(74, 110, "Can't load file !", 
                       GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }

  memcpy(loc_kbd_mapping, gp2x_kbd_mapping, sizeof(gp2x_kbd_mapping));
}

static void
gp2x_keyboard_menu_mapping(int kbd_id, int step)
{
  if (step < 0) loc_kbd_mapping[kbd_id]--;
  else 
  if (step > 0) loc_kbd_mapping[kbd_id]++;

  if (loc_kbd_mapping[kbd_id] <  -1) loc_kbd_mapping[kbd_id] = INTEL_MAX_KEY-1;
  else
  if (loc_kbd_mapping[kbd_id] >= INTEL_MAX_KEY) loc_kbd_mapping[kbd_id] = 0;
}

static void
gp2x_keyboard_menu_save()
{
  int error;

  memcpy(gp2x_kbd_mapping, loc_kbd_mapping, sizeof(gp2x_kbd_mapping));

  error = intel_kbd_save();

  if (! error) /* save OK */
  {
    gp2x_display_screen_kbd_menu();
    gp2x_sdl_fill_print(74, 80, "File saved !", 
                       GP2X_MENU_NOTE_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
  else
  {
    gp2x_display_screen_kbd_menu();
    gp2x_sdl_fill_print(74, 80, "Can't save file !", 
                       GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
}

void 
gp2x_keyboard_menu(void)
{
  long new_pad;
  long old_pad;
  int  last_time;
  int  end_menu;
  int  kbd_id;
  int  intel_key;
  int  danzeff_mode;
  int  danzeff_key;

  gp2x_kbd_wait_no_button();

  old_pad      = 0;
  last_time    = 0;
  end_menu     = 0;
  kbd_id       = 0;

  danzeff_key  = 0;
  danzeff_mode = 0;

  memcpy(loc_kbd_mapping, gp2x_kbd_mapping, sizeof(gp2x_kbd_mapping));
  fprintf(stdout,"memcpy\n");

	while (! end_menu)
  	{
    		gp2x_display_screen_kbd_menu();

    		gp2x_sdl_flip();

    		unsigned long c;
    		c=gp2x_joystick_read();

		if ((SDL_GetTicks() - last_time) > GP2X_MENU_KBD_MIN_TIME) {
      		last_time = SDL_GetTicks();
    		} else continue;

    			if(c&(GP2X_L|GP2X_R|GP2X_START)) {
      			gp2x_sdl_exit(0);
    			} else
    				if ((c&GP2X_X ) || 
        			(c&GP2X_B) || 
        			(c&GP2X_RIGHT ) ||
        			(c&GP2X_LEFT  )) 
    				{
      				int step;

      				if (c&GP2X_LEFT)  step = -1;
      				else 
      				if (c&GP2X_RIGHT) step =  1;
      				else step =  0;

      				if ((cur_menu_id >= MENU_KBD_UP       ) && 
          				(cur_menu_id <= MENU_KBD_Y)) 
      				{
        					kbd_id = gp2x_kbd_menu_id_to_key_id(cur_menu_id);
        					gp2x_keyboard_menu_mapping(kbd_id, step); 
      				}
      				else
      				{
        					switch (cur_menu_id ) 
        					{
          						case MENU_KBD_LOAD  : gp2x_keyboard_menu_load();
                                		old_pad = new_pad = 0;
          						break;
          						case MENU_KBD_SAVE  : gp2x_keyboard_menu_save();
          						break;
          						case MENU_KBD_RESET : gp2x_keyboard_menu_reset_kbd();
          						break;
          						case MENU_KBD_BACK  : end_menu = 1;
          						break;
        					}
      				}
    				} else
    				if(c&GP2X_UP) {
					if (cur_menu_id > 0) cur_menu_id--;
      				else cur_menu_id = MAX_MENU_KBD_ITEM-1;
    				} else
    				if(c&GP2X_DOWN) {
      				if (cur_menu_id < (MAX_MENU_KBD_ITEM-1)) cur_menu_id++;
      				else cur_menu_id = 0;
    				} else  
    				if(c&GP2X_A) {
      				end_menu = -1;
    				} else 
    				if(c&GP2X_SELECT) {
      				end_menu = 1;
    				} else
    				if(c&GP2X_START) {
      				if ((cur_menu_id < MENU_KBD_UP       ) ||
          					(cur_menu_id > MENU_KBD_Y)) {
        					cur_menu_id = MENU_KBD_UP;
      				}
      				danzeff_mode = 1;
    				}
  		}

  		if (end_menu > 0) {
    			memcpy(gp2x_kbd_mapping, loc_kbd_mapping, sizeof(gp2x_kbd_mapping));
  		}
  		gp2x_kbd_wait_no_button();
}
