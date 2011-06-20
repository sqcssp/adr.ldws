include ../Makefile.mk

OBJECTS=adr.o	\

SRCS = $(OBJECTS:.o=.c)

DEPENDENCY=	adr.c	\


CC = $(CROSS)gcc
AR = $(CROSS)ar

INC = -I../include -I../../lib/include 

LDFLAGS = -pthread -lsq8612 -lldws -L../

PROG = adr.ldws


all: $(PROG)

adr.ldws: adr.o
	$(CC) -o $@ $< $(LDFLAGS)

adr.o: adr.c
	$(CC) $(INC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROG)

