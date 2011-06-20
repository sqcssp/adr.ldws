include ../../Makefile.mk

OBJECTS=	ldws.o

SRCS = $(OBJECTS:.o=.c)

DEPENDENCY=	ldws.c


CC = $(CROSS)gcc
AR = $(CROSS)ar

INC = -I../../include

TARGET = libldws.a

all : $(OBJECTS) $(TARGET)

.c.o :
	$(CC) $(INC) $(CFLAGS) $<


$(TARGET) : $(OBJECTS)
	$(AR) rcs $(TARGET) $(OBJECTS)

clean :
	rm -rf $(OBJECTS) $(TARGET)

ldws.o : $(DEPENDENCY) ldws.c
