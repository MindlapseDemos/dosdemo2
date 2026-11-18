src = $(wildcard *.c)
obj = $(src:.c=.emo)
alib = assfile.ema

CC = emcc
AR = emar
CFLAGS = -O3

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

%.emo: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -f $(obj) $(alib)
