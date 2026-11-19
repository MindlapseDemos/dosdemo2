src = $(wildcard src/*.c) $(wildcard src/3dgfx/*.c) $(wildcard src/rt/*.c) \
	  $(wildcard src/scr/*.c) $(wildcard src/sdl/*.c)
obj = $(src:.c=.emo)
bin = demo.html

inc = -Isrc -Isrc/3dgfx -Isrc/rt -Isrc/scr -Isrc/utils -Isrc/sdl -Ilibs \
	  -Ilibs/imago/src -Ilibs/anim/src -Ilibs/assfile -Ilibs/mad -Ilibs/goat3d/include
def = -DNO_ASM
warn = -pedantic -Wall -Wno-unused-variable -Wno-unused-function -Wno-address
opt = -O3 -ffast-math
dbg = -g3

CC = emcc
CFLAGS = $(warn) $(opt) $(def) -fno-pie -fno-strict-aliasing $(dbg) $(inc) -sUSE_SDL
LDFLAGS = libs/imago/imago.ema libs/anim/anim.ema libs/assfile/assfile.ema \
		  libs/goat3d/goat3d.ema libs/mad/mad.ema -lSDL --preload-file data \
		  --preload-file demo.seq --exclude-file data/.svn \
		  -sINITIAL_MEMORY=67108864 -sUSE_SDL --shell-file demopage_shell.html
#		  -gsource-maps --profile-funcs -sSAFE_HEAP=1

$(bin): $(obj) imago anim assfile goat3d mad demopage_shell.html
	$(CC) -o $@ $(obj) $(LDFLAGS)

demopage_shell.html: tools/demotmpl.htm
	tools/fixhtml >$@

%.emo: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: libs
libs: imago anim assfile mad goat3d

.PHONY: imago
imago:
	$(MAKE) -C libs/imago -f Makefile.em

.PHONY: anim
anim:
	$(MAKE) -C libs/anim -f Makefile.em

.PHONY: assfile
assfile:
	$(MAKE) -C libs/assfile -f Makefile.em

.PHONY: mad
mad:
	$(MAKE) -C libs/mad -f Makefile.em

.PHONY: goat3d
goat3d:
	$(MAKE) -C libs/goat3d -f Makefile.em

.PHONY: cleanlibs
cleanlibs:
	$(MAKE) -C libs/imago clean -f Makefile.em
	$(MAKE) -C libs/anim clean -f Makefile.em
	$(MAKE) -C libs/assfile clean -f Makefile.em
	$(MAKE) -C libs/mad clean -f Makefile.em
	$(MAKE) -C libs/goat3d clean -f Makefile.em

.PHONY: clean
clean:
	rm -f $(obj) $(bin) demo.data demo.js demo.wasm

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(bin)
	rsync -vz -e ssh $(bin) goat.mutantstargoat.com:public_html/dosdemo2/index.html
	rsync -vz -e ssh demo.data demo.js demo.wasm goat.mutantstargoat.com:public_html/dosdemo2
