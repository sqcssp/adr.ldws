include ../Makefile.mk

OBJECTS= adr.o config.o test_tvout_camera.o \

SRCS = $(OBJECTS:.o=.c)

DEPENDENCY= adr.c config.c test_tvout_camera.c \


CC = $(CROSS)gcc
AR = $(CROSS)ar

INC = -I../include -I../../lib/include 

LDFLAGS = -pthread -lsq8612 -lldws -L../

PROG = adr.ldws


all: $(PROG)
	
#adr.ldws: adr.o config.o
#	$(CC) -o $@ $< $(LDFLAGS)

adr.ldws: test_tvout_camera.o config.o
	$(CC) -o $@ $< $(LDFLAGS)

test_tvout_camera.o: test_tvout_camera.c
	$(CC) $(INC) $(CFLAGS) -c $<

adr.o: adr.c
	$(CC) $(INC) $(CFLAGS) -c $<
	
config.o: config.c
	$(CC) $(INC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS) $(PROG)

