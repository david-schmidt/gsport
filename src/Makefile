# GSport central makefile - you need a 'vars' file linked/copied from a 'vars_xxx' template to build.
OBJECTS1 = adb.o clock.o config.o debug.o dis.o engine_c.o scc.o iwm.o \
	joystick_driver.o moremem.o paddles.o parallel.o printer.o sim65816.o \
	smartport.o sound.o sound_driver.o video.o scc_socket_driver.o glog.o \
	imagewriter.o scc_imagewriter.o scc_llap.o options.o
ATOBJ = atbridge/aarp.o atbridge/atbridge.o atbridge/elap.o atbridge/llap.o atbridge/port.o
PCAPOBJ = atbridge/pcap_delay.o
TFEOBJ = tfe/tfe.o tfe/tfearch.o tfe/tfesupp.o
FSTOBJ = unix_host_common.o host_common.o host_fst.o host_mli.o

include vars

.SUFFIXES: .dep .proto

AS = $(CC)

XLIBS = -L/usr/X11R6/lib
PERL = perl

all: $(TARGET)

clean:
	- rm -f $(OBJECTS)
	- rm -f $(TARGET)
	- rm -f compile_time.o
	- rm -f 8inst_c.h
	- rm -f 16inst_c.h

specials: 8inst_s 16inst_s 8size 16size 8inst_c 16inst_c size_c size_s

specials_clean:
	rm -f 8inst_s 16inst_s 8size 16size 8inst_c 16inst_c size_c size_s

# Linux/OSX/RPi SDL builds
gsport: $(OBJECTS) compile_time.o
	$(LD) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS)
	echo $(OBJECTS)
	cp gsport ..

# Linux/OSX XWindows builds
gsportx: $(OBJECTS) compile_time.o
	$(LD) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(XLIBS) $(EXTRA_LIBS) -lX11
	echo $(OBJECTS)
	mv gsportx ..

# NOT CURRENTLY SUPPORTED
# Linux framebuffer builds:
gsportfb: $(OBJECTS) compile_time.o
	$(LD) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS)
	echo $(OBJECTS)
	mv gsportfb ..

# Mingw32 / Cygwin builds:  The Win32 API version
gsport32.exe: $(OBJECTS) compile_time.o
	g++ $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS) -lwinmm -lgdi32 -ldsound -lcomctl32 -lws2_32 -lshell32
	#mkdir -p ../GSport.app/lib
	#cp -f gsport.exe ../GSport.app/GSport.exe
	#cp -f ../config.template ../GSport.app/config.txt
	#cp -f ../lib/*.ttf ../GSport.app/lib
	#cp -f ../lib/arch/win32/*.dll ../GSport.app
	#cp -f ../lib/NoBoot.po ../GSport.app
	#cp -f GSport.bat ../GSport.app/GSport.bat
	#cp -f parallel.rom ../GSport.app
	#cp -f ../COPYING.txt ../GSport.app
	cp gsport32.exe ..

# Mingw32 / Cygwin builds:  The SDL version (builds, but non-functioning)
gsport.exe: $(OBJECTS) compile_time.o
	#g++ $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS) -lwinmm -lgdi32 -ldsound -lcomctl32 -lws2_32 -lshell32
	#g++ $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS)  -mwindows
	g++ $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS) -lwinmm -lgdi32 -ldsound -lcomctl32 -lws2_32 -lshell32
	cp gsport.exe ..

8inst_c.h: instable.h
	$(PERL) make_inst c 8 instable.h > 8inst_c.h

16inst_c.h: instable.h
	$(PERL) make_inst c 16 instable.h > 16inst_c.h

size_c.h: size_tab.h
	$(PERL) make_size c size_tab.h > size_c.h

engine_c.o: 8inst_c.h 16inst_c.h size_c.h

8inst_s.h: instable.h
	$(PERL) make_inst s 8 instable.h > 8inst_s.h

16inst_s.h: instable.h
	$(PERL) make_inst s 16 instable.h > 16inst_s.h

size_s.h: size_tab.h
	$(PERL) make_size s size_tab.h > size_s.h

8size_s.h: size_tab.h
	$(PERL) make_size 8 size_tab.h > 8size_s.h

16size_s.h: size_tab.h
	$(PERL) make_size 16 size_tab.h > 16size_s.h

engine_s.o: 8inst_s.h 16inst_s.h 8size_s.h 16size_s.h size_s.h

.s.o:
	$(AS) -c $(OPTS) -I. $*.s

.c.o:
	$(CC) $(CCOPTS) $(XOPTS) -c $(OPTS) -I. -o $*.o $*.c

.cpp.o:
	$(CC) $(CPPOPTS) $(XOPTS) -c $(OPTS) -I. $*.cpp

.cpp.O:
	$(CC) $(CCOPTS) $(XOPTS) -c $(OPTS) -I. $*.c

partls: partls.c
	cc $(CCOPTS) $(XOPTS) $(OPTS) -o partls partls.c

to_pro: prodos.h prodos_protos.h to_pro.c
	cc $(CCOPTS) $(XOPTS) $(OPTS) -o to_pro to_pro.c

gsport32.o:   win32.rc winresource.h
	windres win32.rc -o gsport32.o

compile_time.o: $(OBJECTS)


# dependency stuff
adb.o: adb.c adb.h defc.h defcomm.h iwm.h protos.h
atbridge/aarp.o: defc.h atbridge/atbridge.h atbridge/port.h atbridge/elap.h atbridge/aarp.h atbridge/elap_defs.h
atbridge/atbridge.o: defc.h atbridge/atbridge.h atbridge/port.h atbridge/elap.h atbridge/llap.h atbridge/aarp.h
atbridge/elap.o: defc.h atbridge/atbridge.h atbridge/port.h atbridge/elap.h atbridge/aarp.h atbridge/elap_defs.h atbridge/pcap_delay.h
atbridge/llap.o: defc.h atbridge/atbridge.h atbridge/port.h atbridge/llap.h
atbridge/port.o: atbridge/atalk.h atbridge/port.h
atbridge/pcap_delay.o: atbridge/pcap_delay.h
engine_c.o: engine_c.c defc.h defcomm.h iwm.h protos.h protos_engine_c.h size_c.h op_routs.h defs_instr.h 8inst_c.h 16inst_c.h
clock.o: clock.c defc.h defcomm.h iwm.h protos.h
compile_time.o: compile_time.c
config.o: config.c defc.h defcomm.h iwm.h protos.h config.h
dis.o: dis.c defc.h defcomm.h iwm.h protos.h disas.h
scc.o: scc.c defc.h defcomm.h iwm.h protos.h scc.h
scc_llap.o: atbridge/atbridge.h atbridge/llap.h defc.h scc.h
scc_socket_driver.o: scc_socket_driver.c defc.h defcomm.h iwm.h protos.h scc.h
scc_windriver.o: scc_windriver.c defc.h defcomm.h iwm.h protos.h scc.h
scc_macdriver.o: scc_macdriver.c defc.h defcomm.h iwm.h protos.h scc.h
scc_imagewriter.o: scc_imagewriter.c defc.h defcomm.h protos.h scc.h
iwm.o: iwm.c defc.h defcomm.h iwm.h protos.h iwm_35_525.h
imagewriter.o: imagewriter.cpp
joystick_driver.o: joystick_driver.c defc.h defcomm.h iwm.h protos.h
moremem.o: moremem.c defc.h defcomm.h iwm.h protos.h
paddles.o: paddles.c defc.h defcomm.h iwm.h protos.h
parallel.o: parallel.c defc.h
printer.o: printer.cpp
sim65816.o: sim65816.c defc.h defcomm.h iwm.h protos.h
smartport.o: smartport.c defc.h defcomm.h iwm.h protos.h
sound.o: sound.c defc.h defcomm.h iwm.h protos.h sound.h
sound_driver.o: sound_driver.c defc.h defcomm.h iwm.h protos.h sound.h
video.o: video.c defc.h defcomm.h iwm.h protos.h superhires.h gsportfont.h
tfe.o: tfe/tfe.c tfe/tfe.h tfe/tfe_protos.h
tfearch.o: tfe/tfearch.c tfe/tfearch.h tfe/tfe_protos.h
tfesupp.o: tfe/tfesupp.c tfe/tfesupp.h tfe/tfe_protos.h
macdriver.o: macdriver.c defc.h defcomm.h iwm.h protos.h protos_macdriver.h
macdriver_console.o: macdriver_console.c defc.h defcomm.h iwm.h protos.h protos_macdriver.h
macdriver_generic.o: macdriver_generic.c defc.h defcomm.h iwm.h protos.h protos_macdriver.h
macsnd_driver.o: macsnd_driver.c defc.h defcomm.h iwm.h protos.h sound.h
windriver.o: windriver.c defc.h defcomm.h iwm.h protos.h protos_windriver.h winresource.h gsport32.o
win_console.o: win_console.c defc.h defcomm.h iwm.h protos.h protos_windriver.h winresource.h
win_generic.o: win_generic.c defc.h defcomm.h iwm.h protos.h protos_windriver.h winresource.h
win32snd_driver.o: win32snd_driver.c defc.h defcomm.h iwm.h protos.h sound.h
