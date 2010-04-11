
KEGS: Kent's Emulated GS version 0.91
http://kegs.sourceforge.net/

What is this?
-------------

KEGS is an Apple IIgs emulator for Mac OS X, Linux, and Win32.
The Apple IIgs was the most powerful computer in the Apple II line.
It first was sold in 1986.  An Apple IIgs has the capability to run almost all
Apple II, Apple IIe, and Apple IIc programs.

KEGS supports all Apple IIgs graphics modes (which include all Apple //e
modes), plus plays all Apple IIgs sounds accurately.  It supports
serial port emulation through TCP/IP connections, or can use real
serial ports on Windows and Mac OS X.

The ROMs and GS/OS (the Apple IIgs operating system) are not included
with KEGS since they are not freely distributable.  KEGS is a little
user-hostile now, so if something doesn't work, let me know what went
wrong, and I'll try to help you out.  See my email address at the end of
this file.

I'd like to thank Chea Chee Keong who created KEGS32, the first Windows
port of KEGS.  That version, which has a better Windows-interface but which
is based on older core code, is at http://www.geocities.com/akilgard/kegs2.

KEGS features:
-------------

Fast 65816 emulation:
	About 80MHz on a Pentium 4 1.7GHz or a Mac G4 1GHz.
Emulates low-level 5.25" and 3.5" drive accesses (even nibble-copiers work!).
Emulates classic Apple II sound and 32-voice Ensoniq sound.
	All sound is played in 16-bit stereo at 48KHz (44100 on a Mac).
Emulates all Apple IIgs and Apple II graphics modes, including border effects.
	Can handle display changes at any time (superhires at the top, lores
	at the bottom).  Always does 60 full screen video updates per second.
	Supports 3200-color pictures.
Mouse and joystick support.
Emulated battery RAM remembers control panel settings.
Limited SCC (serial port) emulation to enable PR#1/2 IN#1/2 and Virtual
	Modem enables standard Apple terminal programs to telnet to any
	internet address (or receive connections from another telnet).

KEGS by default emulates a 8MB Apple IIgs, but you can change this from
the Configuration Panel.

KEGS is so accurate, even the built-in ROM selftests pass (you must be in
2.8MHz speed mode to pass the self-tests and you must set the Configuration
Panel entry "Enable Text Page 2 shadow" to "Disabled on ROM 01" for ROM 01).

Release info:
------------

Included files:
	CHANGES			- Description of changes since last release
	README.kegs.txt		- you're here
	README.compile.txt	- Describes how to build KEGS
	README.linux.rpm.txt	- Describes how to install KEGS's RPM for Linux
	README.win32.txt	- Win32 special directions
	README.mac.txt		- Mac OS X special directions
	README.a2.compatibility.txt - List of programs which need tweaking
	src/INTERNALS.overview	- description of how KEGS code works
	src/INTERNALS.xdriver	- Describes the xdriver.c routines for porting
	src/INTERNALS.iwm	- Describes the internal 3.5" and 5.25" disk
				   handling routines
	KEGSMAC			- the Mac OS X executable
	kegswin.exe		- the Windows executable
	config.kegs		- disk image configuration info
	to_pro			- Hard-to-use ProDOS volume creator
	partls			- Lists partitions on Apple-partitioned hard
				   drives or CD-ROMs
	src/			- All the source code, with a Makefile

You need to provide:

	1) Patience.
	2) a ROM file called "ROM", "ROM.01" or "ROM.03" in the KEGS directory
		(or in your home directory).  It can be either from a ROM 01
		(131072 bytes long) or from a ROM 03 machine (262144 bytes
		long.)
	3) A disk image to boot.  This can be either "raw" format or 2IMG.
		See discussion below.  GS/OS would be best.

Getting ROMs
------------

You need a copy of the memory from fe/0000 - ff/ffff from a ROM 01 GS
or fc/0000 - ff/ffff from a ROM 03 GS, and put that in a file called
"ROM".  I'll eventually write detailed instructions on how to do this.

Running KEGS:
------------

The distribution comes with the full source code for all platforms in
the src/ directory, the Windows executable as kegswin.exe, and the Mac OS X
executable as KEGSMAC.

See the README.compile.txt file for more info about compiling for Linux.

On all platforms except the Mac, you must start KEGS from a terminal
window.  KEGS will open a new window and use the window you started it from
as a "debug" window.  

On a MAC, you need to place the "config.kegs" file someplace where KEGS
can find it.  The simplest place is in your home directory, so copy it there
with the Finder (or using the Terminal).  You can also make the directory
Library/KEGS from your home directory, and then place config.kegs there
along with the ROM file.  You do not need a starting config.kegs file
on a Mac--KEGS will offer to make it for you if it cannot find one.

Start kegs by Double-clicking the KEGSMAC icon on a MAC, or by running
the executable (kegswin on Windows, and kegs on Linux).  KEGSMAC can
be run by the Terminal window on a Mac as well (which enables access to
more debug information) by typing: "./KEGSMAC.app/Contents/MacOS/KEGSMAC".

Assuming all goes well, KEGS will then boot up but probably not find any
disk images.  See below for how to tell KEGS what disk images to use.
Tip: Hitting "F8" locks the mouse in the window (and hides the host cursor)
until you hit "F8" again.

Configuration Panel:
-------------------

You enter the Configuration panel by pressing F4 at any time.  You tell
KEGS what disk images to use through the Configuration panel.  (If KEGS
couldn't find a ROM file, you will be forced into the Configuration
Panel mode until you select a valid ROM file).

To select a ROM file, select "ROM File Selection" and then select your
ROM file.  If you were not forced into the panel at startup, the KEGS
found one and you can skip this step.

Disk Images
-----------

The primary use of the Configuration Panel is to select disk images.  To
change disk images being used, select "Disk Configuration".  Each slot
and drive that can be loaded with an image is listed.  "s5d1" means slot
5, drive 1.  Slot 5 devices are 3.5" 800K disks, and slot 6 devices are
5.25" 140K disks.  Slot 7 devices are virtual hard drives, and can be
any size at all (although ProDOS-formatted images should be less than
32MB).

Just use the arrow keys to navigate to the device entry to change, and
then select it by pressing Return.  A scrollable file selection
interface is presented, letting you locate your image files.  To quickly
jump to a particular path, you can press Tab to toggle between entering
a path manually, and using the file selector. Press Return on ".."
entries to go up a directory level.  When you find the image you want,
just press Return.

If the image has partitions that KEGS supports, another selection
dialog will have you select which partition to mount.  You will probably
only have partitions on direct devices you mount (or on a Mac, of .dmg
images of CDs).  For instance, on a Mac, /dev/disk1 can sometimes be the
CDROM drive.

KEGS can handle "raw", .dsk, .po, 2IMG, 5.25" ".nib" images, most Mac
Diskcopy images and partitioned images.  The .dsk and .po formats you often
find on the web are really "raw" formats, and so they work fine.  KEGS uses
the host file permissions to encode the read/write status of the image.
KEGS can open any image file compressed with gzip (with the extension ".gz")
automatically as a read-only disk image.

An image is the representation of an Apple IIgs disk, but in a file on
your computer.  For 3.5" disks, for example, a raw image would be exactly
800K bytes long (819200 bytes).  KEGS directs the emulated GS accesses to
the image, and does the correct reads and writes of the Unix file instead.

To do "useful" things with KEGS, you need to get a bootable disk image.
You can go to http://www.info.apple.com/support/oldersoftwarelist.html and
get Apple IIgs System 6.  Unfortunately, Apple now only has .sea files which
are executable files for Macintosh only.  You need a macintosh to execute
those programs, which creates Disk Copy image files with no special extensions
(and with spaces in the names).  Once you get those files back to your
host machine, you can use them by selecting them from the Configuration Panel.

You can also get Apple II programs in ".dsk" format from a variety of
sites on the internet, and these should all work on KEGS as well.

KEGS also supports partitioned devices.  For instance, if you have a CD-ROM
on your computer, just pop an Apple II CD in, and KEGS can mount it, if
you have a Unix-based system (Linux, any Unix, and Mac OS X).

If you're on a Mac, be careful letting KEGS use your HFS partitions--
GSOS has many HFS bugs when it is writing.  Also avoid having KEGS access
an image which have mounted on your Mac at the same time (always unmount
it from your Mac before letting KEGS access it)!

If you do not have any disk mounted in s7d1, KEGS will jump into the monitor.
To boot slot 6 (or slot 5), use the Apple IIgs Control Panel by pressing
Ctrl-Command-ESC.

Support for 5.25" nibblized images is read-only for now (since the
format is kinda simplistic, it's tricky for KEGS to write to it since KEGS
has more information than fits in that format).  Just select your image,
like "disk.nib" in the kegs_conf file like any .dsk or .po image.

In addition to changing disks, you can also just "eject" and image by
moving the cursor to select that slot/drive and then press "E".  The
emulated IIgs will immediately detect changes to s5d1 and s5d2.

Care should be taken when changing images in slot 7--KEGS does not notify
GSOS that images have changed (or been ejected), and so it's best to make
changes when GSOS is not running.


Key summary:
-----------

F1:	Alias of Command
F2:	Alias of Option
F3:	Alias of ESC for OS/2 compatibility.
F4:	Configuration Panel
F6:	Toggle through the 4 speeds: Unlimited, 1MHz, 2.8MHz, 8.0MHz
Shift-F6: Enter KEGS debugger
F7:	Toggle fast_disk_emul on/off
F8:	Toggle pointer hiding on/off.
F9:	Invert the sense of the joystick.
Shift-F9: Swap x and y joystick/paddle axes.
F10:	Attempt to change the a2vid_palette (only useful on 256-color displays)
F11:	Full screen mode (only on Mac OS X).
F12:	Alias of Pause/Break which is treated as Reset

F2, Alt_R, Meta_r, Menu, Print, Mode_switch, Option:   Option key
F1, Alt_L, Meta_L, Cancel, Scroll_lock, Command:       Command key
Num_Lock:		Keypad "Clear".
F12, Pause, Break:	Reset

"Home": Alias for "=" on the keypad (since my Unix keyboard doesn't have an =).

Using KEGS:
----------

The host computer mouse is the Apple IIgs mouse and joystick by default.
By default, the host pointer is not constrained inside the window and
remains visible.  Press F8 to hide the cursor and constrain the mouse.  F8
again toggles out of constrain mode.  When the GSOS desktop is running,
KEGS hides the host cursor automatically and enables special tracking
which forces the emulated cursor to follow the host cursor.  If this doesn't
work right under some program, just press F8 for better compatibility.

The middle mouse button or Shift-F6 causes KEGS to stop emulation, and enter
the debugger.  You can continue with "g" then return in the debug window.
You can also disassemble memory, etc.  The section "Debugging KEGS"
above describes the debugger interface a little more.

KEGS has no pop-up menus or other interactive interfaces (other than
the debug window, and the occasional error dialogs on Mac OS X).  Input to
the debug window is only acted upon when the emulation is stopped
(Shift-F6, middle mouse button, or hitting a breakpoint).

Quitting KEGS:
-------------

Just close the main KEGS window, and KEGS will exit cleanly.  Or you
can select Quit from the menu.  Or enter ctrl-c in the debugger window.
Or press the middle-mouse button in the emulation window, and then type
"q" return in the debug window.

Command/Option keys:
-------------------

If you have a keyboard with the special Windows keys, you can
use them as the command/option keys.  For those without those keys,
there are several alternatives.

The following keys are Option (closed-apple) (not all keyboards have all
keys):  F2, Meta_R, Alt_R, Cancel, Print_screen, Mode_switch, Option,
or the Windows key just to the right of the spacebar.  The following keys are
Command (open-apple):  F1, Meta_L, Alt_L, Menu, Scroll_lock, Command,
the Windows key left of the spacebar, and the Windows key on the far right
that looks like a pull-down menu.  You can use F1 and F2 if you cannot make
anything else work (especially useful if your OS is intercepting some
Alt or Command key sequences).

If you can't get any of these to work on your machine, let me know.
Note that X Windows often has other things mapped to Meta- and Alt-
key sequences, so they often don't get passed through to KEGS.  So it's
best to use another key instead of Alt or Meta.

The joystick/paddle buttons are just the Command and Option keys.


Reset:
-----

The reset key is Pause/Break or F12.  You must hit it with Ctrl to get it to
take effect (just like a real Apple IIgs).  Ctrl-Command-Reset
forces a reboot.  Ctrl-Command-Option-Reset enters selftests.
Selftests will pass if you force speed to 2.8MHz using the middle
button or F6 (and also set Enable Text Page 2 shadow = Disabled for ROM 01).
Watch out for ctrl-shift-Break--it will likely kill an X Windows session.
Also note that the Unix olvwm X window manager interprets ctrl-F12 and will
not pass it on to KEGS--you'll need to use Break for reset in that case.

Full Screen mode (MAC OS X ONLY):
----------------

KEGS can run in full screen mode--which is especially useful when letting
small kids use KEGS (but it is not really a lock, so do not let a 2 year
old bang on the keyboard while running KEGS).

Full Screen mode is toggled with F11 (or Ctrl-F11, since Expose on a Mac
is intercepting F11).  If KEGS stops in the debugger for any reason,
full screen mode is toggled off automatically.


Joystick Emulation (Mouse, Keypad, or real native joystick):
------------------

The default joystick is the mouse position.  Upper left is 0,0.  Lower right
is 255,255.  Press Shift-F9 to swap the X and Y axes.  Press F9 to reverse
the sense of both paddles (so 0 becomes 255, etc).  Swapping and
reversing are convenient with paddle-based games like "Little Brick Out"
so that the mouse will be moving like the paddle on the screen.  "Little
Brick Out" is on the DOS 3.3 master disk.  The joystick does not work
properly if the pointer is constrained in the window.

You can also select from a "Keypad Joystick" or a real joystick from
the Configuration panel.  Press return on the "Joystick Configuration"
entry, and then select between Mouse Joystick, Keypad Joystick, or one
of two native joysticks.  The Keypad Joystick uses your keypad number
keys as a joystick, where keypad 7 means move to the upper left, and
keypad 3 means move to the lower right.  Pressing multiple keys together
averages the results, allowing finer control than just 8 directions.
Also, joystick scaling is selectable here for games which require
a greater range of motion to work correctly, along with trim adjustment
which moves the centering point.  Adjusting scaling usually means you
will need to adjust the trim as well.

The left mouse button is the mouse button for KEGS.  The right mouse
button (if you have it) or F6 toggles between four speed modes.  Mode 0
(the default) means run as fast as possible.  Mode 1 means run at 1MHz.
Mode 2 means run at 2.8MHz.  Mode 3 means run at 8.0MHz (about the speed
of a ZipGS accelerator).  Most Apple //e (or earlier) games need to be
run at 1MHz.  Many Apple IIgs demos must run at 2.8MHz or they will not
operate correctly.  Try running ornery programs at 2.8MHz.  3200 pictures
generally only display correctly at 2.8MHz or sometimes 8.0MHz.


Debugging KEGS:
--------------

KEGS by default now continues emulation even when it detects buggy programs
running.  (Now I know why Appleworks GS always seemed to crash!).

KEGS divides buggy programs into two severities: Code Yellow and Code Red.
The status is displayed in words in the text area under the emulation window.
If nothing's wrong, nothing is printed.

A Yellow bug is a mild bug where an Apple IIgs program merely read an
invalid location.  Although completely harmless, it indicates the potential
for some Apple IIgs program bug which may become more severe shortly.
For instance, closing the "About This Apple IIgs" window in the Finder
causes a code yellow alert, but it seems quite harmless.

A Code Red bug is a more serious problem.  The Apple IIgs program either
tried to write non-existent memory, entered an invalid system state, or
perhaps just tried to use an Apple IIgs feature which KEGS does not implement
yet.  Note that entering GSBUG tends to cause a Code Red alert always, so if
you intended to enter it, you can ignore it.  My recommendation is to
save work immediately (to new files) and restart KEGS if you get into the
Red mode.

KEGS also supports breakpoints and watchpoints.  In the debug window, you
set a breakpoint at an address by typing the address, followed by a 'B'
(it must be in caps).  To set a breakpoint on the interrupt jump point,
type:

e1/0010B

The format is "bank/address" then "B", where the B must be in caps and
the address must use lower-case hex.  For Apple IIe programs, just use a
bank of 0.

To list all breakpoints, just type 'B' with no number in front of it.
To delete a breakpoint, enter its address followed by 'D', so

e1/0010D

deletes the above breakpoint.  The addresses work like the IIgs monitor:
once you change banks, you can use shortcut addresses:

e1/0010B
14B

will add breakpoints at e1/0010 and e1/0014.

This is a "transparent" breakpoint--memory is not changed.  But any
read or write to that address will cause KEGS to halt.  So you can
set breakpoints on I/O addresses, or ROM, or whatever.  Setting a breakpoint
slows KEGS down somewhat, but only on accesses to the 256 byte "page"
the breakpoint is on. Breakpoints are not just instruction breakpoints,
they also cause KEGS to halt on any data access, too (usually called
watchpoints).

Frederic Devernay has written a nice help screen available in the
debugger when you type "h".

Useful locations for setting breakpoints:
0/3f0B		- Break handler
0/c000B		- Keyboard latch, programs read keys from this address



KEGS command-line option summary:
--------------------------------

There are others, but the Configuration panel provides a better way to
set them so they are no longer listed here.
-skip:	KEGS will "skip" that many screen redraws between refreshes.
	-skip 0 will do 60 frames per second, -skip 1 will do 30 fps,
	-skip 5 will do 10 fps.
-audio [0/1]: Forces audio [off/on].  By default, audio is on unless
	the X display is a remote machine or shared memory is off.
	This switch can override the default.  -audio 0 causes KEGS to
	not fork the background audio process, but Ensoniq emulation
	is still 100% accurate, just the sound is not sent to the
	workstation speaker.  Audio defaults off on Linux for now.
-arate {num}: Forces audio sample rate to {num}.  44100 and 48000 are
	usual, you can try 22050 to reduce KEGS's overhead.  On a reasonably
	fast machine (>250MHz or so), you shouldn't need to mess with this.
-dhr140: Will use the old Double-hires color algorithm that results in
	exactly 140 colors across the screen, as opposed to the blending
	being done by default.

X-Windows/Linux options
-15:	KEGS will only look for a 15-bit X-Window display.
-16:	KEGS will only look for a 16-bit X-Window display (not tested, probably
	 will get red colors wrong).
-24:	KEGS will only look for a 24-bit X-Window display.
-display {machine:0.0}: Same as setting the environment variable DISPLAY.
	Sends X display to {machine:0.0}.
-noshm:	KEGS will not try to used shared memory for the X graphics display.
	This will make KEGS much slower on graphics-intensive tasks,
	by as much as a factor of 10!  By default, -noshm causes an
	effective -skip of 3 which is 15 fps.  You can override this
	default by specifying a -skip explicitly.



Apple IIgs Control Panel:
------------------------

You can get to the Apple IIgs control panel (unless some application
has locked it out) using Ctrl-Command-ESC.


How to use "to_pro":
-------------------

This lame utility serves two purposes:  It "formats" large disk images,
and lets you move files from Unix into the simulator.  It does this
by taking the files you provide, and putting them onto Unix file called
"POOF1" that is an image in ProDOS format.

So, if you have a wolfdemo.bxy file from an FTP site, you can get it
into the emulator by:

to_pro -800 wolfdemo.bxy

which creates an 800K Unix file called "POOF1".  POOF1 is now an
image that can be loaded into KEGS, and when you catalog it, it will
have wolfdemo.bxy on it.

To create a 4MB image:

to_pro -4096 wolfdemo.bxy

which puts wolfdemo.bxy on a much larger image.

I don't know what happens if the file, wolfdemo.bxy, is bigger than
the image...it probably crashes.

Even if you want to format a "blank" image, you have to put something in it.
Like:

echo "This is a lame utility" > foo
to_pro -16384 foo

...creates a 16MB POOF1 with the file foo on it.  Just delete foo
from within KEGS.

See?  I told you it was a lame utility!

to_pro can handle up to 51 files at a time--for example:

to_pro -32000 *.shk

...would put all *.shk files in the current Unix directory into a 31.25MB
image called POOF1.

To_pro tries to truncate Unix filenames to the 15 character ProDOS
limit, and converts all punctuation to dots.  I've tested it enough
that it has worked for my purposes.

The algorithm to_pro uses to create a disk volume is possibly suspect.
I recommend reformatting any images again inside KEGS (using GS/OS, for
instance) just to make sure the directory structure is good.  To_pro
is intended to put files into images quickly and easily, and then to
copy the files off of those images onto images formatted from within
KEGS by an Apple IIgs OS.

Since ProDOS cannot handle > 32MB images, make sure you run to_pro with
arguments under 32767.  I personally haven't tried a partition bigger
than 30000K (about 2.5MB short of the maximum).  Well, you can use bigger
images if you format them HFS, but I don't trust the GS/OS HFS driver.

To_pro automatically sets the ProDOS filetype of files ending in ".shk"
to $E0.

Details on config.kegs and disk images
--------------------------------------

The file "config.kegs" describes the images KEGS will use.  Although you
can edit the file manually, in general you can use the Configuration Panel
to make all the changes you need.  This information is for reference.

KEGS by default will boot s7d1 (unless you've changed that using the
Apple IIgs control panel), so you should put an image in that slot.

KEGS, by default, runs the IWM (3.5" and 5.25" disks) emulation in an
"approximate" mode, called "fast_disk_emul".  In this mode, KEGS
emulates the hardware "faster" than real, meaning the data the code
being emulated expects is made available much faster than on a real
Apple IIgs, providing a nice speed boost.  For instance, the 5.25"
drives run 10x the real speed usually.  Almost everything will work
except for nibble copiers, which don't like the data coming this fast.
(Meaning, unless you're using a nibble copier, you shouldn't run into an
issue.  All games/demos/etc run fine in this mode).  To make nibble
copiers work, Press F7.

KEGS can read in the ".nib" nibblized disk format, but as read-only mode.  If
the emulated image is no longer ProDOS or DOS 3.3 standard, KEGS will
automatically treat the image as "Not-write-through-to-Image" from then
on.  This mode means KEGS will continue to emulate the disk properly in
memory, but it cannot encode the changes in the standard .dsk or .nib
image format.  It prints a message saying it has done so.  However,
the "disk" in emulation is fully useable as long as KEGS is running.  A
standard reformatting will not cause an image to flip to not-write-
through-to-Image, but running things like a "drive-speed" test will cause
further changes not to propagate to the Unix file.  You will need
to "eject" the image and re-insert it before writes will take effect.

In full accuracy mode (i.e., not fast_disk_emul), 5.25" drive accesses
force KEGS to run at 1MHz, and 3.5" drive accesses force KEGS to run at
2.5MHz.

KEGS Timing:
-----------

KEGS supports running at four speeds:  1MHz, 2.8MHz, 8.0MHz, and Unlimited.
Pressing the middle mouse button cycles between these modes.  The 1MHz
and 2.8MHz speeds force KEGS to run at exactly those speeds, providing
accurate reproduction of a real Apple IIgs.

KEGS will always run at 1MHz at least.  If it is unable to keep up,
it will extend the emulated time to maintain the illusion of running
at 1MHz.  That is, it may do just 40 screen refreshes per real second,
instead of the usual 60.  This happens rarely.

If you force KEGS to run at 1MHz, it will strive to run at exactly
1MHz (well, really 1.024MHz).  If it is running faster (almost always),
it will pause briefly several times a second to maintain the 1MHz speed.  It
does this in a friendly way that makes time available to other tasks.
This makes older Apple II games very playable just like a
real Apple IIgs on slow speed.  KEGS is running at exactly the same
speed as an Apple //e when in 1MHz mode.  The 1MHz mode you set
through the right mouse button overrides the "fast" mode you can access
through the control panel.  But, 3.5" accesses will "speed up" to 2.8MHz
to enable that code to operate correctly while the 3.5" disk is being
accessed.

If you force KEGS to run at 2.8MHz, KEGS tries to run at exactly 2.8MHz.  But
like a real unaccelerated Apple IIgs, if you set the control panel to
"slow", it will really be running at 1MHz.  Accesses to 5.25" disk
automatically slow down to 1MHz, when running the IWM in accurate
mode (F7).  KEGS may not be able to keep up with some programs running
at 2.8MHz due to video and sound overheads on lower-end machines.  If
that happens, it effectively runs slower by extending the emulated
"second", like in the 1MHz mode.  You can tell this is happening
when Eff MHz in the status area falls below 2.5MHz.  If KEGS is running
faster than 2.8MHz, it takes small pauses to slow down, just like in
1MHz.  Many Apple IIgs demos must be run at 2.8MHz.  The built-in
selftests (cmd-option-ctrl-Reset) must run at 2.8MHz.  Many Apple IIgs
action games are more playable at 2.8MHz.

The 8.0MHz setting means follow the ZipGS-selected speed, but don't go
faster than 8.0MHz.  If your host computer cannot keep up, then the
emulated second will be extended.  You can use the ZipGS control panel,
or ZIPPY.GS on the sample disk image to set the emulated ZipGS speed to
anything from 1MHz to 8MHz in .5MHz increments.

The Unlimited setting means run as fast as possible, whatever speed that
is (but always above 1MHz).  Eff MHz gives you the current Apple IIgs
equivalent speed.  Many games will be unplayable at the unlimited
setting.  Setting the IIgs control panel speed to "slow" will slow down
to 1MHz.

Sound output has an important relationship to KEGS timing.  KEGS must
play one second of sound per second of emulated time.  Normally, this
works out exactly right.  But as noted above, if KEGS can't maintain the
needed speed, it extends the emulated second.  If it extends the second
to 1.4 real seconds, that means KEGS only produces 1.0 second of sound
data every 1.4 seconds--the sound breaks up!

In all cases, 1MHz to KEGS is 1.024MHz.  And 2.8MHz to KEGS is 2.56MHz
(trying to approximate the slowdown causes by memory refresh on a real
Apple IIgs).  It's just easier to say 1MHz and 2.8MHz.


KEGS SAMPLE_DISK:
----------------

I'm providing a sample disk of freely available utilities/programs to
demonstrate a little of what KEGS can do.  I'm also including my simple
changes to a benchmark called "SPEEDTEST" to make it run under ProDOS and
time itself automatically.  The SAMPLE_DISK is not bootable since I'm
not sure if I can distribute PRODOS (the OS).


   SPEEDTEST:
   ---------

In the folder "SPEEDTEST", there are two BASIC programs.  OLD.SPEEDTEST
is the old, unmodified DOS 3.3 emulator benchmark by Clayten Hamacher.
It does not run properly under ProDOS 8.  My modified version is
SPEED.PRO, meaning converted to ProDOS.  I made few modifications, other
than to make the benchmarks time themselves.

To run, just say "RUN SPEED.PRO".  To run benchmarks, press "B".  If
you say "A)ll tests", make sure you have a 5.25" disk image in s6d1!
(A blank 140K image will work fine).

This modified SPEED.PRO can run on ANY Apple IIgs emulator (or on the real
thing).

   GSOS7, GSOS5, BYE.SYSTEM:
   ------------------------

These are handy utilities I use on my s7d1 boot disk.  Get a GS/OS 6.x
bootable disk image.  (See GSOS.INFO file for how to get GS/OS).
Remove "PRODOS" from that disk's root directory, and copy GSOS7 to
the root directory.  Then copy SYSTEM/P8 to PRODOS.  Then move
BASIC.System into SYSTEM/.  Then copy BYE.SYSTEM to the root directory,
then move BASIC.SYSTEM back to the root directory.

What all this means is that now the root directory of your system disk
is:  GSOS7, (other stuff), PRODOS, BYE.SYSTEM, and BASIC.SYSTEM.
When you boot, ProDOS will boot (this is PRODOS 8) and will search
for the first *.SYSTEM file, and run it.  BYE.SYSTEM just does a BYE
command, which puts you in the PRODOS 8 textual launcher.
If you now select GSOS7 (the first entry, already highlighted, just
hit return), it will boot GSOS on slot 7.  (Use GSOS5 to boot slot5).
Or, just move down and select BASIC.SYSTEM to go to BASIC.  A very simple
program launcher!?

Note that I didn't write GSOS5 or GSOS7--I just made a one byte hack
to the default GS/OS launcher.  No real wizardry is going on here.


   SHRINKIT3.4, GSHK1.1:
   --------------------

Useful for unpacking .SHK files you can download off of the net.
Always use GSHK (GS/OS version of ShrinkIt) for GS programs since
they may have resource forks.  It's also faster.  GSHK must be run from GS/OS.

   LISTV2.0:
   --------

ProDOS 8 text file lister, useful for viewing text files.

   Wolfenstein3D:
   ------

Wolfenstein 3D for the Apple IIgs.  No kidding!  Must be run from GS/OS.

   SOUND22:
   -------

Cool little ProDOS 8 program (SOUND.EDITOR) that plays hi-fidelity
(relatively) through the old Apple II speaker.  This is included as a
demonstration of how accurate KEGS sound emulation is.

   Sound.Smith.95:
   --------------

GS/OS application that plays SoundSmith songs, which are spreadsheet music,
like MODs.  I included some sample songs--FILE.11, FILE.16, FILE.17, and
SPACE.HARRIER.  Enjoy!

   SOLITAIRE:
   ---------

Klondike.  I like the interface on this game.

   CAT.DOCTOR:
   ----------

From Prosel8 (which is now public domain), this utility is very handy for
sorting directories (among other things).  Useful for arranging GSOS7,
and BYE.SYSTEM mentioned above.

   BGSOUND:
   -------

This CDA lets you play Soundsmith songs in the background while other
applications are running.  Very handy for playing Solitaire with some music.

   DOCVu.CDA:
   ---------

This CDA shows the current DOC contents in real-time.  It has neat visual
effects while playing Soundsmith songs.

   Zippy.gs
   --------

Very useful ProDOS 8 program by Andy McFadden for setting ZipGS parameters.
In KEGS, you'll want to use this to change the Zip speed to less than
100% to make the "Unlimited" speed become limited to 7.5MHz, which is
useful for some games.


KEGS:  What works:
-----------------

Basically, just about every Apple II program works.  See the file
README.a2.compatibility for directions on how to make certain games/programs
work.

KEGS is EXTREMELY compatible.  But, I haven't tested everything.  Let
me know if you find a program which is not working correctly.

Some old Apple II 5.25" games require the old C600 ROM image, and don't work
with the default Apple IIgs ROM.  This is not KEGS's fault--these games
don't run on a real Apple IIgs either.  KEGS has built-in the old Apple II
Disk PROM which you can enable by using the IIgs control panel to set
Slot 6 to "Your Card".  This allows many more Apple II games to run, and
is the recommended setting.

The NinjaForce Megademo mostly works, but sometimes hangs in the BBS Demo.
Just skip that demo if it happens.

The California Demo hangs at startup unless you use the IIgs control panel
to boot from slot 5, and then do a ctrl-Open_Apple-Reset to boot--doing
the above lets it work fine. This seems to be a bug in the demo.


KEGS bugs:
---------

On a ROM03, KEGS makes a patch to the ROM image (inside emulation, not
to the Unix file) to fix a bug in the ROM code.  Both ROM01 and ROM03
are patched to enable use of more than 8MB of memory.  I then patch the ROM
self-tests to make the ROM checksum pass.  But other programs, like
the Apple IIgs Diagnostic Disk, will detect a ROM checksum mismatch.
Don't worry about it.

Sound breaks up if KEGS is unable to keep up--it should only be happening
if you are trying to force KEGS to run at 2.8MHz, but cannot due to
sound and video overhead.


Sound emulation:
---------------

KEGS supports very accurate classic Apple II sound (clicking of the
speaker using $C030) and fairly accurate Ensoniq sound.

When KEGS determines that no sound has been produced for more than
5 seconds, it turns off the sound calculation routines for a small
speedup.  It describes that it has done this by saying "Pausing sound"
in the debug window.  However, when sound restarts, it sometimes
"breaks-up" a little.  I will work on fixes for this.

If your display is not using shared memory, audio defaults to off unless
you override it with "-audio 1".

SCC (Serial Port) emulation:
---------------------------

KEGS emulates the two serial ports on a IIgs as being two Unix sockets.
Port 1 (printer port) is at socket address 6501, and port 2 (modem)
is at socket address 6502.

By default, slot 1 is emulated using a simple receive socket, and slot 2
emulates a Virtual Modem.

A Virtual Modem means KEGS acts as if a modem is on the serial port
allowing Apple II communcation programs to fully work, but connected to
internet-enabled sockets.  KEGS emulates a "Hayes- Compatible" modem,
meaning it accepts "AT" commands.  You can use KEGS to connect to free
telnet-BBSs, or run a BBS program on KEGS and become a telnet BBS yourself.

The two main AT commands are: ATDT for dialing out, and ATA for receiving
calls.  To dial out, enter "ATDThostname", or for example,
"ATDTboycot.no-ip.com" (which is down at the moment, unfortunately).
You can also enter an IP address, like "ATDT127.0.0.1".  On a Mac, to
create a telnet server to allow telnet connections (do not use over the
internet, but on a private network behind a firewall, this should be
fine), in a Terminal window type: "sudo /usr/libexec/telnetd -debug".
You must then enable telnet on port 23 through your Mac OS X Firewall in
the System Preferences->Sharing->Firewall page (just add port 23 as
open--you'll need to use the "New..." button and then select Other for
Port Name, and enter Port Number as 23). Then from KEGS in a
communications program, do "ATDT127.0.0.1", and then log-in to your Mac.

KEGS also accepts incoming "calls".  Start KEGS, and initialize the
Virtual Modem with some AT command (ATZ resets all state, and is a useful
start).  KEGS now has a socket port open, 6502 for slot 2, which you
can connect to using any telnet program.  In a Terminal window, then
type "telnet 127.0.0.1 6502" and you will connect to KEGS.  The Virtual
Modem then starts printing "RING" every 2 seconds until you answer with
"ATA".  You are now connected.  I have not tried BBS programs, but have
made connections with ProTERM.

On Windows XP SP2, when KEGS tries to open this incoming socket, you'll
need to enable it and click Unblock to the dialog that Windows pops up.
If you do not want incoming connections, you can block it instead.

Once connected, you can go back to talking to the Virtual Modem by
pressing + three times quickly (+++), and then not type anything for a second.
This goes back to the AT-command mode.  You can now "ATH" to hang up, or
"ATO" to go back online.

On Windows, the socket code is very preliminary and there are problems
receiving connections.

KEGS also supports an older, simpler socket interface, which it defaults
to using on slot 1.  In KEGS, from APPLESOFT, if you PR#1, all output will
then be sent to socket port 6501.  You can see it by connecting to the
port using telnet.  In another terminal window, do: "telnet localhost 6501"
and then you will see all the output going to the "printer".

Under APPLESOFT, you can PR#1 and IN#1.  This gets input from the
socket also.  You can type in the telnet window, it will be sent on
to the emulated IIgs.  You may want to go to the F4 Config Panel and set
"mask off high bit" for serial port accesses to make PR#1 work a little nicer.

You can "print" from BASIC by using something like PR#1 in KEGS and
"telnet localhost 6501 | tee file.out" in another window.


KEGS status area:
----------------

The status area is updated once each second.  It displays info I am
(or was at some time) interested in seeing.

Line 1: (Emulation speed info)
dcycs:  number of seconds since KEGS was started
sim MHz:  Effective speed of KEGS instruction emulation, not counting
		overhead for video or sound routines.
Eff MHz:  Above, but with overhead accounted for.  Eff MHz is the
		speed of an equivalent true Apple IIgs.  This is extremely
		accurate.
sec:	The number of real seconds that have passed during on of KEGS's
		emulated seconds. Should be 1.00 +/- .01.  Under 1
		means KEGS is running a bit fast, over 1 means KEGS is
		running slow.  When you force speed to 2.5MHz, if KEGS
		can't keep up, it extends sec, so you can see how slow
		it's really going here.
vol:	Apple IIgs main audio volume control, in hex, from 0-F.
pal:	Super-hires palette that is unavailable.  KEGS needs one palette
		for the standard Apple // graphics mode on an 8-bit display,
		and it grabs the least-used palette.  Defaults to 0xe.
		You can try changing it with F10.  If you change it to a
		palette that is not least used, KEGS changes it back in
		one second.  Any superhires lines using the unavailable
		palette will have their colors mapped into the
		closest-matching "lores" colors, to minimize visual
		impact.
Limit:	Prints which speed setting the user has requested: 1MHz, 2.8MHz,
		or Unlimited.

Line 2: (Video and X info)
xfer:  In hex, number of bytes transferred to the X screen per second.
xred_cs:	Percentage of Unix processor cycles that were spent in the X
		server (or other processes on the machine).
ch_in:	Percentage of Unix processor cycles spent checking for X input Events.
ref_l:	Percentage of Unix processor cycles spent scanning the Apple IIgs
		memory for changes to the current display screen memory,
		and copying those changes to internal XImage buffers.
ref_x:	Percentage of Unix processor cycles spent sending those XImage buffers
		to the X server.  Very similar to xred_cs.

Line 3: (Interpreter overhead)
Ints:  Number of Apple IIgs interrupts over the last second.
I/O:	Rate of I/O through the fake smartport interface (hard drives).
		Does not count 3.5" or 5.25" disk accesses.
BRK:	Number of BRKs over the last second.
COP:	Number of COPs over the last second.
Eng:	Number of calls to the main instruction interpreter loop in the
		last second.  All "interrupts" or other special behavior
		causes the main interpreter loop to exit.  A high call
		rate here indicates a lot of overhead.  12000-15000 is normal.
		20000+ indicates some sort of problem.
act:	Some instructions are handled by the main interpreter loop returning
		special status "actions" to main event loop.  This is the
		number over the last second.  Should be low.
hev:	This tracks HALT_EVENTs.  KEGS returns to the main loop to recalc
		effective speed whenever any speed-changing I/O location is
		touched.  See the code, mostly in moremem.c
esi:	This counts the number of superhires scan-line interrupts
		taken in the last second.
edi:	This counts the number of Ensoniq "special events" over the last
		second.  A sound that stops playing always causes a KEGS
		event, even if it doesn't cause a IIgs interrupt.

Line 4: (Ensoniq DOC info)
snd1,2,3,4:  Percentage of Unix processor cycles spent handling various
		sound activities.  snd1 is the total sum of all sound overhead.
st:	Percentage of Unix cycles spent starting new Ensoniq oscillators.
est:	Percentage of Unix cycles spent looking for 0 bytes in sounds.
x.yz:	This final number is the average number of oscillators playing
		over the last second.  Up to 4.00 is low overhead, over
		20.0 is high overhead.

Line 5: (Ensoniq DOC info)
snd_plays:	Number of calls to a routine called sound_play, which
		plays Ensoniq sounds.  Always called at least 60 times per sec.
doc_ev:	Number of Ensoniq (DOC) events in the last second.  A sound
		stopping is an event, but changing a parameter of a sound
		while it is playing is also an event.
st_snd:	 Number of sounds that were started in the last second.
snd_parms:	Number of times a sound parameter was changed while it
		was playing.

Line 6: (IWM info)
For each IWM device, this line displays the current track (and side for
3.5" disks).  If a disk is spinning, there will be an "*" next to the
track number.  Only updated once a second, so the disk arm moving may
appear to jump by several tracks.  "fast_disk_emul:1" shows that KEGS
is using less accurate, but faster, IWM emulation.  Press F7 to toggle
to accurate disk emulation.


Documentation To-Do:
-------------------

Describe the tracing and breakpoint debug features.
Describe the debug interface.
Describe how the code works.
Describe more of what's known to work.
Describe my changes to SPEEDTEST.

KEGS To-Do:
----------

Better serial port emulation (printing, comm)
Better nibblized images.
Fix the Ensoniq bugs to make sound more accurate.

-------------------

If you have any problems/questions/etc., just let me know.

Special thanks to Jeff Smoot of climbingwashington.com for letting me use
the picture of a keg for the Mac icon.

Kent Dickey
kadickey@alumni.princeton.edu


X Window (Linux) interface information:
--------------------------------------------

Every version of Linux is different.  Supporting this is very difficult
especially since I do not run Linux myself.

If KEGS fails to start, try the following options:

kegs -audio 0 -noshm

There may be a bug with drawing the border on x86 Linux with Shared Memory--
add the options "-noshm -skip 0" to fix this up (but lose some graphics
performance, sorry).  Try KEGS without these options first, but use
this as a workaround if necessary.

If you want the display to go somewhere different, make sure the shell
environment variable $DISPLAY is set, or give the command-line argument
"-display {foo}".

KEGS also forks off a subprocess to help handle the sound if audio is
active.  If KEGS crashes in a unusual way (a core dump, for instance),
you may have to manually kill the subprocess.  ("ps -ef| grep kegs;kill
xxxxx").

User geoff@gwlink.net adds some notes for mounting disks/floppies/CDs under
Solaris:

  To use a CDROM, insert the CD and let Volume Management mount it.
  Edit kegs_conf and use the filesystem that shows up in the "df -k"
  listing.  The volume name of the CDROM must be included.  For example,
  a CDROM in an IDE drive would look like this:

  /vol/dev/dsk/c1t0d0/ciscocd

  A CDROM in a SCSI drive would look like this:

  /vol/dev/dsk/c0t6d0/j1170_10804

To provide low-level ADB emulation, KEGS turns off Unix key repeat when the
focus is in the KEGS window.  It should be turned back on every time
the pointer leaves the KEGS window, but sometimes it doesn't.  Re-running
KEGS (and then quitting it quickly) should turn key-repeat back on,
or you can type 'xset r' in another terminal window.

Sometimes the converse is true--key repeat is "on" when the cursor is
in the KEGS window.  Moving the cursor out of the window and then
back in should solve it.  This is sometimes noticeable when running
Wolfenstein 3D GS.  I haven't spent much time debugging the problem.
I think it may be the X Server.

KEGS uses a private color-map for its X-window in 8-bit mode.  This
may cause colormap "flash" when your cursor enters the window.

KEGS details/troubleshooting
----------------------------

KEGS will work on all platforms with a 15/16-bit, 24-bit, or 32-bit
color display.  KEGS also supports an 8-bit display on X windows only.
On all platforms, it autodetects the color depth--no color switching
is necessary as long as you're at a supported depth.


Disk Image Details

Images loaded into slot 6 (drive 1 or 2) are assumed to be 140K
5.25" disks, which is usually have the extension ".dsk".  Images
loaded into slot 5 (drive 1 or 2) are assumed to be 800K disk images
and can be in any supported imahe format (including partitions, if
you have 800K partitions).  Images loaded into slot 7 (drives 1
through 32) can be in any format and can be any size up to 4GB.

KEGS boots s7d1 by default.  You can change this using the emulated IIgs
control panel, just like a real Apple IIgs.  KEGS emulates a IIgs with
two 5.25" drives in slot 6, two 3.5" drives in slot 5, and up to 32
"hard drives" in slot 7.  However, the current Configuration Panel only
lets you set through s7d11.  ProDOS 8 can access disks up to s7d8, but GSOS
has no limit, so it's best to put HFS images past s7d8 in order to leave
more slots for ProDOS images.

If you're trying to use a real host device (CD-ROM, or hard drive, or
floppy), you should make the permissions on the /dev/disk* files something
like (meaning, everyone should have read permission):

brw-r--r--  1 root  operator  14, 0 Jun 10 00:01 /dev/disk2

You can do this on a Mac with:

sudo chmod 644 /dev/disk2

DO NOT RUN KEGS AS ROOT.  It is not designed for this and it's almost certain
problems will ensue.

