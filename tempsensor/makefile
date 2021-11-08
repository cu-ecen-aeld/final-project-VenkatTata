ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: i2ctemp

i2ctemp.o: i2ctemp.c
	$(CC) $(CFLAGS) -c i2ctemp.c

clean:
	rm -f *.o
	rm -f i2ctemp
