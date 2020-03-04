#ifndef MESSAGE_H
#define MESSAGE_H

#include <stio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct {
    char type;
} PingMsg;

#endif  // MESSAGE_H