/*
 * ============================================================================
 *  Title:    Graphics Interface Routines
 *  Author:   J. Zbiciak, J. Tanner
 *  $Id: gfx.c,v 1.18 2001/02/03 02:34:21 im14u2c Exp $
 * ============================================================================
 *  GFX_INIT         -- Initializes a gfx_t object.
 *  GFX_TICK         -- Services a gfx_t tick.
 *  GFX_VID_ENABLE   -- Alert gfx that video has been enabled or blanked
 *  GFX_SET_BORD     -- Set the border / offset parameters for the display
 * ============================================================================
 *  GFX_T            -- Graphics subsystem object.
 *  GFX_PVT_T        -- Private internal state to gfx_t structure.
 *  GFX_STIC_PALETTE -- The STIC palette.
 * ============================================================================
 *  The graphics subsystem provides an abstraction layer between the 
 *  emulator and the graphics library being used.  Theoretically, this 
 *  should allow easy porting to other graphics libraries.
 *
 *  TODO:  
 *   -- Make use of dirty rectangle updating for speed.
 * ============================================================================
 */
#include <SDL/SDL.h>
#include "gp2x_sdl.h"
#include "config.h"
#include "global.h"
#include "periph/periph.h"
#include "gfx.h"
#include "file/file.h"
#include "mvi/mvi.h"
#include "gif/gif_enc.h"
#include "gp2x_kbd.h"

/*
 * ============================================================================
 *  GFX_PVT_T        -- Private internal state to gfx_t structure.
 * ============================================================================
 */
typedef struct gfx_pvt_t
{
    SDL_Surface *scr;               /*  Screen surface.                 */
    SDL_Color   pal_on [32];        /*  Palette when video is enabled.  */
    SDL_Color   pal_off[32];        /*  Palette when video is blanked.  */
    int         vid_enable;         /*  Video enable flag.              */
    int         ofs_x, ofs_y;       /*  X/Y offsets for centering img.  */

    int         movie_init;         /*  Is movie structure initialized? */
    mvi_t       *movie;             /*  Pointer to mvi_t to reduce deps */

} gfx_pvt_t;


/*
 * ============================================================================
 *  GFX_STIC_PALETTE -- The STIC palette.
 * ============================================================================
 */
//LUDO:
LOCAL uint_16 gfx_stic_palette_rgb[32];

LOCAL uint_8 gfx_stic_palette[32][3] = 
{
    /* -------------------------------------------------------------------- */
    /*  I generated these colors by directly eyeballing my television       */
    /*  while it was next to my computer monitor.  I then tweaked each      */
    /*  color until it was pretty close to my TV.  Bear in mind that        */
    /*  NTSC (said to mean "Never The Same Color") is highly susceptible    */
    /*  to Tint/Brightness/Contrast settings, so your mileage may vary      */
    /*  with this particular pallete setting.                               */
    /* -------------------------------------------------------------------- */
    { 0x00, 0x00, 0x00 },
    { 0x00, 0x2D, 0xFF },
    { 0xFF, 0x3D, 0x10 },
    { 0xC9, 0xCF, 0xAB },
    { 0x38, 0x6B, 0x3F },
    { 0x00, 0xA7, 0x56 },
    { 0xFA, 0xEA, 0x50 },
    { 0xFF, 0xFC, 0xFF },
    { 0xBD, 0xAC, 0xC8 },
    { 0x24, 0xB8, 0xFF },
    { 0xFF, 0xB4, 0x1F },
    { 0x54, 0x6E, 0x00 },
    { 0xFF, 0x4E, 0x57 },
    { 0xA4, 0x96, 0xFF },
    { 0x75, 0xCC, 0x80 },
    { 0xB5, 0x1A, 0x58 },

    /* -------------------------------------------------------------------- */
    /*  This pink color is used for drawing rectangles around sprites.      */
    /*  It's a temporary hack.                                              */
    /* -------------------------------------------------------------------- */
    { 0xFF, 0x80, 0x80 },
    /* -------------------------------------------------------------------- */
    /*  Grey shades used for misc tasks (not currently used).               */
    /* -------------------------------------------------------------------- */
    { 0x11, 0x11, 0x11 },
    { 0x22, 0x22, 0x22 },
    { 0x33, 0x33, 0x33 },
    { 0x44, 0x44, 0x44 },
    { 0x55, 0x55, 0x55 },
    { 0x66, 0x66, 0x66 },
    { 0x77, 0x77, 0x77 },
    { 0x88, 0x88, 0x88 },
    { 0x99, 0x99, 0x99 },
    { 0xAA, 0xAA, 0xAA },
    { 0xBB, 0xBB, 0xBB },
    { 0xCC, 0xCC, 0xCC },
    { 0xDD, 0xDD, 0xDD },
    { 0xEE, 0xEE, 0xEE },
    { 0xFF, 0xFF, 0xFF },
};

/*  01234567890123
**  ###  ####  ### 
**  #  # #    #
**  ###  ###  #
**  #  # #    #
**  #  # ####  ### 
*/

LOCAL char* gfx_rec_bmp[5] =
{
   "###  ####  ###",
   "#  # #    #   ",
   "###  ###  #   ",
   "#  # #    #   ",
   "#  # ####  ###"
};




/*
 * ============================================================================
 *  GFX_SDL_ABORT    -- Abort due to SDL errors.
 * ============================================================================
 */
LOCAL void gfx_sdl_abort(void)
{
    fprintf(stderr, "gfx/SDL Error:%s\n", SDL_GetError());
    exit(1);
}

extern SDL_Surface *back_surface;
extern SDL_Surface *KeyPad;
extern unsigned int   keypadloc;
extern unsigned int   keypadon;
extern unsigned int   volchange;
extern unsigned int   curvolume;

/*
 * ============================================================================
 *  GFX_INIT         -- Initializes a gfx_t object.
 * ============================================================================
 */
int 
gfx_init(gfx_t *gfx, int desire_x, int desire_y, int desire_bpp, int fs)
{
  int i;
  int actual_x=0, actual_y=0; /* JJT -- actual dimensions of the screen */
  periph_tick_t gfx_tick = NULL;

  /* -------------------------------------------------------------------- */
  /*  Sanity checks and cleanups.                                         */
  /* -------------------------------------------------------------------- */
  assert(gfx);
  memset((void*)gfx, 0, sizeof(gfx_t));

   gfx_tick = gfx_tick_320x200x16;
    /* -------------------------------------------------------------------- */
    /*  Allocate memory for the gfx_t.                                      */
    /* -------------------------------------------------------------------- */
    gfx->vid = calloc(160, 200);
    gfx->pvt = calloc(1, sizeof(gfx_pvt_t));

    if (!gfx->vid && !gfx->pvt)
    {
        fprintf(stderr, "gfx panic:  Could not allocate memory.\n");
        return -1;
    }

    {
        SDL_Rect **available_modes;
        
        available_modes = SDL_ListModes( NULL, 
                                         SDL_DOUBLEBUF|
                                         SDL_HWPALETTE|
                                         SDL_HWSURFACE|SDL_FULLSCREEN );

        if (available_modes == NULL)
            gfx_sdl_abort();
        else
        if (available_modes == (SDL_Rect **)-1)
        {
            actual_x = desire_x;
            actual_y = desire_y;
        }
        else
        {
            i = 0;
            while (available_modes[i] && 
                   available_modes[i]->w >= desire_x && 
                   available_modes[i]->h >= desire_y ) 
            {
                i++;
            }
            i--;
           
            if (i == -1)
                gfx_sdl_abort();

            actual_x = available_modes[i]->w;
            actual_y = available_modes[i]->h;
        }
    }

    gfx->pvt->scr = back_surface;

    gfx->pvt->ofs_x = (actual_x - desire_x) >> 1;
    gfx->pvt->ofs_y = (actual_y - desire_y) >> 1;

    if (!gfx->pvt->scr)
        gfx_sdl_abort();

    /* -------------------------------------------------------------------- */
    /*  Set up our color palette.  We start with video blanked.             */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < 16; i++)
    {
        gfx->pvt->pal_on [i].r = gfx_stic_palette[i][0];
        gfx->pvt->pal_on [i].g = gfx_stic_palette[i][1];
        gfx->pvt->pal_on [i].b = gfx_stic_palette[i][2];
        gfx->pvt->pal_off[i].r = gfx_stic_palette[i][0] >> 1;
        gfx->pvt->pal_off[i].g = gfx_stic_palette[i][1] >> 1;
        gfx->pvt->pal_off[i].b = gfx_stic_palette[i][2] >> 1;
    }
    for (i = 16; i < 32; i++)
    {
        gfx->pvt->pal_on [i].r = gfx_stic_palette[i][0];
        gfx->pvt->pal_on [i].g = gfx_stic_palette[i][1];
        gfx->pvt->pal_on [i].b = gfx_stic_palette[i][2];
        gfx->pvt->pal_off[i].r = gfx_stic_palette[i][0];
        gfx->pvt->pal_off[i].g = gfx_stic_palette[i][1];
        gfx->pvt->pal_off[i].b = gfx_stic_palette[i][2];
    }
    gfx->pvt->vid_enable = 0;
    SDL_SetColors(gfx->pvt->scr, gfx->pvt->pal_off, 0, 32);

  for (i = 0; i < 32; i++) {
    gfx_stic_palette_rgb[i] = gp2x_sdl_rgb(
        gfx_stic_palette[i][0], gfx_stic_palette[i][1], gfx_stic_palette[i][2] );
  }
    /* -------------------------------------------------------------------- */
    /*  Set up the gfx_t's internal structures.                             */
    /* -------------------------------------------------------------------- */
    gfx->periph.read        = NULL;
    gfx->periph.write       = NULL;
    gfx->periph.peek        = NULL;
    gfx->periph.poke        = NULL;
    gfx->periph.tick        = gfx_tick;
    gfx->periph.min_tick    = 14934;
    gfx->periph.max_tick    = 14934;
    gfx->periph.addr_base   = 0;
    gfx->periph.addr_mask   = 0;

    return 0;
}

/*
 * ============================================================================
 *  GFX_TICK_320X200 -- Services a gfx_t tick (in 320x200 mode)
 * ============================================================================
 */
uint_32 gfx_tick_320x200(periph_p gfx_periph, uint_32 len)
{
    gfx_t   *gfx = (gfx_t*) gfx_periph;
    uint_8  *vid, *scr, pix, *tmp;
    int x, y;

    gfx->tot_frames++;

    /* -------------------------------------------------------------------- */
    /*  Drop a frame if we need to.                                         */
    /* -------------------------------------------------------------------- */
    if (gfx->drop_frame)
    {
        gfx->drop_frame--;
        if (gfx->dirty) gfx->dropped_frames++;
        return len;
    }

    /* -------------------------------------------------------------------- */
    /*  Don't bother if display isn't dirty or if we're iconified.          */
    /* -------------------------------------------------------------------- */
    if (!gfx->dirty || gfx->hidden)
    {
        return len;
    }

    /* -------------------------------------------------------------------- */
    /*  DEBUG: Report blocks of dropped frames.                             */
    /* -------------------------------------------------------------------- */
    if (gfx->dropped_frames)
    {
#if 0
        printf("Dropped %d frames.\n", gfx->dropped_frames);
        fflush(stdout);
#endif
        gfx->tot_dropped_frames += gfx->dropped_frames;
        gfx->dropped_frames = 0;
    }

    /* -------------------------------------------------------------------- */
    /*  Draw the frame to the screen surface.                               */
    /* -------------------------------------------------------------------- */

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_LockSurface(gfx->pvt->scr);

    scr = gfx->pvt->ofs_x + gfx->pvt->scr->pitch * gfx->pvt->ofs_y +
          (uint_8 *) gfx->pvt->scr->pixels;
    tmp = scr;
    vid = (uint_8 *) gfx->vid;
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 160; x++)
        {
            pix     = *vid++;
            scr[0] = scr[1] = pix;
            scr   += 2;
        }
        scr += gfx->pvt->scr->pitch - 320;
    }

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_UnlockSurface(gfx->pvt->scr);

    /* -------------------------------------------------------------------- */
    /*  Actually update the display.                                        */
    /* -------------------------------------------------------------------- */
    //SDL_UpdateRect(gfx->pvt->scr, 0, 0, 0, 0);
    SDL_Flip(gfx->pvt->scr);
    gfx->dirty = 0;

    return len;
}

/*
 * ============================================================================
 *  GFX_SET_TITLE    -- Sets the window title 
 * ============================================================================
 */
int gfx_set_title(gfx_t *gfx, const char *title)
{
    (void)gfx;
    SDL_WM_SetCaption(title, title);
    return 0;
}

static void
loc_render_normal(gfx_t *gfx)
{
    uint_8  *vid, *scr, pix, *tmp;
    int x, y;

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_LockSurface(gfx->pvt->scr);

    scr = gfx->pvt->scr->pixels + 120 + (50 * 320);
    //scr = gfx->pvt->ofs_x + gfx->pvt->scr->pitch * gfx->pvt->ofs_y +
    //      (uint_8 *) gfx->pvt->scr->pixels;
    tmp = scr;
    vid = (uint_8 *) gfx->vid;
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 160; x++)
        {
            pix     = *vid++;
            scr[0] = scr[1] = gfx_stic_palette_rgb[pix];
            scr   += 2;
        }
        scr += gfx->pvt->scr->pitch - 320;
    }

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_UnlockSurface(gfx->pvt->scr);
}

static void
loc_render_x2(gfx_t *gfx)
{
  	uint_8  *vid;
  	uint_8   pix;
  	uint_16 *top_scr;
  	uint_16 *scr;
  	int x, y;
  	int color;

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_LockSurface(gfx->pvt->scr);

  	top_scr = gfx->pvt->scr->pixels;
  	vid = (uint_8 *) gfx->vid;

  	for (y = 0; y < 200; y++)
  	{
    		int y_d = y * 240 / 200;
    		scr = top_scr + (y_d * 320);

    		for (x = 0; x < 160; x++)
    		{
      		pix     = *vid++;
      		color   = gfx_stic_palette_rgb[pix & 0x3f];
      		scr[0] = scr[1] = scr[2] = color;
      		scr[0 + 320] = scr[1 + 320] = scr[2 + 320] = color;
      		scr   += 2;
    		}
  	}
    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_UnlockSurface(gfx->pvt->scr);
}

void drawSprite(SDL_Surface* imageSurface, SDL_Surface* screenSurface, int srcX, int srcY, int dstX, int dstY, int width, int height)
{
	SDL_Rect srcRect;
	srcRect.x = srcX;
	srcRect.y = srcY;
	srcRect.w = width;
	srcRect.h = height;

	SDL_Rect dstRect;
	dstRect.x = dstX;
	dstRect.y = dstY;
	dstRect.w = width;
	dstRect.h = height;

	if(!(SDL_BlitSurface(imageSurface, &srcRect, screenSurface, &dstRect)))
	{
         if(strlen(SDL_GetError())>1)
         fprintf(stderr,"Error: %s\n",SDL_GetError());
      }
}

static void
loc_render_21(gfx_t *gfx)
{
  	uint_8  *vid;
  	uint_8   pix;
  	uint_16 *top_scr;
  	uint_16 *scr;
  	int x, y;
  	int color;

    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_LockSurface(gfx->pvt->scr);

  	top_scr = gfx->pvt->scr->pixels;
  	vid = (uint_8 *) gfx->vid;

  	for (y = 0; y < 200; y++)
  	{
    		int y_d = y; //* 240 / 200;
    		scr = top_scr + (y_d * 320);

    		for (x = 0; x < 160; x++)
    		{
      		pix     = *vid++;
      		color   = gfx_stic_palette_rgb[pix & 0x3f];
      		scr[0] = scr[1] = scr[2] = color;
      		//scr[0 + 320] = scr[1 + 320] = scr[2 + 320] = color;
      		scr   += 2;
    		}
        	//scr += gfx->pvt->scr->pitch - 320;
  	}
    if (SDL_MUSTLOCK(gfx->pvt->scr))
        SDL_UnlockSurface(gfx->pvt->scr);
}

uint_32 
gfx_tick_320x200x16(periph_p gfx_periph, uint_32 len)
{
    gfx_t   *gfx = (gfx_t*) gfx_periph;
    uint_8  *vid;
    uint_8   pix;
    uint_16 *scr;
    int x, y;

    gfx->tot_frames++;

    /* -------------------------------------------------------------------- */
    /*  Drop a frame if we need to.                                         */
    /* -------------------------------------------------------------------- */
    if (gfx->drop_frame)
    {
        gfx->drop_frame--;
        if (gfx->dirty) gfx->dropped_frames++;
        return len;
    }

    /* -------------------------------------------------------------------- */
    /*  Don't bother if display isn't dirty or if we're iconified.          */
    /* -------------------------------------------------------------------- */
    if (!gfx->dirty || gfx->hidden)
    {
        return len;
    }

    /* -------------------------------------------------------------------- */
    /*  Draw the frame to the screen surface.                               */
    /* -------------------------------------------------------------------- */
  if (INTEL.gp2x_skip_cur_frame <= 0) {

    if (INTEL.intel_render_mode == INTEL_RENDER_X2 && keypadon==0) loc_render_x2(gfx);
    else
    if (INTEL.intel_render_mode == INTEL_RENDER_NORMAL && keypadon==0) loc_render_normal(gfx);
    else
    if (INTEL.intel_render_mode == INTEL_RENDER_21 || keypadon==1) loc_render_21(gfx);

	if(INTEL.intel_render_mode == INTEL_RENDER_21 || keypadon==1)
	{
		if(KeyPad) drawSprite(KeyPad, gfx->pvt->scr, 0, 0, 0, 240-KeyPad->h, KeyPad->w, KeyPad->h);

		if(keypadon)
		{		
			int kpx=85+(keypadloc*12);
			int kpy=101;	

			if(keypadloc>5)
			{
				kpy+=9;
				kpx=85+((keypadloc-6)*12);
			}
			gp2x_sdl_draw_rectangle(kpx,kpy ,21,17, gp2x_sdl_rgb(0,255,255), 0);
		}
	}

	#if 0
	if(volchange>0) {
		//if(SDL_GetTicks()-volchange>0) {
			int percvol = (curvolume/10)+10;
			int blocks;

			if(percvol<10)
				blocks=0;
			else if(percvol<20)
				blocks=1;
			else if(percvol<30)
				blocks=2;
			else if(percvol<40)
				blocks=3;
			else if(percvol<50)
				blocks=4;
			else if(percvol<60)
				blocks=5;
			else if(percvol<70)
				blocks=6;
			else if(percvol<80)
				blocks=7;
			else if(percvol<90)
				blocks=8;
			else	blocks=9;

			// draw the volume
			int i;
			
			printf("%i-%i\n",curvolume,blocks);
			gp2x_sdl_print( 4, 100, "VOLUME:", gp2x_sdl_rgb(255,255,255));
			for(i=0;i<blocks;i++)
			{
				int x;
				x=(20+(8*i))+2;
				gp2x_sdl_fill_rectangle(x,60,x+1,61,gp2x_sdl_rgb(255,255,255),0);
			}
		//}
		//else
		//	volchange=0;
	}
	#endif

    gp2x_sdl_flip();

    if (gp2x_screenshot_mode) {
      gp2x_screenshot_mode--;
      if (gp2x_screenshot_mode <= 0) {
        gp2x_sdl_save_screenshot();
        gp2x_screenshot_mode = 0;
      }
    }
  } else if (INTEL.gp2x_skip_max_frame) {
    INTEL.gp2x_skip_cur_frame--;
  }

  gfx->dirty = 0;

    return len;
}

/*
 * ============================================================================
 *  GFX_VID_ENABLE   -- Alert gfx that video has been enabled or blanked
 * ============================================================================
 */
void gfx_vid_enable(gfx_t *gfx, int enabled)
{
    /* -------------------------------------------------------------------- */
    /*  Force 'enabled' to be 0 or 1.                                       */
    /* -------------------------------------------------------------------- */
    enabled = enabled != 0;

    /* -------------------------------------------------------------------- */
    /*  Update the palette if there's been a change in blanking state.      */
    /* -------------------------------------------------------------------- */
    if (gfx->pvt->vid_enable ^ enabled)
    {
        SDL_SetColors(gfx->pvt->scr, 
                      enabled ? gfx->pvt->pal_on : gfx->pvt->pal_off, 0, 16);
    }

    gfx->pvt->vid_enable = enabled;
}

/*
 * ============================================================================
 *  GFX_SET_BORD     -- Set the border color for the display
 * ============================================================================
 */
void gfx_set_bord
(
    gfx_t *gfx,         /*  Graphics object.                        */
    int b_color
)
{
    int dirty = 0;

    /* -------------------------------------------------------------------- */
    /*  Set up the display parameters.                                      */
    /* -------------------------------------------------------------------- */
    if (gfx->b_color != b_color) { gfx->b_color = b_color; dirty = 3; }

    /* -------------------------------------------------------------------- */
    /*  If we're using the normal STIC blanking behavior, set our "off"     */
    /*  colors to the currently selected border color.  The alternate mode  */
    /*  (which is useful for debugging) sets the blanked colors to be       */
    /*  dimmed versions of the normal palette.                              */
    /* -------------------------------------------------------------------- */
    if (gfx->debug_blank == 0)
    {
        int i;

        for (i = 0; i < 16; i++)
            gfx->pvt->pal_off[i] = gfx->pvt->pal_on[b_color];
    }

    if (dirty)     { gfx->dirty   = 1; }
    if (dirty & 2) { gfx->b_dirty = 2; }
}

/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License       */
/*  along with this program; if not, write to the Free Software             */
/*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.               */
/* ======================================================================== */
/*          Copyright (c) 1998-2005, Joseph Zbiciak, John Tanner            */
/* ======================================================================== */
