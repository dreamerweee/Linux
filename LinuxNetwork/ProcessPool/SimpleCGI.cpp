#include "ProcessPool.h"

// 用于处理客户CGI请求的类，它可以作为ProcessPool类的模板参数
class CgiConn {
public:
	CgiConn() { }
	~CgiConn() { }

	// 初始化客户连接，清空读缓冲区
	void Init(int epoll_fd, int conn_fd, const sockaddr_in &cli_addr)
	{
		m_epollfd = epoll_fd;
		m_sockfd = conn_fd;
		m_addr = cli_addr;
		memset(m_buffer, '\0', BUFFER_SIZE);
		m_read_idx = 0;
	}

	void Process()
	{
		int idx = 0;
		int ret = -1;

		while (true) {
			idx = m_read_idx;
			ret = recv(m_sockfd, m_buffer + idx, BUFFER_SIZE - idx - 1, 0);
			if (ret < 0) {
				if (errno != EAGAIN) {
					RemoveFD(m_epollfd, m_sockfd);
				}
				break;
			} else if (ret == 0) {
				// 对端关闭连接，则服务器也关闭连接
				RemoveFD(m_epollfd, m_sockfd);
				break;
			} else {
				m_read_idx += ret;
				printf("user content is: %s\n", m_buffer);
				// 如果遇到字符"\r\n"，则开始处理客户请求
				for (; idx < m_read_idx; ++idx) {
					if ((idx >= 1) && (m_buffer[idx-1] == '\r') && (m_buffer[idx] == '\n')) {
						break;
					}
				}
				// 如果没有遇到字符"\r\n"，组需要读取更多客户数据
				if (idx == m_read_idx) {
					continue;
				}
				m_buffer[idx - 1] = '\0';

				char *file_name = m_buffer;
				// 判断客户要运行的CGI程序是否存在
				if (access(file_name, F_OK) == -1) {
					RemoveFD(m_epollfd, m_sockfd);
					break;
				}
				// 创建子进程来执行CGI程序
				ret = fork();
				if (ret == -1) {
					RemoveFD(m_epollfd, m_sockfd);
					break;
				} else if (ret > 0) {
					RemoveFD(m_epollfd, m_sockfd);
					break;
				} else {
					Close(STDOUT_FILENO);
					dup(m_sockfd);
					execl(m_buffer, m_buffer, 0);
					exit(0);
				}
			}
		}
	}

private:
	static const int BUFFER_SIZE = 1024;  // 读缓冲区的大小
	static int m_epollfd;
	int m_sockfd;
	struct sockaddr_in m_addr;
	char m_buffer[BUFFER_SIZE];
	int m_read_idx;  // 标记读缓冲中已经读入的客户数据的最后一个字节的下一个位置
};

int CgiConn::m_epollfd = -1;

int main(int argc, char *argv[])
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	ProcessPool<CgiConn> *pool = ProcessPool<CgiConn>::Create(listen_fd);
	if (pool) {
		pool->Run();
		delete pool;
	}
	Close(listen_fd);
	return 0;
}