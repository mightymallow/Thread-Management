# Makefile for building multi-file application
# by Brian Fraser

# final executable to build:
TARGET= candykids
# .o files to link in (for all .c files)
OBJS = candykids.o bbuff.o stats.o


CFLAGS = -Wall -g -std=c99 -Werror -pthread -lrt -D_POSIX_C_SOURCE=199309L
CC_C = gcc

all: clean $(TARGET)

run:
	echo *********************************
	./candykids 10 10 10
	
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --num-callers=20 ./candykids 10 1 10

%.o : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS)
	$(CC_C) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
