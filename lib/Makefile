include Makefile.mk

OBJECTS=




SRCS = $(OBJECTS:.o=.c)

DEPENDENCY =

CC = $(CROSS)gcc
AR = $(CROSS)ar


INC = -I./include

TARGET = libsq8612.a

all : ldws $(TARGET)

ldws :
	cd ./src/ldws; $(MAKE)


$(TARGET) : $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)

.c.o :
	$(CC) $(INC) $(CFLAGS) $<

clean :
	rm -rf $(OBJECTS) core
	rm -rf ./src/ldws/ldws.o ./src/ldws/libldws.a

