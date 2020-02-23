#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("usage: %s [file_name]\n", argv[0]);
    exit(1);
  }

  struct stat file_st;
  int ret = stat(argv[1], &file_st);
  if (ret != 0) {
    perror("stat");
    exit(1);
  }

  printf("st_dev = %lld\n", file_st.st_dev);
  printf("st_ino = %ld\n", file_st.st_ino);
  printf("st_mode = %d\n", file_st.st_mode);
  printf("st_nlink = %d\n", file_st.st_nlink);
  printf("st_uid = %d\n", file_st.st_uid);
  printf("st_gid  = %d\n", file_st.st_gid);
  printf("st_size = %ld\n", file_st.st_size);
  printf("st_atime = %ld\n", file_st.st_atime);
  printf("st_mtime = %ld\n", file_st.st_mtime);
  printf("st_ctime = %ld\n", file_st.st_ctime);

  printf("st_blksize = %lld\n", file_st.st_blksize);

  exit(0);
}