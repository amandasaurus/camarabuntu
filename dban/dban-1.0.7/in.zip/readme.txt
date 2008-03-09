Darik's Boot and Nuke (dban-1.0.7_i386)
http://www.dban.org/

README.TXT Contents
-------------------

1.0  About Darik's Boot and Nuke

2.0  Installing DBAN
2.1  Creating the DBAN boot media on Microsoft Windows
2.2  Creating the DBAN CD-R disc on Microsoft Windows
2.3  Creating the DBAN floppy disk on Linux
2.4  Creating the DBAN CD-R disc on Linux

3.0  Using DBAN
3.1  Getting Help and Support

4.0  Configuring DBAN
4.1  Automatic Wiping
4.2  SYSLINUX.CFG

5.1  Free Updates
5.2  Contact Information

6.0  Advanced Features
6.1  Logging
6.2  PRNG Seeding

7.0  Legal



1.0  About Darik's Boot and Nuke
--------------------------------

Darik's Boot and Nuke ("DBAN") is a self-contained boot floppy that securely
wipes the hard disks of most computers. DBAN will automatically and completely
delete the contents of any hard disk that it can detect, which makes it an
appropriate utility for bulk or emergency data destruction.


2.0  Installing DBAN
--------------------


2.1  Creating the DBAN boot media on Microsoft Windows 
-------------------------------------------------------

Double-click the 'dban-1.0.7_i386.exe' program to install DBAN to a
floppy disk or USB flash device.

You must use an account with full hardware access to create the DBAN boot
media.  This means that you must be a member of the administrators group or
have similar privileges on your Microsoft Windows computer.  Virus scanners
and domain policies can prevent you from creating the DBAN boot media.

The USB booting capabilities of many computers are incomplete or broken.  Most
computers capable of booting from a USB device require that it report a
removable media type, and that it be unpartitioned and smaller than two
gigabytes (so that the BIOS can boot it like a floppy disk).

If the drive letter of your USB device does not appear in WinImage drive list,
then it is an unsupported media type.  In particular, most USB+IDE bridge
implementations are unrecognized, which means that a 2.5 inch hard disk in an
external USB enclosure is incompatible.


2.2  Creating the DBAN CD-R disc on Microsoft Windows
-----------------------------------------------------------

If you do not have a floppy drive, or if you wish to use a CD drive or
DVD drive instead, then get the "CDR/CDRW version" from the DBAN home page,
which is an ISO file.

The DBAN FAQ has a tutorial for burning the DBAN ISO on Microsoft Windows.

If you already have burning software installed on your computer, then
double-click the ISO file to burn DBAN to blank CD-R or CD-RW media. Keep all
defaults for mode and emulation if you burn the ISO file with a product like
Nero Burning ROM.

If nothing happens when you double-click the DBAN ISO file, or if the "Open
With" dialog that asks you to "choose a program" starts, then you do not have
burning software installed that understands the ISO format.

Note that CD-ROM drives manufactured before 1998 often have problems reading
CD-R media, and will sometimes damage it.  Try to use a DBAN floppy disk on
older computers.

Similarly, CD-RW media is often more problematic than CD-R media.  Try to use
CD-R media with DBAN, especially on older computers.


2.3  Creating the DBAN floppy disk on Linux
-------------------------------------------

# unzip dban-1.0.7_i386.exe
# dd if=dban-1.0.7_i386.ima of=/dev/floppy bs=1024

The DBAN EXE file is a regular zip file with the WinImage SFX module prepended.
InfoZip, which is bundled with most Linux distributions, can unpack the DBAN
EXE file.  Ignore any offset warnings.

If /dev/floppy does not exist on your computer, then try /dev/fd0 or
/dev/floppy/0 instead.  This command also works on BSD systems.


2.4  Creating the DBAN CD-R/CD-RW disc on Linux
-----------------------------------------------

# cdrecord dban-1.0.7_i386.iso

Ensure that the /etc/defaults/cdrecord file is properly configured for your
computer.


3.0  Using DBAN
---------------

Boot the computer with the DBAN media like you would boot a DOS startup disk or
like you would reinstall an operating system.  DBAN cannot be started from
within Microsoft Windows.

Doing this often requires changing the "boot order" of the computer so that
DBAN is started before the operating system on the hard disk is started.

Run the built-in BIOS setup program and put the floppy drive and the CD-ROM
drive first in the boot order. Do this by pushing <ESC>, <DEL>, <F1>, or <F2>
when you power-on the computer. The correct button and menu changes vary
between computers. Watch for a prompt when you power-on the computer or
consult the motherboard documentation.


3.1  Getting Help and Support
-----------------------------

First, read the frequently asked questions at:

  http://www.dban.org/faq/

Second, search for an answer and then ask for help in the forum at:

  http://sourceforge.net/forum/forum.php?forum_id=208932

Third, send email to support@dban.org and ensure that you have "DBAN" somewhere
in the subject line.  

Include this information if you ask a question or make a support request:

  1. The DBAN release that you are trying to use. (eg: dban-1.0.7)
  2. The DBAN media that you are trying to use. (eg: floppy disk)
  3. The computer manufacturer and model information. (eg: Dell OptiPlex 170L)
  4. The exact message that you see. (eg: "DBAN failed with error 255.")
  5. Anything else that is unusual about your computer or pertinent to the
     situation.  (eg: "I am trying to hot swap my disks.") 

Contact us if you wish to purchase a support contract.

You are also welcome to call us directly if you have previously purchased a
DBAN enhancement or customization, or if you have made a donation or other
contribution to the DBAN project.


4.0  Configuring DBAN
---------------------


4.1  Automatic Wiping
---------------------

Enter "autonuke" at the boot prompt to automatically wipe all devices in the
computer without confirmation.


4.2  SYSLINUX.CFG
-----------------

The "A:\SYSLINUX.CFG" file is the DBAN configuration file.  You may change the
default behavior of DBAN by editing this file after you create the DBAN floppy
disk.  The SYSLINUX.CFG is self-documented.

The CDR/CDRW version of DBAN must be remastered to change the SYSLINUX.CFG file
that is embedded in the ISO file.  DBAN will fail if the ISO file is changed by
an editor like IsoBuster.


5.1  Free Updates
-----------------

If you wish to be automatically notified of DBAN updates, then visit:

  http://sourceforge.net/project/monitor_project.php?group_id=61951
  
A similar service is provided by:

  http://freshmeat.net/subscribe/31200/


5.2  Contact Information
------------------------

General inquiries: dajhorn@dban.org
Help and support:  support@dban.org

All e-mail messages must have "DBAN" in the subject line.  Messages without
"DBAN" in the subject line will be blocked by a spam filter.

Send the DBAN log file with all support requests and do not modify the log file
in any way. Please read the FAQ and search the forums prior to sending email.


6.0  Advanced Features
----------------------


6.1  PRNG Seeding 
-----------------

Seeding the PRNG is optional. It is not required for DBAN to work properly.

DBAN can load prng seeds from a floppy disk to improve the quality of
operations that use random data. DBAN will check A:\DBANSEED for seed files
when it starts. Seed files must be exactly 512 bytes in size, but they can
have any name.

(The acronym "prng" means "pseudo random number generator".)

If you are using a Linux system, then run this command to make seed files:

# dd if=/dev/urandom of=MySeedFile bs=512 count=1

If /dev/urandom does not exist on your computer, then try /dev/random instead.

If this command produces files that are smaller than 512 bytes, then your
system entropy pool has been exhausted.  Your system will replenish its entropy
pool over time.

If you are using a Microsoft Windows system, then you will need 3rd party
software to make seed files.  The Cygwin posix environment for Microsoft
Windows can run the command given above.  Visit http://www.cygwin.com/ to
obtain the Cygwin product.


6.2  Logging
------------

DBAN will try to write a log file to A:\DBANLOG after it wipes the hard disks
in a computer. Log files are in the tarball format. The average log file will
be 5 kilobytes in size.

DBAN can boot from a USB device, but it cannot write log files to a USB device.

If you are using a unix system, then use this command to unpack the tarball:

  # tar -xzvf dban0000.tgz

If you are using a Microsoft Windows system, then use 7-Zip to unpack the TGZ
file.  Notepad will not display the text files in the tarball properly, so use
Wordpad instead.

  http://www.7-zip.org/


7.0  Legal
----------

This software is provided without any warranty; without even the implied
warranty of merchantability or fitness for a particular purpose. In no event
shall the software authors or contributors be liable for any damages arising
from the use of this software. This software is provided "as is".

All DBAN components are subject to copyright. Please see the 'licenses' folder
within the DBAN distribution package for additional information.


EOF
