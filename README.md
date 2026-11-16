DOS demo 2 / Mindlapse
======================
Possible next DOS demo project. We'll see how it goes...

Data files
----------
Data files are in subversion:

    svn co svn://mutantstargoat.com/datadirs/dosdemo2 data

License
-------
Copyright (C) 2026 John Tsiombikas <nuclear@mutantstargoat.com>

This demo is free software. Feel free to run, modify, and/or distribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

DOS build
---------
For the DOS build we're using MIDAS for audio output. The MIDAS library comes
pre-compiled in the `data/bin` directory in both DJGPP and Watcom flavors.

### DJGPP
First time copy the MIDAS library `libmidas.a` from the `data/bin`
directory into `libs/midas`.

Run `make -f Makefile.dj` to build the demo. The `djdemo.exe` executable will be
produced.

### Watcom
First time copy the MIDAS library `midas.lib` from `data/bin` into the
`libs/midas` directory.

Run `wmake` to build the demo. The `demo.exe` executable will be produced.
