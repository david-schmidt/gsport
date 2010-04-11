
WIN32 port of KEGS (KEGSWIN)
----------------------------

There is a different port of KEGS by Chea Chee Keong (akilgard) called
KEGS32.  You can get it from http://www.geocities.com/akilgard/kegs32.
This port is leveraged from KEGS32, but mostly a rewrite (perhaps for
the worse).  I am grateful for Chea for doing the windows port since I
did not know any Windows programming.

This port is alpha quality.  Don't expect much.

This is a bare-bones Win32 port.  It was compiled with Mingw2.0,
which you can download at: http://www.mingw.org/.  I also had
previously installed cygwin at http://www.cygwin.com/.  Installing
these two beasts is a bit of a pain, so I'll eventually write up
directions (I hope).

Sound works, the mouse works, and a joystick might work (ported
from KEGS32).  The user-interface is just like every other KEGS
version (i.e., bad), so you can just read the standard README file.

Only tested on a 32-bit graphics display, although I think 16-bit and
24-bit will work.  8-bit definitely does not work.  There are many
other bugs I just haven't had time to list yet.

Usage:
-----

Like most other KEGS versions, KEGSWIN must be run from a terminal
window (command.com is fine).  Just type "KEGSWIN" in the directory
you installed/compiled it in.  You need to have a ROM file (named
ROM, ROM.01, or ROM.03) and a kegs_conf in the same directory (or
read the README--these files are searched for in various places).

To quit, either click the close box, or force quit the application.
You can also middle-click (if you have a 3-button mouse) or
Shift-F6 to get the debugger in the terminal window, and then type "q".

Compile directions
------------------

In order to compile,

1) cd into the src/ directory
2) rm vars
3) ln -s vars_win32 vars
3) ./make_win


You can contact me at kadickey@alumni.princeton.edu

