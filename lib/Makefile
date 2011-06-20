include Makefile.mk

OBJECTS=./src/ldws/ldws.o 	\
	./src/ldws/libldws.a




SRCS = $(OBJECTS:.o=.c)

DEPENDENCY =./src/ldws/ldws.c

CC = $(CROSS)gcc
AR = $(CROSS)ar


INC = -I./include

TARGET = libsq8612.a

all : camera tvout ldws $(TARGET)

ldws :
	cd ./src/ldws; $(MAKE)


$(TARGET) : $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)

.c.o :
	$(CC) $(INC) $(CFLAGS) $<

clean :
	rm -rf $(OBJECTS) core

