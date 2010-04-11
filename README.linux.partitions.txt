[ This info provided by Mike Thomas <phoenyx@texas.net> ]
[ Updated 10/30/2003 by Kent: This file mentions editing "kegs.conf" to ]
[  mount images--this is now replaced by the built-in Configuration Panel. ]

Setup and configuration for x86 Linux:
--------------------------------------

KEGS is very easy to setup on your Linux box, but not foolproof.  First
you will need to decide where it will live.  When doing this you will
have to take into consideration any users of your machine.  It really
doesn't matter where it goes but it should have it's own directory and
any supplied sub-directories should be there.  You may decide to use the
/usr/local path where most systems recommend you install applications.
Again, this is entirely up to you.  On my system I have a separate
partition for KEGS and all the miscellaneous files I've accumulated for
it.  This makes it easy for me to reinstall the system should the need
arise.  Since I fool around with a large variety of software and OS's
this happens more often than it would for normal users.

Once you have put the files into the proper place you will need to
compile it.  You should not need to be 'root' to do this part.  The file
README.compile explains the steps required for this.  Basically all you
should need to do is set the vars link to point to the file
vars_x86linux.  You will want to check the file and make sure it is
accurate for your system.  On my Redhat 6.0 system the default compile
setup works fine.  I use the pentium GCC compiler instead of the
supplied EGCS since it seems to build better binaries.  I do not
recommend using optimization levels higher than 2.  Once you have
successfully built the binaries you will need to copy them to the KEGS
directory.  At a minimum copy the file kegs and to_pro.

Ok, now that you have the binaries you're almost ready.  You will need a
copy of the IIgs rom placed in the KEGS directory.  It should be named
ROM.  You will also need some disk images.  This is the hardest part.
You will need to create an HD image to work with.  Kent mentions an easy
way in his readme.  From the shell type this:

echo "testfile" > testfile
to_pro -16384 testfile

If you're using bash try this:

echo "testfile" > testfile
./to_pro -16384 testfile

This should create a 16 megabyte HD image. This image will NOT be properly 
formatted to boot a system. The block zero is not properly setup. There is no 
easy way to fix this with the current KEGS/Linux system. There seems to be a 
problem formating HD files for Prodos using KEGS. The system will easily erase 
them but this doesn't help you. One solution is to make the primary boot drive
use a disk partition. This is more involved and will be explained later. 
Another solution is to have access to the utility Block.Warden or some other 
P8 block utility. What you need to do is copy the boot blocks (blocks 0 and 1)
from any bootable disk to the HD image. With that done you can now install 
GS/OS.

Make sure you set the proper file permissions on files needed by KEGS. You 
will not be able to properly use it while logged on as root. KEGS uses the 
file permissions to test if it should write the image to disk or the memory 
image. As root, KEGS will never write the image since it thinks root
always has execute privilege. The main files which you will need read/write 
access to are bram.data.1 and disk_conf. I suggest you have read access to all
the other files in the directory.

Once you've got all the proper permissions set, log onto the system with your
normal account. Start up X and a shell and cd to the KEGS directory. Assuming
you have the disk images for GS/OS edit your disk_conf file so it looks 
similar to this:

# boot from install disk
s7d1 = /usr/local/fst/gsos1

# our HD image
# you should rename POOF1 file created with to_pro
# I use *.hdv to be compatible with other utilities
s7d2 = /usr/local/fst/boot.hdv

# other GSOS images
s7d3 = /usr/local/fst/gsos2
...

If you include all the GSOS images this way you will have a simple setup.
Execute KEGS. For now go with the simplest command line possible. Since the
newer versions support auto detect on video resolutions this should be the
command kegs by itself. Hopefully you will boot into the emulator. If so,
install GSOS and you're ready to go.


Sound
-----

Kent says sound doesn't work under Linux. This is partially true and much
depends on the sound system you have installed. I have been successful with
sound on both Soundblaster and Soundpro systems. For long compositions the
sound may break up but it is functional for games and system sounds.


System Video Setup
------------------

This is rather personal and based upon system hardware so I'll just give you my
thoughts and setup info on this. My normal X system is configured 
@ 1152x864 15bpp due to constraints in the X server for my video system. I
have custom configured it to boot into this mode and then I change to 800x600
by using the CTRL+ALT+(keypad)PLUS sequence when I use KEGS. This makes the
system much easier to read.


KEGS and disk partitions
------------------------

Kent mentions using partitions in his readme file but doesn't go into much
details. I suspect this is mostly for accessing CD-roms. But it also works
well for HFS and Prodos formatted partitions also. Linux can also handle HFS
partitions but not Prodos. To accomplish this you will need some free space on
your hard disk to create the partitions. You should not attempt this unless you
know what you are doing. You have been warned!

This task is not easy, nor is it supported by Kent. This was done and tested
on my own system so I know it works. You will need the HFS support utilities
for Linux. These are available on several Linux software sites. The primary
need for these utilities is to change an ext2fs partition to an HFS partition.
You can also use them to format the HFS volumes and copy files to and from
the partition. Newer versions of the Linux kernel support HFS file systems but
you will still need the utilities to create the original volume.

You must decide how you want to divide this partition. You can use it all for
HFS or you can create Prodos volumes and HFS volumes. There are pros and cons
for using Prodos partitions instead of files. The pros, it is a little faster
than using an HD file. It is a real Prodos partition, formatted by KEGS. The
cons, It must be backed up by using software on the emulator. You can't just
copy the HD file to another drive.

You must weigh these pros and cons and decide for yourself. Of course you
are not limited to using partitions. I have a mix of partitions and files
which works quite well. I like the P8 partitions for holding my boot system
and applications. I back them up with GSHK to an HFS volume which I can then
transfer to another drive if I need even more security.

If you decide to use the whole partition for HFS then all you need to do is
run the HFS utilities and setup the HFS volume. Read the documentation which
comes with the utility package and follow it faithfully.

If you want to use P8 and HFS partitions you have some more work to do. If
you have never worked with low level partitions or are worried about destroying
your HD then you should probably forget this. For this to work you will have
to change the partition table on your HD. This can and most likely will ruin
any data you already have on there. I can not state this enough. Back up any
important data on the hard drive! It is possible to change the partitions in
Linux and not destroy the system. I have done this on mine but I also used
the last defined partition to make the changes and designed the system for
change should it be necessary.

For those of you who know how to handle this, take the partition you have
decided to use for KEGS and divide it into at least one 32 meg partition.
More is better but you will have to use the emulator to back up any drives
setup this way since Linux can't access a Prodos partition (yet). I have setup
4 32 meg P8 partitions and several smaller HFS partitions of about 96 megs.
The reason I use smaller HFS partitions is because HFS isn't real efficient
with larger drives, but that's another story. The reason for the separate
HFS partitions is so Linux can mount the HFS volumes like any other file system.
I find this works quite easily for accessing files instead of using the HFS
utilities. Just my opinion though. For P8 utilities you will still need to
use the HFS utility and configure the drive as an HFS volume. This lets KEGS
read the partition when it loads the partition the first time. KEGS doesn't
like the Linux file system.

Ok, everybody got their partitions defined? You want to use the HFS tools
and setup all the partitions (P8 and HFS) as HFS volumes. Next you will have
to setup the HFS partitions. No, I'm not repeating myself. This is not the same
thing as the low level partitions. HFS volumes have their own partition table.
For our purposes the table will only hold one partition on each whole volume.
The utility will give you the block count when you setup the partitions,
write it down so you don't forget. After you have the volume partition setup
you can format the drive. Yeah I know you made a Prodos partition but it
doesn't hurt anything and KEGS will be able to read the partition when it
boots up.

Well, I think I covered everything needed to set them up. Now you will need to
edit the /etc/fstab file. Make sure there are no references to the original
partition. If you want to access the HFS volumes you will need to add them to
this file. You will also need to make sure that your Linux can understand the
HFS format. Most newer kernels will as long as you've compiled it into the
kernel or set it up as a module. KEGS doesn't need these entries to access
the volumes, they are just here for your convenience. In fact, if you don't
need Linux access I suggest you either leave them out or set them up as
NOAUTO and mount them only when needed. Unmount them when finished to avoid
any potential problems also. Do not give Linux access to any P8 partitions.
For one thing it can't recognize them and most likely will give you problems
when you boot the system. For safety's sake the only partition I have listed
in my /etc/fstab is a volume called transfer. You must set the filetype to HFS
so Linux doesn't complain about the partitions if you mount them.

Ok, all partitions are defined, the /etc/fstab is setup correct, now you need
to change the permissions on the device files associated with the partitions.
This allows you to access the files as a normal user. (Thanks Kent, guess I
got too involved and forgot it should be treated like the CD). You will need
to reboot to ensure the system sees the new partitions and has the correct
/dev/hd?# device files. If you setup the partitions with fdisk you should know
the correct hd info to select the files. For the sake of example let's assume
the device is HDB and the partitions numbers are 1,2,3. From the shell,

cd /dev
chmod 666 /dev/hdb1
chmod 666 /dev/hdb2
chmod 666 /dev/hdb3

After you start KEGS you can format the Prodos partitions. If you use the
method mentioned earlier for installing GS/OS you will want to quit the
installer and run the advanced disk utilities on the utilities disk, then
quit back to the installer program or reboot.

Now I know this all sounds like a lot of trouble but (IMHO) it's worth it. For
one thing, KEGS will format a Prodos partition without problems (unlike an HD
file image) which makes a great boot system. And with GS/OS 6.01 you can access
large HFS volumes for storage and GS applications. You can also download from
the net to the HFS volume (if it's mounted) and avoid the trouble of copying
files to an image with to_pro. Not to mention the fact that the newest GNO
works with HFS drives.

One more note, if you use HFS you will need to patch the HFS fst. There is a
one byte bug which mis-calculates HFS block sizes and WILL trash your HFS
drive. The patch is at several places on the net or ask someone in one of
the comp.sys.apple2 news groups.

Miscellanea
-----------

To ease access to the KEGS binary, make an executable script which contains
any command line parms you need. Then put this script somewhere in the path
so you can execute it from any shell. Depending on the desktop you use you
may want to setup links for editing the disk_conf file also. With GNO/ME you
can launch KEGS without the shell but I don't recommend this since you won't
know what happened when it dies. With KDE you can set up the launcher to use
a shell, this is much better but until you have your setup stable you will
want to use a regular terminal so you can keep track of what's going on. Like
GNO/ME, the KDE shell will close as soon as KEGS quits with an error.


Thanks
------

I hope this info helps you enjoy KEGS. Many thanks to Kent for creating this
fine emulator and for putting up with me bugging him with 'bug' reports. Many
of these weren't actually bugs but were my own fault due to lack of knowledge
about *nix systems. But Kent was prompt in fixing the ones which truly were
bugs. Thanks to him I can now play my favorite game, Questron 2 (gs version)
which requires a player disk in slot 5. I know no other emulator which can
actually play this game.

Mike Thomas

