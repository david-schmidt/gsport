# GSport OS/2 makefile

OBJECTS1 = dirport.obj adb.obj clock.obj dis.obj engine_c.obj scc.obj \
	iwm.obj joystick_driver.obj moremem.obj paddles.obj \
	sim65816.obj smartport.obj sound.obj \
	video.obj config.obj scc_socket_os2.obj os2driver.obj

{..\..}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

{src\}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /I..\.. /Ss /Q /Ti /Gm /G4 /Ft- /C %s

gsport.exe: $(OBJECTS1)
	@echo " Link::Linker "
	icc.exe @<<
	/B" /de /pmtype:pm /st:196608 "
	/Fegsport.exe 
	so32dll.lib
	tcp32dll.lib
	$(OBJECTS1)
<<
	rc gsportos2.rc gsport.exe
	echo done

clean:
	- rm $(OBJECTS1)

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
