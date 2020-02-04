all: cpu mem threads io

BUILD = ./build

cpu: cpu.c common.h
	gcc -o ${BUILD}/cpu cpu.c -Wall

mem: mem.c common.h
	gcc -o ${BUILD}/mem mem.c -Wall

threads: threads.c common_threads.h
	gcc -o ${BUILD}/threads threads.c -Wall -lpthread

io: io.c
	gcc -o ${BUILD}/io io.c -Wall

clean:
	rm -f ${BUILD}/cpu ${BUILD}/mem ${BUILD}/threads ${BUILD}/io