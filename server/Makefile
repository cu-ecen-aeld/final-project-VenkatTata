ifeq ($(CC),)
	CC = ${CROSS_COMPILE}gcc
endif

ifeq ($(CFLAGS),)
	CFLAGS = -g -Wall -Werror 
endif

ifeq ($(LDFLAGS),)
	LDFLAGS = -pthread -lrt
endif

all: aesdsocket

aesdsocket: aesdsocket.c
	$(CC) $^ -o $@ $(INCLUDES) -pthread -lrt -g -Wall -Werror

clean:
	rm -f aesdsocket *.o
	rm -f valgrind-out.txt
