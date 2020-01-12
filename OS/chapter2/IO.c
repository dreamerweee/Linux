#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

int main(int argc, char *argv[])
{
  int fd = open("/tmp/file", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  assert(fd >= 0);
  char buffer[20];
  sprintf(buffer, "hello world\n");
  int cnt = write(fd, buffer, strlen(buffer));
  assert(cnt == (strlen(buffer)));
  fsync(fd);
  close(fd);
  return 0;
}