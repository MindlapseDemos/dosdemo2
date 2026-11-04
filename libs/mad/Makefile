obj = bit.obj decoder.obj fixed.obj frame.obj huffman.obj layer12.obj &
	layer3.obj stream.obj synth.obj timer.obj version.obj

!ifdef __UNIX__
alib = ../../mad.lib
!else
alib = ..\..\mad.lib
!endif

def = -DHAVE_CONFIG_H -DFPM_DEFAULT

CC = wcc386
CFLAGS = $(cfg_dbg) $(cfg_opt) $(def) -zq -bt=dos

$(alib): $(obj)
	%write objects.lbc $(obj)
	wlib -b -n $@ @objects

.c.obj: .autodepend
	%write cflags.occ $(CFLAGS)
	$(CC) -fo=$@ @cflags.occ $[*

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(alib)
!else
clean: .symbolic
	del *.obj
	del objects.lbc
	del cflags.occ
	del $(alib)
!endif
