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

# ifndef _GP2X_KBD_H_
# define _GP2X_KBD_H_

 enum intel_keys_emum {
  INTEL_RESET = 0,   
  INTEL_PD0L_KP1,   
  INTEL_PD0L_KP2,   
  INTEL_PD0L_KP3,   
  INTEL_PD0L_KP4,   
  INTEL_PD0L_KP5,   
  INTEL_PD0L_KP6,   
  INTEL_PD0L_KP7,   
  INTEL_PD0L_KP8,   
  INTEL_PD0L_KP9,   
  INTEL_PD0L_KPC,   
  INTEL_PD0L_KP0,   
  INTEL_PD0L_KPE,   
  INTEL_PD0L_A_T,   
  INTEL_PD0L_A_L,   
  INTEL_PD0L_A_R,   
  INTEL_PD0L_D_E,   
  INTEL_PD0L_D_ENE, 
  INTEL_PD0L_D_NE,  
  INTEL_PD0L_D_NNE, 
  INTEL_PD0L_D_N,   
  INTEL_PD0L_D_NNW, 
  INTEL_PD0L_D_NW,  
  INTEL_PD0L_D_WNW, 
  INTEL_PD0L_D_W,   
  INTEL_PD0L_D_WSW, 
  INTEL_PD0L_D_SW,  
  INTEL_PD0L_D_SSW, 
  INTEL_PD0L_D_S,   
  INTEL_PD0L_D_SSE, 
  INTEL_PD0L_D_SE,  
  INTEL_PD0L_J_E,   
  INTEL_PD0L_J_ENE, 
  INTEL_PD0L_J_NE,  
  INTEL_PD0L_J_NNE, 
  INTEL_PD0L_J_N,   
  INTEL_PD0L_J_NNW, 
  INTEL_PD0L_J_NW,  
  INTEL_PD0L_J_WNW, 
  INTEL_PD0L_J_W,   
  INTEL_PD0L_J_WSW, 
  INTEL_PD0L_J_SW,  
  INTEL_PD0L_J_SSW, 
  INTEL_PD0L_J_S,   
  INTEL_PD0L_J_SSE, 
  INTEL_PD0L_J_SE,  
  INTEL_PD0L_J_ESE, 
  INTEL_PD0R_KP1,   
  INTEL_PD0R_KP2,   
  INTEL_PD0R_KP3,   
  INTEL_PD0R_KP4,   
  INTEL_PD0R_KP5,   
  INTEL_PD0R_KP6,   
  INTEL_PD0R_KP7,   
  INTEL_PD0R_KP8,   
  INTEL_PD0R_KP9,   
  INTEL_PD0R_KPC,   
  INTEL_PD0R_KP0,   
  INTEL_PD0R_KPE,   
  INTEL_PD0R_A_T,   
  INTEL_PD0R_A_L,   
  INTEL_PD0R_A_R,   
  INTEL_PD0R_D_E,   
  INTEL_PD0R_D_ENE, 
  INTEL_PD0R_D_NE,  
  INTEL_PD0R_D_NNE, 
  INTEL_PD0R_D_N,   
  INTEL_PD0R_D_NNW, 
  INTEL_PD0R_D_NW,  
  INTEL_PD0R_D_WNW, 
  INTEL_PD0R_D_W,   
  INTEL_PD0R_D_WSW, 
  INTEL_PD0R_D_SW,  
  INTEL_PD0R_D_SSW, 
  INTEL_PD0R_D_S,   
  INTEL_PD0R_D_SSE, 
  INTEL_PD0R_D_SE,  
  INTEL_PD0R_J_E,   
  INTEL_PD0R_J_ENE, 
  INTEL_PD0R_J_NE,  
  INTEL_PD0R_J_NNE, 
  INTEL_PD0R_J_N,   
  INTEL_PD0R_J_NNW, 
  INTEL_PD0R_J_NW,  
  INTEL_PD0R_J_WNW, 
  INTEL_PD0R_J_W,   
  INTEL_PD0R_J_WSW, 
  INTEL_PD0R_J_SW,  
  INTEL_PD0R_J_SSW, 
  INTEL_PD0R_J_S,   
  INTEL_PD0R_J_SSE, 
  INTEL_PD0R_J_SE,  
  INTEL_PD0R_J_ESE, 
  INTEL_MAX_KEY 
  };

# define KBD_UP           0
# define KBD_UPLEFT       1
# define KBD_LEFT         2
# define KBD_DOWNLEFT     3
# define KBD_DOWN 	  4
# define KBD_DOWNRIGHT    5
# define KBD_RIGHT        6
# define KBD_UPRIGHT      7
# define KBD_START     	  8
# define KBD_SELECT       9
# define KBD_LTRIGGER    10
# define KBD_RTRIGGER    11 
# define KBD_A		 12
# define KBD_B		 13
# define KBD_X		 14
# define KBD_Y		 15
# define KBD_VOLUP	 16
# define KBD_VOLDOWN	 17
# define KBD_TAT		 18

enum MAP_KEY
{
	VK_UP         , // 0
	VK_UP_LEFT    , // 1
	VK_LEFT       , // 2
	VK_DOWN_LEFT  , // 3
	VK_DOWN       , // 4
	VK_DOWN_RIGHT , // 5
	VK_RIGHT      , // 6
	VK_UP_RIGHT   , // 7
	VK_START      , // 8
	VK_SELECT     , // 9
	VK_FL         , // 10
	VK_FR         , // 11
	VK_FA         , // 12
	VK_FB         , // 13
	VK_FX         , // 14
	VK_FY         , // 15
	VK_VOL_UP     , // 16
	VK_VOL_DOWN   , // 17
	VK_TAT          // 18
};

# define KBD_MAX_BUTTONS 19

# define KBD_ALL_BUTTONS 19

  typedef struct intel_key_trans {
    char       name[16];
    v_uint_32   *pvalue;
    uint_32    mask_and[2];
    uint_32    mask_or[2];

  } intel_key_trans;

  extern int gp2x_screenshot_mode;
  extern int gp2x_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern struct intel_key_trans gp2x_intel_key_to_name[INTEL_MAX_KEY];

  extern int  gp2x_update_keys(void);
  extern void kbd_wait_start(void);
  extern void gp2x_init_keyboard(void);
  extern void gp2x_kbd_wait_no_button(void);
  extern int  gp2x_kbd_is_danzeff_mode(void);

# endif
