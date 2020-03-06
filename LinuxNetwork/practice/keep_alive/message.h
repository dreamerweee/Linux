#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct {
    int type;
} Message;

#define MSG_PING        1
#define MSG_PONG        2

#endif  // MESSAGE_H