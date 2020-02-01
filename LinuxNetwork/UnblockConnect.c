
/*
* 超时连接函数
* 参数： 服务器IP地址、端口号、超时时间（毫秒）
* 函数成功时返回已处于连接状态的socket，失败返回-1
*/
int UnblockConnect(const char *ip, int port, int time)
{
	int conn_fd = Socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	InetPton(AF_INET, ip, &serv_addr.sin_addr);

	int old_fdopt = SetNonblocking(conn_fd);
	int ret = connect(conn_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (ret == 0) {
		/* 连接成功，恢复conn_fd的属性，并返回*/
		fcntl(conn_fd, F_SETFL, old_fdopt);
		return conn_fd;
	} else if (errno != EINPROGRESS) {
		return -1;
	}
	
	fd_set write_fds;
	FD_ZERO(&write_fds);
	FD_SET(conn_fd, &write_fds);
	int maxfdp1 = conn_fd + 1;

	struct timeval timeout;
	timeout.tv_sec = time;
	timeout.tv_usec = 0;

	ret = select(maxfdp1, NULL, &write_fds, NULL, &timeout);
	if (ret < 0) { // select超时或出错，返回-1
		Close(conn_fd);
		return -1;
	}
	if (!FD_ISSET(conn_fd, &write_fds)) {
		Close(conn_fd);
		return -1;
	}

	int error = 0;
	socklen_t len = sizeof(error);
	// 调用getsockopt来获取并清除conn_fd上的错误
	if (getsockopt(conn_fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
		Close(conn_fd);
		return -1;
	}
	// 错误号不为0表示连接出错
	if (error != 0) {
		Close(conn_fd);
		return -1;
	}

	// 连接成功
	fcntl(conn_fd, F_SETFL, old_fdopt);
	return conn_fd;
}
