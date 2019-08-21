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
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include "global.h"
#include "config.h"
#include "gp2x_sdl.h"
#include "gp2x_menu.h"

#include "gp2x.h"

extern SDL_Surface *back_surface;

#include "gp2x_kbd.h"
#include "gp2x_sdl.h"
#include "gp2x_fmgr.h"

# define GP2X_FMGR_MIN_TIME         7000

static struct dirent files[GP2X_FMGR_MAX_ENTRY];
static struct dirent *sortfiles[GP2X_FMGR_MAX_ENTRY];
static int nfiles;
static int user_file_format = 0;

void SJISCopy(struct dirent *a, char *file)
{
    unsigned char ca;
    int i;
    int len = strlen(a->d_name);

    for(i=0;i<=len;i++){
            ca = a->d_name[i];
            if (((0x81 <= ca)&&(ca <= 0x9f))
            || ((0xe0 <= ca)&&(ca <= 0xef))){
                  file[i++] = ca;
                  file[i] = a->d_name[i];
            }
            else{
                  if(ca>='a' && ca<='z') ca-=0x20;
                  file[i] = ca;
            }
    }
}

int cmpFile(struct dirent *a, struct dirent *b)
{
    	return 0;
}

void sort(struct dirent **a, int left, int right) 
{
	struct dirent *tmp, *pivot;
  	int i, p;

  	if (left < right) {
    		pivot = a[left];
    		p = left;
    		for (i=left+1; i<=right; i++) {
      			if (cmpFile(a[i],pivot)<0){
        			p=p+1;
        			tmp=a[p];
        			a[p]=a[i];
        			a[i]=tmp;
      			}
    		}
    		a[left] = a[p];
    		a[p] = pivot;
    		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}

int gp2x_fmgr_getExtId(const char *szFilePath) 
{
  char *pszExt;
  if((pszExt = strrchr(szFilePath, '.'))) {
    if ((!strcasecmp(pszExt, ".rom")) ||
        (!strcasecmp(pszExt, ".int")) ||
        (!strcasecmp(pszExt, ".itv")) ||
        (!strcasecmp(pszExt, ".bin"))) {
      return FMGR_FORMAT_ROM;
    } else if (!strcasecmp(pszExt, ".sta")) {
      return FMGR_FORMAT_STATE;
    } else if (!strcasecmp(pszExt, ".kbd")) {
      return FMGR_FORMAT_KBD;
    } else if (!strcasecmp(pszExt, ".zip")) {
      return FMGR_FORMAT_ZIP;
    }
  }
  return 0;
}

void getDir(const char *path) 
{
	DIR * fd = opendir(path);
	struct dirent * de;

	int b=0;
	int format=0;
  	nfiles = 0;

  	if(strcmp(path,"sd:/")){
    		strcpy(files[nfiles].d_name,"..");
    		sortfiles[nfiles] = files + nfiles;
    		nfiles++;
    		b=1;
  	}

  	while(nfiles<GP2X_FMGR_MAX_ENTRY){
    		memset(&files[nfiles], 0x00, sizeof(struct dirent));

		if((de = readdir(fd)) == NULL) break;
		memcpy(&files[nfiles],de,sizeof(struct dirent));
	
    		//if(files[nfiles].d_name[0] == '.') continue;

		if(de->d_type == DT_DIR)
		{
      		strcat(files[nfiles].d_name, "/");
      		sortfiles[nfiles] = files + nfiles;
      		nfiles++;
    		}

    		sortfiles[nfiles] = files + nfiles;
    		format = gp2x_fmgr_getExtId(files[nfiles].d_name);
    		if ((format == user_file_format) || ((format == FMGR_FORMAT_ZIP) && (user_file_format != FMGR_FORMAT_KBD))) 
    		{
      		nfiles++;
		}

	}
	closedir(fd);
  
	if(b)
    		sort(sortfiles+1, 0, nfiles-2);
  	else
    		sort(sortfiles, 0, nfiles-1);
	
}

int gp2x_fmgr_get_dir_list(char *basedir, int dirmax, char **dirname)
{
	DIR * fd = opendir(basedir);
	struct dirent * de;
  	int index = 0;
  	nfiles = 0;

	fprintf(stdout,"Start DIR search\n");
  	while ((nfiles<GP2X_FMGR_MAX_ENTRY) && (nfiles < dirmax)) {
    		memset(&files[nfiles], 0x00, sizeof(struct dirent));
    		if((de=readdir(fd))==NULL) break;
    		memcpy(&files[nfiles],de,sizeof(struct dirent));
    		if(files[nfiles].d_name[0] == '.') continue;

		if(de->d_type == DT_DIR)
		{
      			strcat(files[nfiles].d_name, "/");
      			sortfiles[nfiles] = files + nfiles;
      			nfiles++;
      			continue;
    		}
  	}
	fprintf(stdout,"End DIR search\n");

	closedir(fd);
  	sort(sortfiles, 0, nfiles-1);
  	for (index = 0; index < nfiles; index++) {
    		dirname[index] = strdup(sortfiles[index]->d_name);
  	} 
	fprintf(stdout,"Found %i directories\n",nfiles);
  	return nfiles;
}

void gp2x_display_screen_fmgr(void)
{
    char buffer[32];

    # if 0 
            gp2x_sdl_clear_screen( GP2X_MENU_BACKGROUND_COLOR );
    # else
            gp2x_sdl_blit_background();
    # endif

    gp2x_sdl_fill_rectangle(0,0,318,237,GP2X_MENU_BACKGROUND_COLOR,0);
    gp2x_sdl_draw_rectangle(0,0,318,237,GP2X_MENU_BORDER_COLOR,0);
    gp2x_sdl_draw_rectangle(1,1,314,233,GP2X_MENU_BORDER_COLOR,0);

    //gp2x_sdl_fill_print( 2, 2, " Start+L+R: EXIT ", 
    //                 GP2X_MENU_WARNING_COLOR,  GP2X_MENU_BACKGROUND_COLOR);

    //gp2x_sdl_fill_print( 2, 40, " []: Cancel  B/X: Valid  Triangle: Up ", 
    //                 GP2X_MENU_BORDER_COLOR, GP2X_MENU_BACKGROUND_COLOR);
    //gp2x_sdl_fill_print(200, 2, " By Nemesis/112 ",
    //                 GP2X_MENU_AUTHOR_COLOR, GP2X_MENU_BACKGROUND_COLOR);
}

int gp2x_file_request(char *out, char *pszStartPath)
{
    static  int sel=0;

    unsigned long pad;
    int   display_screen;
    int  last_time;

    long color;
    int top, rows=18, x, y, i, up=0;
    char path[GP2X_FMGR_MAX_PATH];
    char oldDir[GP2X_FMGR_MAX_NAME];
    char *p;
    int  file_selected;

    memset(files, 0x00, sizeof(struct dirent) * GP2X_FMGR_MAX_ENTRY);
    memset(sortfiles, 0x00, sizeof(struct dirent *) * GP2X_FMGR_MAX_ENTRY);
    nfiles = 0;

    strcpy(path, pszStartPath);
    getDir(path);

    display_screen = 1;
    last_time = 0;
    top = 0;
    file_selected = 0;

    for(;;) 
    {
            if (display_screen) {
                 gp2x_display_screen_fmgr();
                 display_screen = 1;
            }

            x = 8; y = 8;

            for(i=0; i<rows; i++){
                     if(top+i >= nfiles) break;
                     if(top+i == sel) color = GP2X_MENU_SEL_COLOR;
                     else             color = GP2X_MENU_TEXT_COLOR;
                 
                     gp2x_sdl_fill_rectangle(x, y, 200, 10, GP2X_MENU_BACKGROUND_COLOR,0);
                     gp2x_sdl_fill_print(x, y, sortfiles[top+i]->d_name, color, GP2X_MENU_BACKGROUND_COLOR);
                     y += 6;
            }

            gp2x_sdl_flip();

		    pad=gp2x_joystick_read();

    		if ((pad & (GP2X_L|GP2X_R|GP2X_START))) {
      		   /* Exit ! */
      		   gp2x_sdl_exit(0);
    		}
            if ((SDL_GetTicks() - last_time) > GP2X_FMGR_MIN_TIME) {
                   last_time = SDL_GetTicks();
            }

            if ((pad&GP2X_X) || (pad&GP2X_B)) {
                   if ( sortfiles[sel]->d_type == DT_DIR ) {
                           if(!strcmp(sortfiles[sel]->d_name,"..")){
                                  up=1;
                           } else {
                                  strcat(path,sortfiles[sel]->d_name);
                                  display_screen = 1;
                                  getDir(path);
                                  sel=0;
					    up=0;
					    pad=0;
    					    gp2x_kbd_wait_no_button();
                           }
                   }else{
                           strcpy(out, path);
                           strcat(out, sortfiles[sel]->d_name);
                           strcpy(pszStartPath,path);
                           file_selected = 1;
                           break;
                   }
            } else if(pad&GP2X_Y){
                   up=1;
            } else if((pad&GP2X_A) || (pad&GP2X_SELECT)) {
                   /* Cancel */
                   file_selected = 0;
                   break;
            } else if(pad&GP2X_UP){
                   sel--;
            } else if(pad&GP2X_DOWN){
                   sel++;
            } else if(pad&GP2X_L){
                   sel-=10;
            } else if(pad&GP2X_R){
                   sel+=10;
            }

            if(up) {
                   display_screen = 1;
                   if(strcmp(path,"sd:/")){
                           p=strrchr(path,'/');
                           *p=0;
                           p=strrchr(path,'/');
                           p++;
                           strcpy(oldDir,p);
                           strcat(oldDir,"/");
                           *p=0;
                           getDir(path);
                           sel=0;
                           for(i=0; i<nfiles; i++) {
                                   if(!strcmp(oldDir, sortfiles[i]->d_name)) {
                                         sel=i;
                                         top=sel-3;
                                         break;
                                   }
                           }
                   }
                   up=0;
            }

            if(top > nfiles-rows) top=nfiles-rows;
            if(top < 0)           top=0;
            if(sel >= nfiles)     sel=nfiles-1;
            if(sel < 0)           sel=0;
            if(sel >= top+rows)   top=sel-rows+1;
            if(sel < top)         top=sel;
	}

    return file_selected;
}

int gp2x_fmgr_menu(int format)
{
    static char user_filedir[GP2X_FMGR_MAX_NAME];
    static int  first = 1;

    char user_filename[GP2X_FMGR_MAX_NAME];
    struct stat       aStat;
    int               file_format;
    int               error;
    int               ret;

    user_file_format = format;
    ret = 0;

    if (first) {
            first = 0;
            getcwd(user_filedir, GP2X_FMGR_MAX_NAME);
            strcat(user_filedir, "/");
    }

    gp2x_kbd_wait_no_button();

    if (gp2x_file_request(user_filename, user_filedir)) {
            error = 0;
            if (stat(user_filename, &aStat)) error = 1;
            else 
            {
                     file_format = gp2x_fmgr_getExtId(user_filename);

                     if (file_format == FMGR_FORMAT_ZIP) {
                               if (user_file_format == FMGR_FORMAT_ROM) /* Rom */ error = intel_load_rom(user_filename, 1);
                     }
                     else 
                     {
                               if (file_format == FMGR_FORMAT_ROM) /* Rom */ error = intel_load_rom(user_filename, 0);
                               else
                               if (file_format == FMGR_FORMAT_KBD) /* Keyboard */ error = intel_kbd_load(user_filename);
                     }
            }

            if (error) ret = -1;
            else       ret =  1;
    }

    gp2x_kbd_wait_no_button();

    return ret;
}
