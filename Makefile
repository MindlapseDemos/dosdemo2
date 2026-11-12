!ifdef __UNIX__
dosobj = src/dos/audos.obj src/dos/djdpmi.obj src/dos/gfx.obj src/dos/keyb.obj &
	src/dos/logger.obj src/dos/main.obj src/dos/sball.obj src/dos/timer.obj &
	src/dos/vbe.obj src/dos/vga.obj src/dos/s3.obj src/dos/watdpmi.obj &
	src/dos/mouse.obj src/dos/pci.obj src/dos/util_s.obj src/dos/cpuid_s.obj &
	src/dos/cpuid.obj
3dobj = src/3dgfx/3dgfx.obj src/3dgfx/mesh.obj src/3dgfx/meshload.obj &
	src/3dgfx/polyclip.obj src/3dgfx/polyfill.obj src/3dgfx/scene.obj
rtobj = src/rt/rt.obj src/rt/rtgeom.obj
srcobj = src/cfgopt.obj src/console.obj src/demo.obj src/dynarr.obj &
	src/gfxutil.obj src/msurf2.obj src/noise.obj src/rbtree.obj src/screen.obj &
	src/tinyfps.obj src/treestor.obj src/image.obj src/ts_text.obj &
	src/util.obj src/darray.obj src/dseq.obj src/font.obj src/curve.obj &
	src/au_mp3.obj src/ts_bin.obj
scrobj = src/scr/temple.obj src/scr/raytrace.obj

incpath = -Isrc -Isrc/dos -Isrc/3dgfx -Isrc/rt -Ilibs -Ilibs/imago/src &
	-Ilibs/anim/src -Ilibs/midas -Ilibs/goat3d/include -Ilibs/mad
libpath = libpath libs/imago libpath libs/anim libpath libs/midas &
	libpath libs/goat3d libpath libs/assfile libpath libs/mad
!else

dosobj = src\dos\audos.obj src\dos\djdpmi.obj src\dos\gfx.obj src\dos\keyb.obj &
	src\dos\logger.obj src\dos\main.obj src\dos\sball.obj src\dos\timer.obj &
	src\dos\vbe.obj src\dos\vga.obj src\dos\s3.obj src\dos\watdpmi.obj &
	src\dos\mouse.obj src\dos\pci.obj src\dos\util_s.obj src\dos\cpuid_s.obj &
	src\dos\cpuid.obj
3dobj = src\3dgfx\3dgfx.obj src\3dgfx\mesh.obj src\3dgfx\meshload.obj &
	src\3dgfx\polyclip.obj src\3dgfx\polyfill.obj src\3dgfx\scene.obj
rtobj = src\rt\rt.obj src\rt\rtgeom.obj
srcobj = src\cfgopt.obj src\console.obj src\demo.obj src\dynarr.obj &
	src\gfxutil.obj src\msurf2.obj src\noise.obj src\rbtree.obj src\screen.obj &
	src\tinyfps.obj src\treestor.obj src\image.obj src\ts_text.obj &
	src\util.obj src\darray.obj src\dseq.obj src\font.obj src\curve.obj &
	src\au_mp3.obj src\ts_bin.obj
scrobj = src\scr\temple.obj src\scr\raytrace.obj

incpath = -Isrc -Isrc\dos -Isrc\3dgfx -Isrc\rt -Ilibs -Ilibs\imago\src &
	-Ilibs\anim\src -Ilibs\midas -Ilibs\goat3d\include -Ilibs\mad
libpath = libpath libs\imago libpath libs\anim libpath libs\midas &
	libpath libs\goat3d libpath libs\assfile libpath libs\mad
!endif

obj = $(dosobj) $(3dobj) $(rtobj) $(scrobj) $(srcobj) $(csprobj)
bin = demo.exe

!include watcfg.mk

def = -dM_PI=3.141592653589793 -dUSE_HLT $(cfg_def)
libs = imago.lib anim.lib goat3d.lib assfile.lib midas.lib mad.lib

AS = nasm
CC = wcc386
LD = wlink
ASFLAGS = -fobj
CFLAGS = $(cfg_dbg) $(cfg_opt) $(cfg_prof) $(def) -s -zq -bt=dos $(incpath)
LDFLAGS = option map option stack=16k $(libpath) library { $(libs) }

$(bin): cflags.occ $(obj) $(libs)
	%write objects.lnk $(obj)
	%write ldflags.lnk $(LDFLAGS)
	$(LD) debug all name $@ system dos4g file { @objects } @ldflags

.c: src;src/dos;src/3dgfx;src/rt;src/scr
.asm: src;src/dos;src/3dgfx;src/rt;src/scr
.lib: libs/imago;libs/anim;libs/midas;libs/goat3d;libs/assfile;libs/mad

cflags.occ: Makefile
	%write $@ $(CFLAGS)

!ifdef __UNIX__
src/dos/audos.obj: src/dos/audos.c
!else
src\dos\audos.obj: src\dos\audos.c
!endif
	$(CC) -fo=$@ @cflags.occ -zu $[*

.c.obj: .autodepend
	$(CC) -fo=$@ @cflags.occ $[*

.asm.obj:
	nasm -f obj -o $@ $[*.asm

!ifdef __UNIX__
clean: .symbolic
	rm -f $(obj)
	rm -f $(bin)
	rm -f cflags.occ *.lnk

imago.lib:
	cd libs/imago
	wmake
	cd ../..

anim.lib:
	cd libs/anim
	wmake
	cd ../..

goat3d.lib:
	cd libs/goat3d
	wmake
	cd ../..

assfile.lib:
	cd libs/assfile
	wmake
	cd ../..

mad.lib:
	cd libs/mad
	wmake
	cd ../..

cleanlibs: .symbolic
	cd libs/imago
	wmake clean
	cd ../anim
	wmake clean
	cd ../goat3d
	wmake clean
	cd ../assfile
	wmake clean
	cd ../mad
	wmake clean
!else
clean: .symbolic
	del src\*.obj
	del src\dos\*.obj
	del src\3dgfx\*.obj
	del src\rt\*.obj
	del src\scr\*.obj
	del *.lnk
	del cflags.occ
	del $(bin)

imago.lib:
	cd libs\imago
	wmake
	cd ..\..

anim.lib:
	cd libs\anim
	wmake
	cd ..\..

goat3d.lib:
	cd libs\goat3d
	wmake
	cd ..\..

assfile.lib:
	cd libs\assfile
	wmake
	cd ..\..

mad.lib:
	cd libs\mad
	wmake
	cd ..\..

cleanlibs: .symbolic
	cd libs\imago
	wmake clean
	cd ..\anim
	wmake clean
	cd ..\goat3d
	wmake clean
	cd ..\assfile
	wmake clean
	cd ..\mad
	wmake clean
!endif
