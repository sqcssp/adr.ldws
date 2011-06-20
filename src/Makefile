include Makefile.mk

CC = $(CROSS)gcc 
CPP = $(CROSS)g++
LD = $(CROSS)ld
STRIP = $(STRIP)strip

STRIPTOOL = $(CROSS)strip
STRIP = $(STRIPTOOL) --remove-section=.note --remove-section=.comment $(NAME)

AR = $(CROSS)ar


CFLAGS = -pthread -Wall -I./include -I./../lib/include
CFLAGS += -O3
#CFLAGS += -g2
#CFLAGS += -static
CFLAGS += -DIVAN_DEBUG
#CFLAGS += -static
CFLAGS += -DSINGLE_8612
#CFLAGS += -Wl,-elf2flt=-s10240
CFLAGS += -DNO_DP -DNDEBUG

CFLAGS += -I./include/ipcam   


SQFLAGS = -Wall
SQFLAGS += -DTARGET_SQ

LDFLAGS = -lm -lsq8612 -L./

LIBS :=


DEPEND_FILES = 	
	
PROG = adr.8612


all: test $(PROG)

$(PROG): $(DEPEND_FILES)
	$(CC) -o $@ $(CFLAGS) $(DEPEND_FILES) $(LDFLAGS) $(LIBS)
	$(STRIP) -s $@

.c.o:
	$(CC) -c -o $@ $(CFLAGS) $<

test :
	cd ./test; $(MAKE)

clean :
	rm -rf $(DEPEND_FILES) $(PROG) core

