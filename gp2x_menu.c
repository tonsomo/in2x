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

#include "global.h"
#include "config.h"
#include "gp2x_sdl.h"
#include "gp2x_kbd.h"
#include "gp2x_menu.h"
#include "gp2x_fmgr.h"
#include "gp2x_menu_kbd.h"

extern SDL_Surface *back_surface;

# define MENU_SOUND        0
# define MENU_VOLUME	   1
# define MENU_SKIP_FPS     2
# define MENU_RENDER       3
# define MENU_CLOCK        4
# define MENU_SCREENSHOT   5
# define MENU_LOAD_ROM     6
# define MENU_KEYBOARD     7

# define MENU_SAVE_CFG     8
# define MENU_RESET        9
# define MENU_BACK        10
# define MENU_ABOUT	  11
# define MENU_EXIT        12

# define MAX_MENU_ITEM (MENU_EXIT + 1)

  static menu_item_t menu_list[] =
  { 
    "Sound enable    :",
    "Sound volume    :",
    "Skip frame      :",
    "Render mode     :",
    "Clock frequency :",
    "Save Screenshot :",
    "Load Rom",     
    "Keyboard", 
    "Save Config",
    "Reset",
    "Back",
    "About",
    "Quit"
  };

  static int cur_menu_id = MENU_LOAD_ROM;
  static int cur_slot    = 0;

  static int intel_snd_enable        = 0;
  static int intel_render_mode       = 2;
  static int gp2x_cpu_clock          = 200;
  static int intel_skip_fps          = 0;
  static int intel_slow_down_max     = 0;
  static int gp2x_volume	       = 100;

  int  end_menu, cartloaded, resetdone=0;

void
gp2x_menu_display_save_name()
{
  char buffer[62];
  int Length;
  snprintf(buffer, 61, "Game: %s", INTEL.intel_save_name);
  Length = strlen(buffer);
  //gp2x_sdl_fill_print(180 - (8*Length), 115, buffer, GP2X_MENU_TEXT2_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  gp2x_sdl_fill_print(4, 115, buffer, GP2X_MENU_TEXT2_COLOR, GP2X_MENU_BACKGROUND_COLOR);
}

static void 
gp2x_display_screen_menu(void)
{
  char buffer[32];
  int menu_id = 0;
  int slot_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  gp2x_sdl_blit_background();

  gp2x_sdl_draw_rectangle(0,0,318,237,GP2X_MENU_BORDER_COLOR,0);
  gp2x_sdl_draw_rectangle(1,1,314,233,GP2X_MENU_BORDER_COLOR,0);

  //gp2x_sdl_fill_print( 4, 0, " Start+L+R: EXIT ", 
  //                   GP2X_MENU_WARNING_COLOR,  GP2X_MENU_BACKGROUND_COLOR);

  gp2x_sdl_fill_print( 4, 0, " R: RESET/START GAME ", 
                     GP2X_MENU_NOTE_COLOR,  GP2X_MENU_BACKGROUND_COLOR);

  //gp2x_sdl_fill_print( 4, 236, " A: Cancel  B/X: Select  SELECT: Back ", 
  //                   GP2X_MENU_BORDER_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  //gp2x_sdl_fill_print( 260, 236, " By Nemesis/112 ",
  //                   GP2X_MENU_AUTHOR_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  
  x      = 4;
  y      = 8;
  y_step = 6;
  
  for (menu_id = 0; menu_id < MAX_MENU_ITEM; menu_id++) {
    color = GP2X_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = GP2X_MENU_SEL_COLOR;

    gp2x_sdl_fill_print(x, y, menu_list[menu_id].title, color, GP2X_MENU_BACKGROUND_COLOR);

    if (menu_id == MENU_SOUND) {
      if (intel_snd_enable) strcpy(buffer,"yes");
      else               strcpy(buffer,"no ");
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
    } else
    if (menu_id == MENU_SKIP_FPS) {
      sprintf(buffer,"%d", intel_skip_fps);
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
    } else
    if (menu_id == MENU_RENDER) {

      if (intel_render_mode == INTEL_RENDER_NORMAL) strcpy(buffer, "normal");
      else 
      if (intel_render_mode == INTEL_RENDER_X2    ) strcpy(buffer, "fit");
	else
      if (intel_render_mode == INTEL_RENDER_21    ) strcpy(buffer, "2:1");
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
    } else
    if (menu_id == MENU_CLOCK) {
      sprintf(buffer,"%d", gp2x_cpu_clock);
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
    } else
    if (menu_id == MENU_VOLUME) {
      sprintf(buffer,"%d", gp2x_volume);
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
    } else
    if (menu_id == MENU_SCREENSHOT) {
      sprintf(buffer,"%d", INTEL.gp2x_screenshot_id);
      gp2x_sdl_fill_print(74, y, buffer, color, GP2X_MENU_BACKGROUND_COLOR);
      y += y_step;
    } else
    if (menu_id == MENU_KEYBOARD) {
      y += y_step;
    } else
    if (menu_id == MENU_ABOUT) {
      y += y_step;
    }

    y += y_step;
  }

  gp2x_menu_display_save_name();
}

static void
gp2x_main_menu_reset(void)
{
  if(cartloaded==0) return;
  /* Reset ! */
  gp2x_display_screen_menu();
  gp2x_sdl_fill_print(74, 74, "RESET !", 
                     GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  gp2x_sdl_flip();
  emulator_reset();
  sleep(1);
}

static void
gp2x_main_menu_clock(int step)
{
	gp2x_cpu_clock+=step;
	if(gp2x_cpu_clock<66) gp2x_cpu_clock=66;
	if(gp2x_cpu_clock>320) gp2x_cpu_clock=320;
}

static void
gp2x_main_menu_skip_fps(int step)
{
  if (step > 0) {
    if (intel_skip_fps < 25) intel_skip_fps++;
  } else {
    if (intel_skip_fps >  0) intel_skip_fps--;
  }
}

static void
gp2x_main_menu_volume(int volume)
{
  if (volume > 0) {
    if (gp2x_volume < 100) gp2x_volume++;
  } else {
    if (gp2x_volume >  0) gp2x_volume--;
  }
}

static void
gp2x_main_menu_render(int step)
{
  if (step > 0) {
    if (intel_render_mode < INTEL_LAST_RENDER) intel_render_mode++;
    else                                       intel_render_mode = 0;
  } else {
    if (intel_render_mode > 0) intel_render_mode--;
    else                       intel_render_mode = INTEL_LAST_RENDER;
  }
}

static void
gp2x_main_menu_load(int format)
{
  int ret;

  ret = gp2x_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    gp2x_display_screen_menu();
    gp2x_sdl_fill_print(74, 110, "File loaded !", 
                       GP2X_MENU_NOTE_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
    cartloaded=1;
    resetdone=1;
  }
  else 
  if (ret == -1) /* Load Error */
  {
    gp2x_display_screen_menu();
    gp2x_sdl_fill_print(74, 160, "Can't load file !", 
                       GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
}

static void
gp2x_main_menu_screenshot(void)
{
  gp2x_screenshot_mode = 10;
}

static void
gp2x_main_menu_validate(void)
{
  /* Validate */
  INTEL.intel_snd_enable    = intel_snd_enable;
  INTEL.intel_render_mode   = intel_render_mode;
  INTEL.gp2x_cpu_clock       = gp2x_cpu_clock;
  INTEL.gp2x_skip_max_frame  = intel_skip_fps;
  INTEL.intel_slow_down_max = intel_slow_down_max;
  INTEL.gp2x_skip_cur_frame  = 0;
  INTEL.volume = gp2x_volume;

  gp2x_set_cpu_speed(INTEL.gp2x_cpu_clock);
  gp2x_sound_volume(INTEL.volume,INTEL.volume);
}

static void
gp2x_main_menu_save_config()
{
  int error;

  gp2x_main_menu_validate();

  error = intel_save_configuration();

  if (! error) /* save OK */
  {
    gp2x_display_screen_menu();
    gp2x_sdl_fill_print(74, 160, "File saved !", 
                       GP2X_MENU_NOTE_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
  else 
  {
    gp2x_display_screen_menu();
    gp2x_sdl_fill_print(74, 160, "Can't save file !", 
                       GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    gp2x_sdl_flip();
    sleep(1);
  }
}

int
gp2x_main_menu_exit(void)
{
	gp2x_sdl_flip();
  	//gp2x_display_screen_menu();
  	gp2x_kbd_wait_no_button();

  	int end_menu  = 0;

	while (! end_menu)
  	{
		gp2x_sdl_clear_screen(0);
  		gp2x_sdl_fill_print(4, 160, "Really Quit? press Y to confirm, ", 
                     GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);
  		gp2x_sdl_fill_print(15, 170, "A to cancel !", 
                     GP2X_MENU_WARNING_COLOR, GP2X_MENU_BACKGROUND_COLOR);

    		gp2x_sdl_flip();

    		unsigned long new_pad;
    		new_pad=gp2x_joystick_read();

    		if (new_pad & GP2X_Y) {
      		gp2x_sdl_exit(0);
			return 1;
    		} else if(new_pad&GP2X_A)
			end_menu=1;
	}
	gp2x_kbd_wait_no_button();
	return 0;

  	gp2x_display_screen_menu();
  	gp2x_sdl_flip();
}

int
gp2x_main_menu_about(void)
{
  	gp2x_kbd_wait_no_button();
	gp2x_show_about();
}

int 
gp2x_main_menu(int cl)
{
  int  last_time;

  cartloaded=cl;

  audio_pause();

  gp2x_kbd_wait_no_button();

  last_time = 0;
  end_menu  = 0;

  intel_snd_enable     = INTEL.intel_snd_enable;
  intel_render_mode    = INTEL.intel_render_mode;
  intel_skip_fps       = INTEL.gp2x_skip_max_frame;
  intel_slow_down_max  = INTEL.intel_slow_down_max;
  gp2x_cpu_clock       = INTEL.gp2x_cpu_clock;
  gp2x_volume	     = INTEL.volume;

  while (! end_menu)
  {
    	gp2x_display_screen_menu();
    	gp2x_sdl_flip();


    	unsigned long new_pad;
    	new_pad=gp2x_joystick_read();

    	if (new_pad&GP2X_R) {
		if(cartloaded==1) {
      		gp2x_main_menu_reset();
      		end_menu = 1;
		}
    	} else if ((new_pad&GP2X_X ) || (new_pad&GP2X_B) || (new_pad&GP2X_RIGHT ) || (new_pad&GP2X_LEFT  )) {
     		int step;
     		if (new_pad&GP2X_LEFT)  step = -1;
     		else 
     		if (new_pad&GP2X_RIGHT) step =  1;
     		else step =  0;

     		switch (cur_menu_id ) 
     		{
       		case MENU_SOUND     : intel_snd_enable = ! intel_snd_enable;
        		break;              
        		case MENU_SKIP_FPS  : gp2x_main_menu_skip_fps( step );
        		break;              
        		case MENU_RENDER    : gp2x_main_menu_render( step );
        		break;              
        		case MENU_CLOCK     : gp2x_main_menu_clock( step );
        		break;
        		case MENU_VOLUME    : gp2x_main_menu_volume( step );
        		break;
        		case MENU_LOAD_ROM  : gp2x_main_menu_load(FMGR_FORMAT_ROM);
        		break;              
        		case MENU_KEYBOARD   : gp2x_keyboard_menu();
        		break;
        		case MENU_SCREENSHOT : gp2x_main_menu_screenshot();
                        		     end_menu = 1;
        		break;              
        		case MENU_SAVE_CFG  : gp2x_main_menu_save_config();
        		break;
        		case MENU_RESET     : gp2x_main_menu_reset();
                            			end_menu = 1;
							resetdone = 1;
        		break;
        		case MENU_BACK      : end_menu = 1;
        		break;
        		case MENU_EXIT      : gp2x_main_menu_exit();
        		break;
			case MENU_ABOUT	  : gp2x_main_menu_about();
      	}

    	} else if(new_pad&GP2X_UP) {
      	if (cur_menu_id > 0) cur_menu_id--;
      	else cur_menu_id = MAX_MENU_ITEM-1;
    	} else if(new_pad&GP2X_DOWN) {
      	if (cur_menu_id < (MAX_MENU_ITEM-1)) cur_menu_id++;
      	else cur_menu_id = 0;
    	} else if(new_pad&GP2X_A) {
      	/* Cancel */
      		end_menu = -1;
    		} else 
    			if(new_pad&GP2X_START) {
      		/* Back to INTEL.*/
      		end_menu = 1;
    		}
   	}
	if(cartloaded==0) end_menu=0;

	if(cartloaded==1)
	{ 
  		if (end_menu > 0) {
    			fprintf(stdout,"Validate\n");
			fflush(stdout);
    			gp2x_main_menu_validate();
  		}

  		fprintf(stdout,"wait\n");
		fflush(stdout);
  		gp2x_kbd_wait_no_button();

    		fprintf(stdout,"Clear Screen\n");
		fflush(stdout);
  		gp2x_sdl_clear_screen( GP2X_MENU_BLACK_COLOR );
  		gp2x_sdl_flip();
  		gp2x_sdl_clear_screen( GP2X_MENU_BLACK_COLOR );
  		gp2x_sdl_flip();

  		fprintf(stdout,"audio_resume\n");
		fflush(stdout);
  		audio_resume();
    		fprintf(stdout,"exit\n");
		fflush(stdout);
  		return resetdone;
	} else return 0;
}

