src = $(wildcard *.c)
obj = $(src:.c=.emo)
alib = mad.ema

def = -DFPM_DEFAULT -DHAVE_CONFIG_H

CC = emcc
AR = emar
CFLAGS = -O3 $(def)

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.emo: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
