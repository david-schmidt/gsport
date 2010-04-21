# GSport OS/2 makefile

OBJECTS1 = dirport.obj adb.obj clock.obj dis.obj engine_c.obj scc.obj \
	iwm.obj joystick_driver.obj moremem.obj paddles.obj \
	sim65816.obj smartport.obj sound.obj sound_driver.obj \
	video.obj os2driver.obj config.obj

{..\..}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /Q /Gm /Gd- /C %s

{src\}.c.obj:
    @echo " Compile::C++ Compiler "
    icc.exe /DTCPV40HDRS /DBSD_SELECT /Dinline= /Q /Gm /Gd- /C %s

gsport.exe: $(OBJECTS1) ../../os2driver.c
	@echo " Link::Linker "
	icc.exe @<<
	/B" /pmtype:vio /st:196608 "
	/FePrivoxy.exe 
	so32dll.lib
	tcp32dll.lib
	$(OBJECTS1)
<<
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
os2driver.obj: ../../os2driver.c ../../scc.c ../../defc.h ../../defcomm.h ../../iwm.h ../../protos.h
