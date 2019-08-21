/*
 * ============================================================================
 *  Title:    MAIN
 *  Author:   J. Zbiciak
 *  $Id: jzintv.c,v 1.41 2001/11/02 02:00:02 im14u2c Exp $
 * ============================================================================
 *  Main Simulator Driver File
 * ============================================================================
 *  This doesn't do much yet.  :-)
 * ============================================================================
 */

#include "config.h"
#include "global.h"

#ifndef macintosh
#include <SDL/SDL.h>            /* Ack!  Needed for hacks to main()     */
#endif

#include <signal.h>
#include "macros.h"
#include "plat/plat.h"
#include "periph/periph.h"
#include "cp1600/cp1600.h"
#include "mem/mem.h"
#include "icart/icart.h"
#include "bincfg/bincfg.h"
#include "bincfg/legacy.h"
#include "pads/pads.h"
#include "pads/pads_cgc.h"
#include "pads/pads_intv2pc.h"
#include "gfx/gfx.h"
#include "snd/snd.h"
#include "ay8910/ay8910.h"
#include "demo/demo.h"
#include "stic/stic.h"
#include "speed/speed.h"
#include "debug/debug_.h"
#include "event/event.h"
#include "file/file.h"
#include "ivoice/ivoice.h"
#include "cfg/cfg.h"

#ifdef macintosh
# include "console.h"

#endif

double elapsed(int);

/*volatile int please_die = 0;*/
/*volatile int reset = 0;*/
static cfg_t *intv;

static uint_32 checksum(uint_16 *mem, int len) UNUSED;
/*static void usage(void);*/
void dump_state(void);
static void graceful_death(int);

void Terminate()
{
	graceful_death(2);
	printf("Terminate\n");
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", NULL);
	printf("Terminate_end\n");
}

/*
 * ============================================================================
 *  RELEASE      -- Program name / release
 * ============================================================================
 */
static char * release(void)
{
    char *s2;
    const char *Name = "$Name:  $", *s1;
    static char name_buf[64];
    static int named = 0;

    if (named) return name_buf;

    s1 = Name;

    while (*s1 && *s1 != ':') s1++;
    if (*s1 == ':') s1++;
    while (*s1 && isspace(*s1) && *s1 != '$') s1++;

    if (!*s1 || *s1 == '$')
    {
        strncpy(name_buf, "[unreleased]", sizeof(name_buf)-1);
        name_buf[sizeof(name_buf)-1] = 0;
    } else
    {
        s2 = name_buf;
        while (s2 < (name_buf + sizeof(name_buf) - 1) && *s1 && *s1!='$')
            *s2++ = *s1++;
        *s2++ = 0;
    }
    
    named = 1;

    return name_buf;
}

#if 1
/*
 * ============================================================================
 *  CART_NAME    -- Look for a game name in a cartridge image.
 * ============================================================================
 */
static char * cart_name(void)
{
    static char name_buf[64];
    uint_16 title_addr, lo, hi, ch;
    int year;
    int i;
    char *base_name;

    if ((base_name = strrchr(intv->fn_game, '/')) == NULL &&
        (base_name = strrchr(intv->fn_game, '\\')) == NULL)
        base_name = intv->fn_game;
    else
        base_name++;


    lo = periph_peek(&intv->intv->periph, &intv->intv->periph, 0x500A, ~0);
    hi = periph_peek(&intv->intv->periph, &intv->intv->periph, 0x500B, ~0);

    if ((lo | hi) & 0xFF00)
        return base_name;

    title_addr = ((hi << 8) | lo);

    year = 1900 + periph_peek(&intv->intv->periph, 
                              &intv->intv->periph, title_addr, ~0);

    for (i = 0; i < 64 - 8; i++)
    {
        ch = periph_peek(&intv->intv->periph, 
                         &intv->intv->periph, title_addr + i + 1, ~0);

        name_buf[i] = ch;

        if (ch == 0)
            break;

        if (ch < 32 || ch > 126)
            return base_name;
    }

    sprintf(name_buf + i, " (%4d)", year);

    return name_buf;
}
#endif

/*
 * ============================================================================
 *  GRACEFUL_DEATH -- Die gracefully.
 * ============================================================================
 */
static void graceful_death(int x)
{
    fprintf(stdout,"graceful_death\n");

    if (intv->do_exit < 2)
    {
        if (intv->do_exit) fprintf(stderr, "\nOUCH!\n");
        fprintf(stderr, 
                "\nRequesting exit:  Received signal %d.\n"
                "(You may need to press enter if you're at a prompt.)\n", x);
    } else
    {
        fprintf(stderr, "\nReceived 3 signals:  Aborting on signal %d.\n", x);
	  fflush(stderr);
        exit(1);
    }
    intv->do_exit++;

    fflush(stdout);
    fflush(stderr);
}

/*
 * ============================================================================
 *  ELAPSED      -- Returns amount of time that's elapsed since the program
 *                  started, in CP1610 clock cycles (895kHz)
 * ============================================================================
 */
double elapsed(int restart)
{
    static double start;
    static int init = 0;
    double now;

    if (!init || restart) 
    {
        start = get_time();
        init = 1;
    }

    now = get_time();

    return (now - start) * 894886.25;
}

/*
 * ============================================================================
 *  In the beginning, there was a main....
 * ============================================================================
 */
int 
main(int argc, char *argv[]) 
{ 
	int iter = 0, arg;
	double cycles = 0, rate, irate, then, now, icyc;
	uint_32 s_cnt = 0;
	int paused = 0;
	char title[128];

	intel_global_init();
	atexit(Terminate);
	gp2x_joystick_init();
	gp2x_sdl_init();

  /* -------------------------------------------------------------------- */
  /*  Go get an Intellivision!                                            */
  /* -------------------------------------------------------------------- */
	intv = cfg_init(argc, argv);
  	gp2x_set_cpu_speed(INTEL.gp2x_cpu_clock);
	//gp2x_sound_volume(INTEL.volume,INTEL.volume);
	gp2x_main_menu(0);
	intv->do_exit=0;
	gp2x_sdl_clear_screen(0);
	gp2x_sdl_flip();	
	gp2x_sdl_clear_screen(0);
	gp2x_sdl_flip();
    /* -------------------------------------------------------------------- */
    /*  Set the window title.  If we recognize a standard Intellivision     */
    /*  ROM header at 0x5000, then also include the cartridge name.         */
    /* -------------------------------------------------------------------- */

    /* -------------------------------------------------------------------- */
    /*  Run the simulator.                                                  */
    /* -------------------------------------------------------------------- */
	if (intv->debugging) {
 		debug_tk((periph_p)&(intv->debug),1);
	}

    	fprintf(stdout,"Starting jzIntv...\n");
    	fflush(stdout);
    	sleep(1);
restart:

    fprintf(stdout,"restart\n");
    fflush(stdout);

    iter = 1;
    now = elapsed(1);
    while (now == elapsed(0))    /* Burn some time. */
        ;

    icyc   = 0;
    s_cnt  = 0;
    cycles = 0;
    if (intv->rate_ctl) {
        speed_resync(&(intv->speed));
    }

    if (!intv->debugging) {
        intv->debug.step_count = ~0U;
    }

    paused = 0;

    while (intv->do_exit == 0)
    {
        uint_64 max_step;
        int do_reset = intv->do_reset;

        if (do_reset)
        {
            max_step = 1000; /* arbitrary */
        } else
        {
            if (s_cnt)
            {
                s_cnt = 0;
                periph_reset(intv->intv);
            }
            /* This is incredibly hackish, and is an outgrowth of my
             * decoupled tick architecture.  */
            max_step = intv->stic.next_phase - intv->stic.stic_cr.now;
#if 1
            if (intv->cp1600.periph.now > intv->stic.stic_cr.now)
            {
                uint_64 diff;
                diff = intv->cp1600.periph.now - intv->stic.stic_cr.now;
                if (diff < max_step)
                    max_step -= diff;
            } else if (intv->stic.stic_cr.now > intv->cp1600.periph.now)
            {
                uint_64 diff;
                diff = intv->stic.stic_cr.now - intv->cp1600.periph.now;
                if (diff < max_step)
                    max_step -= diff;
            } 
#endif
            if (max_step < 5) max_step = 5;
        }

        if (intv->do_pause) 
        { 
            paused = !paused; intv->do_pause = 0; 
            if (!paused)
                speed_resync(&(intv->speed));
        }
 
        if (intv->event.change_kbd)
        {
            if (intv->event.change_kbd == 5)
            {
                intv->event.cur_kbd = (intv->event.cur_kbd + 1) & 3;
            } else
            if (intv->event.change_kbd == 6)
            {
                intv->event.cur_kbd = (intv->event.cur_kbd - 1) & 3;
            } else
            {
                intv->event.cur_kbd = (intv->event.change_kbd - 1) & 3;
            }

            intv->event.change_kbd = 0;
            memset(intv->pad0.l, 0, sizeof(intv->pad0.l));
            memset(intv->pad0.r, 0, sizeof(intv->pad0.r));
            memset(intv->pad0.k, 0, sizeof(intv->pad0.k));
            memset(intv->pad1.l, 0, sizeof(intv->pad1.l));
            memset(intv->pad1.r, 0, sizeof(intv->pad1.r));
            memset(intv->pad1.k, 0, sizeof(intv->pad1.k));
        }
        if (paused)
        {
            intv->event.periph.tick((periph_p)&(intv->event), 0);
            plat_delay(1000/60);
        } else if (do_reset)
        {
            intv->gfx.dirty = 1;
            intv->gfx.periph.tick  ((periph_p)&(intv->gfx),   STIC_FRAMCLKS);
            intv->event.periph.tick((periph_p)&(intv->event), STIC_FRAMCLKS);
            plat_delay(1000/60);
            cycles += STIC_FRAMCLKS;
        } else
            cycles += periph_tick((periph_p)(intv->intv), max_step);

        if (!intv->debugging && intv->debug.step_count == 0)
            intv->debug.step_count = ~0U;

        if (!intv->debugging && !do_reset && (iter++&1023) == 0)
        {

            then  = now;
            now   = elapsed(0);
            rate  = (cycles / now);
            if (now - then > 0.01)
            {
                irate = (cycles - icyc) / (now - then);
                icyc  = cycles;
            }

            if ((iter&2047)==0)
            {
                elapsed(1);
                cycles = icyc = 0;
            }
        }

        if (do_reset)
        {
            intv->cp1600.r[7] = 0x1000;
            gfx_vid_enable(&(intv->gfx), 0);
            s_cnt++;
        } else 
        {
            if (s_cnt > 140) break;
            s_cnt = 0;
        }
    }

    s_cnt = 0x2A3A4A5A;

    arg = 0;
    gfx_set_bord  (&(intv->gfx), 0);
    gfx_vid_enable(&(intv->gfx), 1);
    intv->gfx.scrshot |= GFX_RESET;

    while (intv->do_exit == 0)
    {
        int i, j;
        uint_8 p;

	  fprintf(stdout,"Second Loop\n");
	  fflush(stdout);

        if (intv->do_reset) arg = 1;
        if (!intv->do_reset && arg) 
        {
            p = intv->stic.raw[0x2C] & 15;
            for (i = 0; i < 160 * 200; i++)
                intv->stic.disp[i] = p;

            gfx_set_bord  (&(intv->gfx), p);
            intv->gfx.scrshot &= ~GFX_RESET;
            goto restart;
        }
        
        for (i = 0; i < 160 * 200; i++)
        {
            for (j = 0; j < 16; j++)
                s_cnt = (s_cnt << 1) | (1 & ((s_cnt >> 28) ^ (s_cnt >> 30)));


            p = (s_cnt & 0xF) + 16;
            if (p == 16) p = 0;

            intv->stic.disp[i] = p;
        }

        intv->gfx.dirty = 1;
        intv->gfx.periph.tick  ((periph_p)&(intv->gfx),   0);
	  fprintf(stderr,"periph_tick\n");
        intv->event.periph.tick((periph_p)&(intv->event), 0);
        plat_delay(1000/60);
    }

    if (intv->do_exit)
        fprintf(stdout,"\nExited on user request.\n");

    fflush(stdout);
    return 0;
}


#include "cp1600/op_decode.h"
#include "cp1600/op_exec.h"

/*
 * ============================================================================
 *  DUMP_STATE   -- Called by op_exec:fn_HLT.  Dumps state of universe.
 * ============================================================================
 */

void dump_state(void)
{
# if 0 //LUDO:
    FILE *f;
    int addr, data, i, j;

    f = fopen("dump.mem","wb");
    if (!f) 
    { 
        perror("fopen(\"dump.mem\", \"w\")");
        printf("couldn't open dump.mem, not dumping memory.\n"); 
    }
    else
    {
        intv->debug.show_rd = 0;
        intv->debug.show_wr = 0;
        for (addr = 0; addr <= 0xFFFF; addr++)
        {   
            data = periph_peek((periph_p)intv->intv, (periph_p)intv->intv, 
                               addr, 0);
            fputc((data >> 8) & 0xFF, f);
            fputc((data     ) & 0xFF, f);
        }

        fclose(f);
    }

    f = fopen("dump.cpu", "wb");
    if (!f) 
    { 
        perror("fopen(\"dump.cpu\", \"w\")");
        printf("couldn't open dump.cpu, not dumping cpu info.\n"); return; 
    }

    fprintf(f, "CP-1600 State Dump\n");
    fprintf(f, "Tot Cycles:   %lld\n", intv->cp1600.tot_cycle);
    fprintf(f, "Tot Instrs:   %lld\n", intv->cp1600.tot_instr);
    fprintf(f, "Tot Cache:    %d\n", intv->cp1600.tot_cache);
    fprintf(f, "Tot NonCache: %d\n", intv->cp1600.tot_noncache);
    fprintf(f, "Registers:    %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x\n",
            intv->cp1600.r[0], intv->cp1600.r[1], 
            intv->cp1600.r[2], intv->cp1600.r[3],
            intv->cp1600.r[4], intv->cp1600.r[5], 
            intv->cp1600.r[6], intv->cp1600.r[7]);
    fprintf(f, "Flags:        S:%d C:%d O:%d Z:%d I:%d D:%d intr:%d irq:%d\n",
            intv->cp1600.S, intv->cp1600.C, intv->cp1600.O, intv->cp1600.Z, 
            intv->cp1600.I, intv->cp1600.D,
            intv->cp1600.intr, intv->cp1600.req_bus.intrq);

    fprintf(f, "Cacheability Map:\n");

    for (i = 0; i < 1 << (CP1600_MEMSIZE-CP1600_DECODE_PAGE - 5); i++)
    {
        addr = (i << (CP1600_DECODE_PAGE + 5));

        fprintf(f, "   %.4x-%.4x:", addr, addr+(32<<CP1600_DECODE_PAGE)-1);
        for (j = 0; j < 32; j++)
        {
            fprintf(f, " %d", 1 & (intv->cp1600.cacheable[i] >> j));
        }
        fprintf(f, "\n");
    }


    fprintf(f,"Decoded Instruction Map:\n");

    for (i = 0; i < 1 << (CP1600_MEMSIZE - 6); i++)
    {
        addr = i << 6;

        fprintf(f, "   %.4x-%.4x:", addr, addr + 63);
        for (j = 0; j < 64; j++)
        {
            fprintf(f, "%c", 
                (void*)intv->cp1600.execute[addr+j]==(void*)fn_decode_1st?'-' :
                (void*)intv->cp1600.execute[addr+j]==(void*)fn_decode    ?'N' :
                (void*)intv->cp1600.execute[addr+j]==(void*)fn_invalid   ?'!' :
                                                                          'C');
        }
        fprintf(f, "\n");
    }

    fclose(f);
# endif
}


/*
 * ============================================================================
 *  READ_ROM     -- Reads a ROM image file, adjusting it for endian if 
 *                  necessary
 * ============================================================================
 */

int read_rom(char * filename, int size, uint_16 * image)
{
    FILE *f;
    int tot = 0, block, addr;

#ifdef macintosh
# pragma unused( addr )
#endif

    memset(image, 0xFF, size * 2);

    f = fopen(filename, "rb");

    if (!f)
    {
        fprintf(stderr, "Could not open '%s' for reading.\n", filename);
        perror("fopen()"); exit(1);
    }

    while (tot < size)
    {
        block = size - tot > 128 ? 128 : size - tot;
        block = fread(image + tot, 2, block, f);
        tot += block;

        if (block < 0)
        {
            fprintf(stderr, "Error while reading '%s'\n", filename);
            perror("fread()");  exit(1);
        }
        if (block == 0)
            break;
    }
    fclose(f);

#ifdef _LITTLE_ENDIAN
    for (addr = 0; addr < size; addr++)
    {
        image[addr] = (0x00FF & (image[addr] >> 8)) | 
                      (0xFF00 & (image[addr] << 8));
    }
#endif

    printf("Read %d words from '%s'.\n", tot, filename);
    return tot;
}

/*
 * ============================================================================
 *  CHECKSUM     -- Returns a simple checksum on a region of memory.
 *                  I was using this to try to find some bugs, assuming
 *                  memory corruption was happening.
 * ============================================================================
 */

static uint_32 checksum(uint_16 *mem, int len)
{
    uint_32 cksum = 0x2A3A4A5A;

    while (len-->0)
    {
        cksum ^= *mem;
        cksum  = (cksum >> 1) | (cksum << 31);
        cksum += *mem++;
    }

    return cksum;
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
/*                 Copyright (c) 1998-1999, Joseph Zbiciak                  */
/* ======================================================================== */
