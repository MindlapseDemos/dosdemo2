include ../libs.mk

src = $(wildcard *.c)
obj = $(src:.c=.o)
alib = libmad.a

def = -DFPM_DEFAULT -DHAVE_CONFIG_H

ifeq ($(sys), android-arm64)
	obj = $(src:.c=.arm64.o)
	alib = ../android64/libmad.a
	pic = -fPIC
endif
ifeq ($(sys), android-arm32)
	obj = $(src:.c=.arm32.o)
	alib = ../android32/libmad.a
	pic = -fPIC
endif

CFLAGS = $(arch) -g -O3 $(def) $(pic)

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.arm64.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

%.arm32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
