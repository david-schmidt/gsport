# $Id: README.compile,v 1.20 2004/10/18 04:05:14 kentd Exp $

General build instructions:
--------------------------

You need to build with a make utility.  I've only tested GNU make.
There's a default Makefile, which should work for nearly any environment.
The Makefile includes a file called "vars" which defines the platform-
dependent variables.  You need to make vars point to the appropriate
file for your machine.

This makes my maintenance of the diverse platforms a bit easier.

WIN32 build instructions:
------------------------

See the file README.win32 for build instructions and other information
for Microsoft Windows.


Mac OS X  build instructions (the default):
------------------------------------------

KEGS is easy to compile.  Just cd to the src directory and type "make".
KEGS requires perl to be in your path (or just edit the vars file to give
the full path to wherever you installed perl).  Perl version 4 or 5 is
fine.

After the "make" has finished, it will create the application KEGSMAC.

To run, see README.mac.

X86 Linux build instructions:
----------------------------

Use the vars_x86linux file with:

rm vars; ln -s vars_x86linux vars
make

The resulting executable is called "xkegs".

The build scripts assume perl is in your path. If it is somewhere else,
you need to edit the "PERL = perl" line in the vars file and make it point
to the correct place.

For audio, KEGS needs access to /dev/dsp.  If the permissions do not allow
KEGS to access /dev/dsp, it can fail with a cryptic error message.  As root,
just do: "chmod 666 /dev/dsp".


PowerPC Linux build instructions:
----------------------------

Use the vars_linuxppc vars file by:

rm vars; ln -s vars_linuxppc vars
make

The build scripts assume perl is in your path. If it is somewhere else,
you need to edit the "PERL = perl" line in the vars file and make it point
to the correct place.

Audio is currently disabled by default, but you can try turning it on
by runnning "xkegs -audio 1".  It sounds horrible to me, but sounds do
come out.

Solaris SPARC build instructions:
--------------------------------

Use the vars_solaris vars file by:

rm vars; ln -s vars_solaris vars
make

The build scripts assume perl is in your path. If it is somewhere else,
you need to edit the "PERL = perl" line in the vars file and make it point
to the correct place.

Audio is currently disabled by default, but you can try turning it on
by runnning "xkegs -audio 1".

Solaris x86 build instructions:
--------------------------------

Use the vars_x86solaris vars file by:

rm vars; ln -s vars_x86solaris vars
make

The build scripts assume perl is in your path. If it is somewhere else,
you need to edit the "PERL = perl" line in the vars file and make it point
to the correct place.

Audio is currently disabled by default, but you can try turning it on
by runnning "xkegs -audio 1".

HP-UX assembly-emulation instructions:
-------------------------------------

Use the vars_hp vars file by:

rm vars; ln -s vars_hp vars

Edit the Makefile, and remove "engine_c.o" from the "OBJECTS1=" line at
the top.  Then just type "make".

This version is quite out of date and most likely does not compile any more.

Other platform "C" build instructions:
-------------------------------------

If you are porting to an X-windows and Unix-based machine, it should be
easy.  Start with vars_x86linux if you are a little-endian machine, or
vars_linuxppc if you are big endian.  Don't define -DKEGS_LITTLE_ENDIAN
unless your processor is little-endian (x86, Alpha).  Mac, Sun, MIPS,
HP, Motorola, and IBM are big-endian.

