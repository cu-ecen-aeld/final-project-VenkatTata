ifeq ($(CC),)
	CC = $(CROSS_COMPILE)gcc
endif
ifeq ($CFLAGS),)
	CFLAGS = -g -Wall -Werror
endif

ifeq ($(INCLUDES),)
	INCLUDES = -I/
endif

all: bttest

bttest.o: bttest.c
	$(CC) $(CFLAGS) -c bttest.c

bttest: bttest.o
	$(CC) $(CFLAGS) bttest.o -o bttest -lbluetooth
clean:
	rm -f *.o
	rm -f bttest
