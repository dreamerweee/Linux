#include "SocketPack.h"
#include <netinet/tcp.h>

// 来自Linux高性能服务器编程

#define BUFFER_SIZE 4096  // 读缓冲区大小

// 主机状态的两种可能状态，分别表示：当前正在分析请求行，当前正在分析头部字段
enum CHECK_STATE {
	CHECK_STATE_REQUESTLINE = 0,
	CHECK_STATE_HEADER
};

/*
* 从状态机的三种可能状态，即行的读取状态，
* 分别表示：读取到一个完整的行、行出错和行数据尚且不完整
*/
enum LINE_STATUS {
	LINE_OK = 0,
	LINE_BAD,
	LINE_OPEN
};

/*
* 服务器处理HTTP请求的结果：
*	NO_REQUEST 表示请求不完整，需要继续读取客户数据
*	GET_REQUEST 表示获得了一个完整的客户请求；
*	BAD_REQUEST 表示客户请求有语法错误；
*	FORBIDDEN_REQUEST 表示客户对资源没有足够的访问权限；
*	INTERNAL_ERROR 表示服务器内部错误；
*	CLOSED_CONNECTION 表示客户端已经关闭连接了。
*/
enum HTTP_CODE {
	NO_REQUEST,
	GET_REQUEST,
	BAD_REQUEST,
	FORBIDDEN_REQUEST,
	INTERNAL_ERROR,
	CLOSED_CONNECTION
};

/*
* 为了简化问题，我们没有给客户端发送一个完整的HTTP应答报文，
* 而只是根据服务器的处理结果发送如下成功或失败信息
*/
static const char *g_sz_ret[] = {"I get a correct result\n", "Something wrong\n"};

/* 从状态机，用于解析出一行内容*/
LINE_STATUS ParseLine(char *buffer, int &checked_idx, int &read_idx)
{
	char temp;
	/*
	 * checked_idx 指向buffer(应用程序的读缓冲区)中当前正在分析的字节，
	 * read_idx 指向buffer中客户数据的尾部的下一字节。
	 * buffer中第0~checked_idx字节都已分析完毕，第checked_idx~read_idx-1 字节由下面的循环挨个分析
	*/
	for (; checked_idx < read_idx; ++checked_idx) {
		// 获取当前要分析的字节
		temp = buffer[checked_idx];
		// 如果当前的字节是'\r'，即回车符，则说明可能读取到一个完整的行
		if (temp == '\r') {
			// 如果'\r'字符碰巧是目前buffer中的最后一个已经被读入的客户数据，那么这次分析没有读取到一个完整的行
			// 返回LINE_OPEN以表示还需继续读取客户数据才能进一步分析
			if ((checked_idx + 1) == read_idx) {
				return LINE_OPEN;
			} else if (buffer[checked_idx + 1] == '\n') { // 如果下一个字符是'\n'，则说明我们成功读取到一个完整的行
				buffer[checked_idx++] = '\0';
				buffer[checked_idx++] = '\0';
				return LINE_OK;
			}
			// 否则的话，说明客户发送的HTTP请求存在语法问题
			return LINE_BAD;
		} else if (temp == '\n') { // 如果是'\n',也说明可能读取到一个完整的行
			if ((checked_idx > 1) && buffer[checked_idx - 1] == '\r') {
				buffer[checked_idx - 1] = '\0';
				buffer[checked_idx++] = '\0';
				return LINE_OK;
			}
		}
		return LINE_BAD;
	}

	// 如果所有内容都分析完毕也没遇到'\r'字符，则返回LINE_OPEN，表示还需要继续读取客户数据才能进一步分析
	return LINE_OPEN;
}

// 分析请求行
HTTP_CODE ParseRequestline(char *temp, CHECK_STATE &check_state)
{
	char *url = strpbrk(temp, " \t");
	// 如果请求行中没有空白字符或"\t"字符，则HTTP请求必有问题
	if (!url) {
		return BAD_REQUEST;
	}
	*url++ = '\0';
	char *method = temp;
	if (strcasecmp(method, "GET") == 0) {  // 仅支持GET方法
		printf("The request method is GET\n");
	} else {
		return BAD_REQUEST;
	}

	url += strspn(url, " \t");
	char *version = strpbrk(url, " \t");
	if (!version) {
		return BAD_REQUEST;
	}
	*version++ = '\0';
	version += strspn(version, " \t");
	if (strcasecmp(version, "HTTP/1.1") != 0) { // 仅支持HTTP/1.1
		return BAD_REQUEST;
	}
	// 检查url是否合法
	if (strncasecmp(url, "http://", 7) == 0) {
		url += 7;
		url = strchr(url, '/');
	}
	if (!url || url[0] != '/') {
		return BAD_REQUEST;
	}
	printf("The request URL is: %s\n", url);
	// HTTP请求行处理完毕，状态转移到头部字段的分析
	check_state = CHECK_STATE_HEADER;
	return NO_REQUEST;
}

// 分析头部字段
HTTP_CODE ParseHeaders(char *temp)
{
	// 遇到一个空行，说明我们得到了一个正确的HTTP请求
	if (temp[0] == '\0') {
		return GET_REQUEST;
	} else if (strncasecmp(temp, "Host:", 5) == 0) { // 处理HOST头部字段
		temp += 5;
		temp += strspn(temp, " \t");
		printf("the request host is: %s\n", temp);
	} else { // 其他头部字段都不处理
		printf("I can't handle this header");
	}
	return NO_REQUEST;
}

// 分析HTTP请求的入口函数
HTTP_CODE ParseContent(char *buffer, int &checked_idx, CHECK_STATE &check_state, int &read_idx, int &start_line)
{
	LINE_STATUS line_status = LINE_OK;  // 记录当前行的读取状态
	HTTP_CODE ret_code = NO_REQUEST;    // 记录HTTP请求的处理结果
	// 主状态机，用于从buffer中取出所有完整的行
	while ((line_status = ParseLine(buffer, checked_idx, read_idx)) == LINE_OK) {
		char *temp = buffer + start_line;  // start_line是行在buffer中的起始位置
		start_line = checked_idx;
		// check_state记录主状态机当前的状态
		switch (check_state) {
			case CHECK_STATE_REQUESTLINE: // 第一个状态，分析请求行
			{
				ret_code = ParseRequestline(temp, check_state);
				if (ret_code == BAD_REQUEST) {
					return BAD_REQUEST;
				}
				break;
			}
			case CHECK_STATE_HEADER:  // 分析头部字段
			{
				ret_code  = ParseHeaders(temp);
				if (ret_code == NO_REQUEST) {
					return BAD_REQUEST;
				} else if (ret_code == GET_REQUEST) {
					return GET_REQUEST;
				}
				break;
			}
			default:
			{
				return INTERNAL_ERROR;
			}
		}
	}
	// 若没有读取到一个完整的行，则表示还需要继续读取客户数据才能进一步分析
	if (line_status == LINE_OPEN) {
		return NO_REQUEST;
	} else {
		return BAD_REQUEST;
	}
}

int main(int argc, char *argv[])
{
	int listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
	int on_reuseaddr = 1;
	printf("on_reuseaddr sizeof: %d\n", sizeof(on_reuseaddr));
	// setsockopt(listen_fd, SOL_SOCK, SO_REUSEADDR, &on_reuseaddr, sizeof(on_reuseaddr));
	struct sockaddr_in serv_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	Listen(listen_fd, 5);

	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	int conn_fd = Accept(listen_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);

	char buffer[BUFFER_SIZE + 1];  // 读缓冲区
	memset(buffer, '\0', BUFFER_SIZE + 1);
	int data_read = 0;
	int read_idx = 0; 		// 当前已经读取了多少字节的客户数据
	int checked_idx = 0; 	// 当前已经分析完了多少字节的客户数据
	int start_line = 0; 	// 行在buffer中的起始位置
	// 设置主状态机的初始状态
	CHECK_STATE check_state = CHECK_STATE_REQUESTLINE;
	while (1) {
		data_read = recv(conn_fd, buffer + read_idx, BUFFER_SIZE - read_idx, 0);
		if (data_read == -1) {
			printf("recv error\n");
			break;
		} else if (data_read == 0) {
			printf("remote client has closed the connection\n");
			break;
		}
		read_idx += data_read;
		// 分析目前已经获得的所有客户数据
		HTTP_CODE result = ParseContent(buffer, checked_idx, check_state, read_idx, start_line);
		if (result == NO_REQUEST) { // 尚未得到一个完整的HTTP请求
			continue;
		} else if (result == GET_REQUEST) { // 得到一个完整的、正确的HTTP请求
			send(conn_fd, g_sz_ret[0], strlen(g_sz_ret[0]), 0);
			break;
		} else {
			send(conn_fd, g_sz_ret[1], strlen(g_sz_ret[1]), 0);
			break;
		}
	}
	Close(conn_fd);
	Close(listen_fd);
	return 0;
}