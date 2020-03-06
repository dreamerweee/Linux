CC = gcc
CFLAG = -std=gnu99

SOURCEDIR = .
SOURCES = $(wildcard $(SOURCEDIR)/*.c)
TARGETS = $(SOURCES:.c=)

all : $(TARGETS)

$(TARGETS): %:%.c
	$(CC) $(CFLAG) $^ -o $@

clean:
	rm -f $(TARGETS)