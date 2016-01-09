SRC=suim.c
OBJS=$(SRC:.c=.o)
PROG=suim.dll
CC=gcc
AFLAGS=
CFLAGS=-O3 -DWINDOWS
LDFLAGS=-shared

%.o: %.c
	$(CC) $(AFLAGS) $(CFLAGS) -o $@ -c $<

.PHONY : all
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(AFLAGS) $(LDFLAGS) $(OBJS) -o $@

.PHONY : clean
clean:
	rm $(OBJS) $(PROG)
