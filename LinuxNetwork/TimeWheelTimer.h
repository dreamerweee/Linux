#ifndef TIME_WHEEL_TIMER
#define TIME_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64

class TimeWheelTimer;

// 绑定socket和定时器
typedef struct ClientData {
	struct sockaddr_in addr;
	int sock_fd;
	char buffer[BUFFER_SIZE];
	TimeWheelTimer *timer;
}ClientData;

class TimeWheelTimer {
public:
	TimeWheelTimer(int rot, int ts) : 
		m_next(NULL), m_prev(NULL), m_rotation(rot), m_time_slot(ts) { }

	int m_rotation;   // 记录定时器在时间轮转多少圈后生效
	int m_time_slot;  // 记录定时器属于时间轮上哪个槽（对应的链表，下同）
	void (*m_callback)(ClientData*);  // 定时回调函数
	ClientData *m_user_data;  // 客户数据
	TimeWheelTimer *m_next;  // 指向下一个定时器
	TimeWheelTimer *m_prev;  // 指向前一个定时器
};

class TimeWheel {
public:
	TimeWheel() : m_cur_slot(0)
	{
		for (int i = 0; i < sc_ts_number; ++i) {
			m_slots[i] = NULL;  // 初始化每个槽的头结点
		}
	}
	~TimeWheel()
	{
		for (int i = 0; i < sc_ts_number; ++i) {
			TimeWheelTimer *temp = m_slots[i];
			while (temp) {
				m_slots[i] = temp->m_next;
				delete temp;
				temp = m_slots[i];
			}
		}
	}

	TimeWheelTimer* AddTimer(int timeout)
	{
		if (timeout < 0) {
			return NULL;
		}
		int ticks = 0;
		// 下面根据待插入定时器的超时值计算它将在时间轮转动多少个滴答后被触发，并将该滴答数存储于变量ticks中
		// 如果待插入定时器的超时值小于时间轮的槽间隔sc_si，则将ticks向上折合为1，否则就将tics向下折合为timeout/sc_si
		if (timeout < sc_si) {
			ticks = sc_si;
		} else {
			ticks = timeout / sc_si;
		}

		// 计算待插入的定时器在时间轮转动多少圈后被触发
		int rotation = ticks / sc_ts_number;
		// 计算待插入的定时器应该被插入哪个槽中
		int ts = (m_cur_slot + ticks) % sc_ts_number;
		// 创建新的定时器，它在时间轮转动rotation圈之后被触发，且位于第ts个槽上
		TimeWheelTimer *timer = new TimeWheelTimer(rotation, ts);
		// 如果第ts个槽中尚无任何定时器，则把新建的定时器插入其中，并将该定时器设置为该槽的头结点
		if (!m_slots[ts]) {
			printf("AddTimer rotation is %d, ts is %d, m_cur_slot is %d\n",
				rotation, ts, m_cur_slot);
			m_slots[ts] = timer;
		} else {
			timer->m_next = m_slots[ts];
			m_slots[ts]->m_prev = timer;
			m_slots[ts] = timer;
		}
		return timer;
	}

	// 删除定时器
	void DelTimer(TimeWheelTimer *timer)
	{
		if (!timer) {
			return;
		}
		int ts = timer->m_time_slot;
		if (timer == m_slots[ts]) {
			m_slots[ts] = m_slots[ts]->m_next;
			if (m_slots[ts]) {
				m_slots[ts]->m_prev = NULL;
			}
		} else {
			timer->m_prev->m_next = timer->m_next;
			if (timer->m_next) {
				timer->m_next->m_prev = timer->m_prev;
			}
		}
		delete timer;
	}

	// sc_si时间到后，调用该函数，时间轮向前滚动一个槽的间隔
	void Tick()
	{
		TimeWheelTimer *temp = m_slots[m_cur_slot];
		printf("current slot is %d\n", m_cur_slot);
		while (temp) {
			printf("tick the timer once\n");
			if (temp->m_rotation > 0) {
				temp->m_rotation--;
				temp = temp->m_next;
			} else {
				temp->m_callback(temp->m_user_data);
				if (temp == m_slots[m_cur_slot]) {
					printf("delete header in m_cur_slot\n");
					m_slots[m_cur_slot] = temp->m_next;
					delete temp;
					if (m_slots[m_cur_slot]) {
						m_slots[m_cur_slot]->m_prev = NULL;
					}
					temp = m_slots[m_cur_slot];
				} else {
					temp->m_prev->m_next = temp->m_next;
					if (temp->m_next) {
						temp->m_next->m_prev = temp->m_prev;
					}
					TimeWheelTimer *temp2 = temp->m_next;
					delete temp;
					temp = temp2;
				}
			}
		}
		m_cur_slot = ++m_cur_slot % sc_ts_number;
	}

private:
	static const int sc_ts_number = 60;  // 时间轮上槽的数目
	static const int sc_si = 1;  // 每1s时间轮转动一次，即槽间隔为1s
	TimeWheelTimer *m_slots[sc_ts_number];  // 时间轮的槽，其中每个元素指向一个定时器链表，链表无序
	int m_cur_slot;   // 时间轮的当前槽
};

#endif  // TIME_WHEEL_TIMER