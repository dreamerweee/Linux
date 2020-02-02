all: cpu mem

BUILD = ./build

cpu: cpu.c common.h
	gcc -o ${BUILD}/cpu cpu.c -Wall

mem: mem.c common.h
	gcc -o ${BUILD}/mem mem.c -Wall

# threads: threads.c

clean:
	rm -f ${BUILD}/cpu ${BUILD}/mem