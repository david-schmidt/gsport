
MAC OS X port of KEGS (KEGSMAC): http://kegs.sourceforge.net
-------------------------------------------------------------

There is a different port of KEGS to Mac OS X called KEGS-OSX.
You can get it from http://casags.net/.

This port is not leveraged from KEGS-OSX (which is based on SDL).

This is a Mac OS X Carbon port.  It will not work on Mac OS 9.

Everything pretty much works, but 8-bit color doesn't work.  Make sure your
Mac is set to Thousands or Millions of colors.

Usage:
-----

Like most other KEGS versions, KEGSMAC is usually run from a Terminal
window.  Just type "./KEGSMAC.app/Contents/MacOS/KEGSMAC" in the directory
you installed/compiled it in.  You need to have a ROM file (named
ROM, ROM.01, or ROM.03) and a config.kegs in the same directory or in your
home directory (read the README--these files are searched for in various
places).

KEGSMAC can also be run from the Finder, but if you do this, there
will be no debug window at all.  This is not well tested, yet.

To quit, either click the window close box, or select Quit from the menu.
You can also middle-click (if you have a 3-button mouse) or
Shift-F6 to get the debugger in the terminal window, and then type "q".

Compile directions
------------------

In order to compile,

1) cd into the src/ directory
2) copy vars_mac to vars
3) run make

You can contact me at kadickey@alumni.princeton.edu
