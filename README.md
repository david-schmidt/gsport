# GSport: an Apple IIgs Emulator 

GSport is a portable (in the programming sense) Apple IIgs emulator, based on the KEGS cross-platform IIgs emulator by Kent Dickey. The base emulator builds and runs on all of the same platforms that KEGS did, and the new capabilities are being integrated as contributors have time and interest.  Full documentation is available at the GSport website [here.](https://david-schmidt.github.io/gsport/)

[![alt](https://raw.githubusercontent.com/david-schmidt/gsport/master/doc/web/src/site/resources/images/gsport.png)](https://david-schmidt.github.io/gsport/) [![Download Link](https://raw.githubusercontent.com/david-schmidt/gsport/master/doc/web/src/site/resources/images/download.png)](https://github.com/david-schmidt/gsport/releases)

## Now with network support on Linux!

The GSPort project has been updated to support building for modern 64-bit Linux systems, including support for
networking via the Uthernet emulation!  This has only been tested on a Ubuntu 18.04 machine and may require some
tweaking, but the following steps have worked:

* Make sure multiarch support is enabled
* Download the i386 libraries, especially libX11, libXext, and libpcap (e.g. `sudo apt install libpcap-dev:i386`, etc)
* `cd src`, `ln -s vars_x86linux vars`, and `make`, and you're on your way

One important note is that the Uthernet emulation relies on sending layer 2 frames in the raw and the use of
promiscuous mode to discover frames addressed to the MAC of the emulated Uthernet card.  Because of this *wifi is not
supported*!  I repeat **wifi is not supported**!  It's highly encouraged you start out running this on a machine with
only one active interface, and that interface should be wired Ethernet.

## Project Goals

The main goal for GSport is to provide a free, open ecosystem for the continuation of cross-platform development of IIgs emulation.

Some interesting advances that initially spawned this project are Uthernet and printer support. Now that those objectives are met, we will move on to the next objectives. Do you have something that you wish a GS emulator did?  Dive in and contribute!
