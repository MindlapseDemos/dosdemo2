include ../libs.mk

src = $(wildcard *.c)
obj = $(src:.c=.o)
alib = libminiaudio.a

ifeq ($(sys), android-arm64)
	obj = $(src:.c=.arm64.o)
	alib = ../android64/libminiaudio.a
	pic = -fPIC
endif
ifeq ($(sys), android-arm32)
	obj = $(src:.c=.arm32.o)
	alib = ../android32/libminiaudio.a
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
