

CROSS_COMPILE = C:/devkitGP2X/bin/arm-linux-
SDL_BASE = C:/devkitGP2X/bin/arm-linux-
LDFLAGS = -static

CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
STRIP = $(CROSS_COMPILE)strip

CFLAGS = -I. -I.. `$(SDL_BASE)sdl-config --cflags` -O2 -Wall
CXXFLAGS = -I. -I.. `$(SDL_BASE)sdl-config --cflags` -O2 -Wall
LIBS = `$(SDL_BASE)sdl-config --libs` -lSDL_mixer -lsmpeg -lvorbisfile -lpthread -lvorbis -logg -lmikmod -lSDL -lpthread 
ASFLAGS = -mno-fpu

SDLTEST_TARGET = intvX2.gpe
SDLTEST_OBJS = \
global.o \
gp2x_sound.o \
gp2x_fmgr.o \
gp2x_menu.o \
gp2x_menu_kbd.o \
gp2x_virtualpad.o \
gp2x_kbd.o \
gp2x_sdl.o \
gp2x_joystick.o \
gp2x_about.o \
jzintv.o cfg/cfg.o cfg/usage.o misc/crc32.o misc/crc16.o misc/avl.o misc/ll.o \
plat/plat_lib.o plat/gnu_getopt.o event/event.o \
event/event_tbl.o gfx/gfx.o snd/snd.o mvi/mvi.o debug/debug.o \
debug/debug_dasm1600.o periph/periph.o cp1600/cp1600.o cp1600/op_decode.o \
cp1600/op_exec.o cp1600/tbl/fn_cond_br.o cp1600/tbl/fn_dir_2op.o \
cp1600/tbl/fn_imm_2op.o cp1600/tbl/fn_impl_1op_a.o cp1600/tbl/fn_impl_1op_b.o \
cp1600/tbl/fn_ind_2op.o cp1600/tbl/fn_reg_1op.o cp1600/tbl/fn_reg_2op.o \
cp1600/tbl/fn_rot_1op.o cp1600/tbl/formats.o cp1600/emu_link.o mem/mem.o \
icart/icart.o icart/icartrom.o icart/icartbin.o icart/icartfile.o stic/stic.o \
pads/pads.o pads/pads_cgc.o pads/pads_cgc_linux.o pads/pads_cgc_win32.o \
pads/pads_intv2pc.o ay8910/ay8910.o ivoice/ivoice.o speed/speed.o file/file.o \
bincfg/bincfg.o bincfg/bincfg_grmr.tab.o bincfg/bincfg_lex.o  bincfg/legacy.o \
gif/gif_enc.o gif/lzw_enc.o demo/demo.o  joy/joy.o serializer/serializer.o

all : $(SDLTEST_TARGET)

$(SDLTEST_TARGET) : $(SDLTEST_OBJS)
	$(CXX) $(LDFLAGS) -o $(SDLTEST_TARGET) $(SDLTEST_OBJS) $(LIBS)
	$(STRIP) $(SDLTEST_TARGET)

clean:
	rm -f $(ALL_TARGETS) *.o *~