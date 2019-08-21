/*
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

# ifndef _GP2X_SDL_H_
# define _GP2X_SDL_H_

# define gp2x_debug(m)   loc_gp2x_debug(__FILE__,__LINE__,m)

# define GP2X_SDL_NOP   0
# define GP2X_SDL_XOR   1

# define GP2X_LINE_SIZE  320

# define GP2X_SDL_SCREEN_WIDTH    320
# define GP2X_SDL_SCREEN_HEIGHT   240

  typedef unsigned char   uchar;
  typedef unsigned int    uint;
  typedef unsigned short  ushort;

# if 0
# define gp2x_sdl_rgb(r, g, b)  (((b & 0xf8) << 7) | ((g & 0xf8) << 2) | (r >> 3))
# endif

  Uint32 gp2x_sdl_rgb(Uint8 R, Uint8 G, Uint8 B);
  extern int gp2x_load_fonts(void);
  extern int gp2x_print_text(char * str, int colour, int v, int h);

  extern void loc_gp2x_debug(char *file, int line, char *message);

  /* PG -> SDL function */

# if 0
  extern unsigned int gp2x_sdl_rgb(uchar R, uchar G, uchar B);
# endif
  extern void gp2x_sdl_print(int x,int y, char *str, int color);
  extern void gp2x_sdl_clear_screen(int color);
  extern void gp2x_sdl_fill_rectangle(int x, int y, int w, int h, int color, int mode);
  extern void gp2x_sdl_draw_rectangle(int x, int y, int w, int h, int border, int mode);
  extern void gp2x_sdl_put_char(int x, int y, int color, int bgcolor, char c, int drawfg, int drawbg);
  extern void gp2x_sdl_fill_print(int x,int y,const char *str, int color, int bgcolor);
  extern void gp2x_sdl_flip(void);

  extern void gp2x_sdl_lock(void);
  extern void gp2x_sdl_unlock(void);

  extern void gp2x_sdl_flush(void);
  extern void gp2x_sdl_save_bmp(char *filename);
  extern void gp2x_sdl_blit_background();
  extern void gp2x_sdl_exit(int status);

# endif
