all : write read

write : write.c common.c
	gcc -o write write.c common.c

read : read.c common.c
	gcc -o read read.c common.c

clean :
	rm read write