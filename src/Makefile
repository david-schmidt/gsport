# $Id: release_makefile_base,v 1.15 2003/11/21 20:00:42 kentd Exp kentd $

OBJECTS1 = adb.o clock.o config.o dis.o engine_c.o scc.o iwm.o \
	joystick_driver.o moremem.o paddles.o sim65816.o smartport.o \
	sound.o sound_driver.o video.o scc_socket_driver.o scc_windriver.o \
	scc_macdriver.o

include vars

.SUFFIXES: .dep .proto

AS = $(CC)

XLIBS = -L/usr/X11R6/lib
PERL = perl

all: $(TARGET)

specials: 8inst_s 16inst_s 8size 16size 8inst_c 16inst_c size_c size_s

specials_clean:
	rm -f 8inst_s 16inst_s 8size 16size 8inst_c 16inst_c size_c size_s


# Mac builds:
gsportmac: $(OBJECTS) compile_time.o
	$(CC) $(CCOPTS) $(LDOPTS) -arch ppc $(OBJECTS) compile_time.o $(LDFLAGS) -o gsport $(EXTRA_LIBS) -prebind -framework Carbon -framework Quicktime
	mkdir -p ../GSport.app/Contents/Resources/English.lproj/main.nib
	mkdir -p ../GSport.app/Contents/MacOS
	mv gsport ../GSport.app/Contents/MacOS/GSportMac
	echo "APPL????" > ../GSport.app/Contents/PkgInfo
	cp -f Info.plist ../GSport.app/Contents/
	cp -f info.nib ../GSport.app/Contents/Resources/English.lproj/main.nib
	cp -f classes.nib ../GSport.app/Contents/Resources/English.lproj/main.nib
	cp -f objects.xib ../GSport.app/Contents/Resources/English.lproj/main.nib
	cp -f kegsicon.icns ../GSport.app/Contents/Resources/
	cp -f 525.icns ../GSport.app/Contents/Resources/
	cp -f 2mg.icns ../GSport.app/Contents/Resources/
	touch '../GSport.app/Icon?'
	cp -f ../config.template ../config.txt 

# Linux for X builds:
gsportx: $(OBJECTS) compile_time.o
	$(CC) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(XLIBS) $(EXTRA_LIBS) -lX11
	mv gsportx ..
	cp -f ../config.template ../config.txt 

# Cygwin for X builds:
gsport.exe: $(OBJECTS) compile_time.o
	$(CC) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(XLIBS) $(EXTRA_LIBS) -lXext -lX11 -lm
	mv gsport.exe ..
	cp -f ../config.template ../config.txt 

# Mingw32 (native windows) builds:
gsportwin.exe: $(OBJECTS) compile_time.o
	$(CC) $(CCOPTS) $(LDOPTS) $(OBJECTS) compile_time.o $(LDFLAGS) -o $(NAME)$(SUFFIX) $(EXTRA_LIBS) -lwinmm -lgdi32 -ldsound -lcomctl32 -lws2_32
	mv $(NAME)$(SUFFIX) ..
	cp -f ../config.template ../config.txt 


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
	$(CC) $(CCOPTS) $(XOPTS) -c $(OPTS) -I. $*.c

partls: partls.c
	cc $(CCOPTS) $(XOPTS) $(OPTS) -o partls partls.c

to_pro: prodos.h prodos_protos.h to_pro.c
	cc $(CCOPTS) $(XOPTS) $(OPTS) -o to_pro to_pro.c


compile_time.o: $(OBJECTS)


# dependency stuff
adb.o: adb.c adb.h defc.h defcomm.h iwm.h protos.h
engine_c.o: engine_c.c defc.h defcomm.h iwm.h protos.h protos_engine_c.h size_c.h op_routs.h defs_instr.h 8inst_c.h 16inst_c.h
clock.o: clock.c defc.h defcomm.h iwm.h protos.h
compile_time.o: compile_time.c
config.o: config.c defc.h defcomm.h iwm.h protos.h config.h
dis.o: dis.c defc.h defcomm.h iwm.h protos.h disas.h
scc.o: scc.c defc.h defcomm.h iwm.h protos.h scc.h
scc_socket_driver.o: scc_socket_driver.c defc.h defcomm.h iwm.h protos.h scc.h
scc_windriver.o: scc_windriver.c defc.h defcomm.h iwm.h protos.h scc.h
scc_macdriver.o: scc_macdriver.c defc.h defcomm.h iwm.h protos.h scc.h
iwm.o: iwm.c defc.h defcomm.h iwm.h protos.h iwm_35_525.h
joystick_driver.o: joystick_driver.c defc.h defcomm.h iwm.h protos.h
moremem.o: moremem.c defc.h defcomm.h iwm.h protos.h
paddles.o: paddles.c defc.h defcomm.h iwm.h protos.h
sim65816.o: sim65816.c defc.h defcomm.h iwm.h protos.h
smartport.o: smartport.c defc.h defcomm.h iwm.h protos.h
sound.o: sound.c defc.h defcomm.h iwm.h protos.h sound.h
sound_driver.o: sound_driver.c defc.h defcomm.h iwm.h protos.h sound.h
video.o: video.c defc.h defcomm.h iwm.h protos.h superhires.h kegsfont.h
macdriver.o: macdriver.c defc.h defcomm.h iwm.h protos.h protos_macdriver.h
macsnd_driver.o: macsnd_driver.c defc.h defcomm.h iwm.h protos.h sound.h
windriver.o: windriver.c defc.h defcomm.h iwm.h protos.h protos_windriver.h winresource.h
win32snd_driver.o: win32snd_driver.c defc.h defcomm.h iwm.h protos.h sound.h
