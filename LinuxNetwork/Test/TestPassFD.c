#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>

static const int gsc_control_len = CMSG_LEN(sizeof(int));

void SendFD(int fd, int fd_to_send)
{
	struct iovec iov[1];
	struct msghdr msg;
	char buf[0];
	iov[0].iov_base = buf;
	iov[0].iov_len = 1;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	cmsghdr cm;
	cm.cmsg_len = gsc_control_len;
	cm.cmsg_level = SOL_SOCKET;
	cm.cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(&cm) = fd_to_send;
	msg.msg_control = &cm;
	msg.msg_controllen = gsc_control_len;

	sendmsg(fd, &msg, 0);
}

int RecvFD(int fd)
{
	struct iovec iov[1];
	struct msghdr msg;
	char buf[0];

	iov[0].iov_base = buf;
	iov[0].iov_len = 1;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	cmsghdr cm;
	msg.msg_control = &cm;
	msg.msg_controllen = gsc_control_len;

	recvmsg(fd, &msg, 0);

	int fd_to_read = *(int *)CMSG_DATA(&cm);
	return fd_to_read;
}