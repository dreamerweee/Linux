#ifndef LIST_TIMER_H
#define LIST_TIMER_H

#include <iostream>
#include <time.h>

using std::cout;
using std::endl;

#define BUFFER_SIZE 64

class UtilTimer;   // 前向声明

// 用户数据结构：客户端socket地址、socket文件描述符、读缓存和定时器
typedef struct ClientData {
	struct sockaddr_in cli_addr;
	int sock_fd;
	char buff[BUFFER_SIZE];
	UtilTimer *timer;
}ClientData;

// 定时器类
class UtilTimer{
public:
	UtilTimer() : m_prev(NULL), m_next(NULL) { }

	time_t m_expire;  // 任务的超时时间，这里使用绝对时间
	void (*cb_func)(ClientData*);  // 任务回调函数
	ClientData *m_user_data;  // 回调函数处理的客户数据，由定时器的执行者传递给回调函数
	UtilTimer *m_prev;   // 指向前一个定时器
	UtilTimer *m_next;   // 指向下一个定时器
};

// 定时器链表，它是一个升序、双向链表，且带有头节点和尾节点
class SortTimerList {
public:
	SortTimerList() : m_head(NULL), m_tail(NULL) { }
	~SortTimerList()  // 销毁所有定时器
	{
		UtilTimer *temp = m_head;
		while (temp) {
			m_head = temp->m_next;
			delete temp;
			temp = m_head;
		}
	}

	// 将目标定时器timer添加到链表中
	void AddTimer(UtilTimer *timer)
	{
		if (!timer) {
			return;
		}
		if (!m_head) {
			m_head = m_tail = timer;
			return;
		}
		/*
		* 如果目标定时器的超时时间小于当前链表中所有定时器的超时时间，则把该定时器插入链表头部，
		* 作为链表新的头节点。否则就需要调用重载函数AddTimer(UtilTimer *timer, UtilTimer *head)，
		* 把它插入链表中合适的位置，以保证链表的升序特性
		*/
		if (timer->m_expire < m_head->m_expire) {
			timer->m_next = m_head;
			m_head->m_prev = timer;
			m_head = timer;
			return;
		}
		AddTimer(timer, m_head);
	}

	/*
	* 当某个定时任务发送变化时，调整对应的定时器在链表中的位置。
	* 这个函数只考虑被调整的定时器的超时时间延长的情况，即该定时器需要往链表尾部移动
	*/
	void AdjustTimer(UtilTimer *timer)
	{
		if (!timer) {
			return;
		}
		UtilTimer *temp = timer->m_next;
		// 如果被调整的目标定时器处在链表尾部，或者该定时器新的超时值仍然小于其下一个定时器的超时值，则不用调整
		if (!temp || (timer->m_expire < temp->m_expire)) {
			return;
		}
		// 如果目标定时器是链表的头节点，则将该定时器从链表中取出并重新插入链表
		if (timer == m_head) {
			m_head = m_head->m_next;
			m_head->m_prev = NULL;
			timer->m_next = NULL;
			AddTimer(timer, m_head);
		} else {  // 如果目标定时器不是链表的头节点，则将该定时器从链表中取出，然后插入其原来所在位置之后的部分链表中
			timer->m_prev->m_next = timer->m_next;
			timer->m_next->m_prev = timer->m_prev;
			AddTimer(timer, timer->m_next);
		}
	}

	// 删除目标定时器
	void DelTimer(UtilTimer *timer)
	{
		if (!timer) {
			return;
		}
		// 只有一个定时器
		if ((timer == m_head) && (timer == m_tail)) {
			delete timer;
			m_head = m_tail = NULL;
			return;
		}
		// 有多个定时器
		if (timer == m_head) {
			m_head = m_head->m_next;
			m_head->m_prev = NULL;
			delete timer;
			return;
		}
		if (timer == m_tail) {
			m_tail = m_tail->m_prev;
			m_tail->m_next = NULL;
			delete timer;
			return;
		}
		timer->m_prev->m_next = timer->m_next;
		timer->m_next->m_prev = timer->m_prev;
		delete timer;
	}


	// SIGALRM信号每次被触发就在其信号处理函数（如果使用统一事件源，则是主函数）中执行一次Tick函数，以处理链表上到期的任务
	void Tick()
	{
		if (!m_head) {
			return;
		}
		cout << "Timer Tick" << endl;

		time_t cur = time(NULL);  // 获得系统当前的时间
		UtilTimer *temp = m_head;
		while (temp) {
			if (cur < temp->m_expire) {
				break;
			}
			temp->cb_func(temp->m_user_data);
			// 重新设置头节点
			m_head = temp->m_next;
			if (m_head) {
				m_head->m_prev = NULL;
			}
			delete temp;
			temp = m_head;
		}
	}

private:
	// 辅助函数
	void AddTimer(UtilTimer *timer, UtilTimer *head)
	{
		UtilTimer *prev = head;
		UtilTimer *temp = prev->m_next;
		while (temp) {
			if (timer->m_expire < temp->m_expire) {
				prev->m_next = timer;
				timer->m_next = temp;
				temp->m_prev = timer;
				timer->m_prev = prev;
				break;
			}
			prev = temp;
			temp = temp->m_next;
		}
		// 未找到合适位置，则插入尾部
		if (!temp) {
			prev->m_next = timer;
			timer->m_prev = prev;
			timer->m_next = NULL;
			m_tail = timer;
		}
	}

private:
	UtilTimer *m_head;
	UtilTimer *m_tail;
};

#endif  // LIST_TIMER_H