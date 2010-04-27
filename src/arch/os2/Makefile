# GSport OS/2 makefile

TARGET = gsport.exe

OBJECTS1 = dirport.obj adb.obj clock.obj dis.obj engine_c.obj scc.obj \
	iwm.obj joystick_driver.obj moremem.obj paddles.obj \
	sim65816.obj smartport.obj sound.obj \
	video.obj config.obj scc_socket_os2.obj os2driver.obj os2snd_driver.obj

{..\..}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

{src\}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

$(TARGET): $(OBJECTS1)
	@echo " Link::Linker "
	icc.exe @<<
	/B" /de /pmtype:pm /st:196608 "
	/Fe$(TARGET) 
	so32dll.lib
	tcp32dll.lib
	$(OBJECTS1)
<<
	rc gsportos2.rc $(TARGET)
	echo done

clean:
	- rm $(OBJECTS1)
	- rm $(TARGET)
	- rm gsportos2.res
	- rm ../../size_s.h
	- rm ../../size_c.h
	- rm ../../8size_s.h
	- rm ../../16size_s.h
	- rm ../../8inst_s.h
	- rm ../../8inst_c.h
	- rm ../../16inst_s.h
	- rm ../../16inst_c.h

../../size_c.h:
	call make_size.cmd

../../8inst_c.h:
	call make_inst.cmd

# dependency stuff
dirport.obj: src/dirport.c src/dirport.h
adb.obj: ../../adb.c ../../adb.h ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
engine_c.obj: ../../engine_c.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../protos_engine_c.h ../../size_c.h ../../op_routs.h ../../defs_instr.h ../../8inst_c.h ../../16inst_c.h
clock.obj: ..\..\clock.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
compile_time.obj: ../../compile_time.c
config.obj: ../../config.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../config.h
dis.obj: ../../dis.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../disas.h
scc.obj: ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../scc.h
scc_socket_driver.obj: ../../scc_socket_driver.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../scc.h
scc_socket_os2.obj: scc_socket_os2.c
iwm.obj: ../../iwm.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../iwm_35_525.h
joystick_driver.obj: ../../joystick_driver.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
moremem.obj: ../../moremem.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
paddles.obj: ../../paddles.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
parallel.obj: ../../parallel.c ../../defc.h
printer.obj: ../../printer.cpp
sim65816.obj: ../../sim65816.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
smartport.obj: ../../smartport.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
sound.obj: ../../sound.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
sound_driver.obj: ../../sound_driver.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../sound.h
video.obj: ../../video.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h ../../superhires.h ../../gsportfont.h
os2driver.obj: os2driver.c gsportos2.h ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
os2snd_driver.obj: os2snd_driver.c
