.PHONY: clean all

CC = gcc
CFLAG = -g -Wall -std=c99

SRCS=$(wildcard *.c)
PROGS=$(SRCS:.c=)

all: $(PROGS)
$(PROGS): %:%.c
	$(CC) $^ -o $@ $(CFLAG)

clean:
	rm -f $(PROGS)
