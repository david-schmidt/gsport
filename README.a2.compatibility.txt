# $Id: README.a2.compatibility,v 1.2 2004/10/18 18:17:21 kentd Exp $

Flobynoid: Must disable Fast Disk Emul (hit F7 to toggle it off) since
	game's loader relies on the sector order on the disk (reads 8
	sectors from the start without checking headers, assumes every other
	physical sector is skipped due to decode delays).

Bard's Tale II GS: Doesn't recognize any save disk as a ProDOS disk.
	It's detecting a "ProDOS" disk by checking for a string on block
	0 at offset 0x15e.  GSOS on system 6 has moved the string to 0x162,
	so disks inited under GSOS will be detected as "Not a PRODOS disk".
	Just make a copy of the Bard's Tale disk image to another file and
	then mount that new image and remove all the files using the Finder.
	Then rename the volume and you have a working save disk.

Robotron 2084:
Robot Battle:
	These cracks use a "Fastloader" which is buggy.
	It tries to JMP $F3D4 and expects to hit an RTS soon.
	But on a IIgs it will access some illegal memory causing a code
	yellow.  You can just ignore the code yellow.

Beyond Castle Wolfenstein:  Make sure your disk is writeable (not compressed!)

Breakout: Has trouble loading from the cracked copy.
	From the BASIC prompt, do: "CALL -151" then "C083 N C083" then
	"BLOAD INTBASIC" then run breakout.  Then it runs fine.

Burgertime:  This is a bad crack.  Loader starts the game by writing
	the game entry point into $0036/$0037, and then executing a BRK
	instruction.  The BRK handler on an old Apple II will try to write
	out a message by calling through $0036/$0037, and this will start
	the game.  But on a IIgs, the ROM sets the $0036/$0037 vectors
	back to the default, and so we crash into the monitor instead.
	Here's a memory fix and a disk-image fix: From the crack screen,
	press Shift-F6 (or middle mouse button) and in the KEGS debugger
	window enter: "1d0a:ea 6c 36 0" and then "g".  You can make
	this fix to the disk image using a sector editor and change
	Track $1E sector $09 offset $0A from "60 78 A9 03" to "EA 6C 36 00"
	and write it back.

Caverns of Callisto: Requires old C600 ROM in slot 6 (Slot 6==Your Card).

Championship Loderunner: Requires disk to be writeable or else it starts
	the level and then jumps immediately back to the title page.

Jeopardy: Disk must be writeable or else it displays "DISK ERROR" and
	then crashes into the monitor.


Drol: Needs slot 6 set to "Your Card" from the IIgs control panel
	(Ctrl-Cmd-ESC, then choose "Slots").
	I found Drol hard, so here are some cheats.  First, the standard cheat
	for Drol is to have infinite lives, this cheat is to edit the disk
	image:
	Track $0B, Sector $0A, byte $22 to EA EA
	Track $11, Sector $0A, byte $10 to EA EA
	Track $17, Sector $09, byte $B2 to EA EA
	I didn't create those cheats, I got it from textfiles.com.
	My cheats are for the monsters to never kill you--just run right
	through them.
	While playing Drol, press Shift-F6 (or middle mouse button) to
	enter the KEGS debugger, and then:
	0/f28:18 18			# Monsters' missiles won't kill you
	0/e05:90 0c			# Monsters won't kill you
	Continue emulation by type "g" and then enter.
	Other things, like the bird, axes, swords still kill you.
	To easily solve the third screen, move your man to the far right
	side on the top level, so that you are directly above the woman
	on the bottom row.  Fly into the air "A" and then get to the KEGS
	debugger, and type:
	0/c:4
	and then continue with "g".   Now press "Z" and you will go all
	the way down and land on the woman and end the level.  It's
	useful to have made the two above patches so that touching monsters
	won't kill you.
	Two more patches that only apply to level 3, and so most be made
	in memory each time you enter level 3:
	6cf3:18 18 18			# Axes won't kill you
	6f70:38 38			# Swords/arrows won't kill you
	The bird and the guy you can't kill will still kill you.  These
	cheats were enough to make the game easily playable.
	In the game, your death is indicated by setting location $001E to
	$FF.  Setting breakpoints there can let you find other cheats.

Moon Patrol: Crashes into the monitor after completing checkpoint E.
	To fix the Moon Patrol/Dung beetles version, from within KEGS:
		BLOAD MOON PATROL
		CALL -151
		1E15:EA
		919G
	and it will work fine.
	If you have the booting version that just has Moon Patrol on it,
	then from any point after the crack screen is shown, enter the
	KEGS debugger (Shift-F6 or middle moust button) and then enter:
		0/1e15:ea
		g
	and then it will play fine.
	The bug is that the code executes an instruction with opcode $02,
	which is a NOP on 6502, but is a COP instruction to 65816.  The
	COP instruction acts like BRK and will crash.  The patch just
	makes it a real NOP.

Microbe: Crashes upon booting.
	Code at $599E tries to pull a return address off of a location
	beneath the stack pointer and jump to it, but it doesn't add 1
	correctly so it jumps to $5917 when it meant to jump to $5918.
	On a IIgs, this causes a BRK to be executed and the game to crash.
	This can be patched in memory in two places:
		0/599e:ba ca 9a 7d 00 01 48 98 7d 01 01 9d 01 01 60
		0/6f1d:ba ca 9a 7d 00 01 48 98 7d 01 01 9d 01 01 60
	The original byte sequence for both locations is:
		00/599e: ba             TSX
		00/599f: 7d ff 00       ADC     $00ff,X
		00/59a2: 85 94          STA     $94
		00/59a4: 98             TYA
		00/59a5: 7d 00 01       ADC     $0100,X
		00/59a8: 85 95          STA     $95
		00/59aa: 6c 94 00       JMP     ($0094)
	You can also patch the code onto the disk image.  I found
	the $599E version on Track $05, Sector $06, Byte $9E.
	I found the $6F1D version on the image at Track $0C, Sector $00,
	at byte $1D.
