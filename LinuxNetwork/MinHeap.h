#ifndef MIN_HEAP
#define MIN_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>

using std::exception;

#define BUFFER_SIZE 64

class HeapTimer;

typedef struct ClientData {
	struct sockaddr_in addr;
	int sock_fd;
	char buffer[BUFFER_SIZE];
	HeapTimer *timer;
}ClientData;

class HeapTimer {
public:
	HeapTimer(int delay)
	{
		m_expire = time(NULL) + delay;
	}

	time_t m_expire;   // 定时器生效的绝对时间
	void (*m_callback)(ClientData*);  // 定时器的回调函数
	ClientData *m_user_data;   // 用户数据
};

class TimeHeap {
public:
	TimeHeap(int cap) throw (std::exception) : m_capacity(cap), m_cur_size(0)
	{
		m_array = new HeapTimer*[m_capacity];
		if (!m_array) {
			throw std::exception();
		}
		for (int i = 0; i < m_capacity; ++i) {
			m_array[i] = NULL;
		}
	}
	TimeHeap(HeapTimer **init_array, int size, int capacity) throw (std::exception) :
		m_cur_size(size), m_capacity(capacity)
	{
		if (capacity < size) {
			throw std::exception();
		}
		m_array = new HeapTimer*[capacity];
		if (!m_array) {
			throw std::exception();
		}
		for (int i = 0; i < capacity; ++i) {
			m_array[i] = NULL;
		}
		if (size != 0) {
			for (int i = 0; i < size; ++i) {
				m_array[i] = init_array[i];
			}
			for (int i = (m_cur_size - 1) / 2; i >= 0; --i) {
				PercolateDown(i);  // 对数组中的第[m_cur_size - 1) / 2]~0个元素执行下滤操作
			}
		}
	}

	~TimeHeap()
	{
		for (int i = 0; i < m_cur_size; ++i) {
			delete m_array[i];
		}
		delete [] m_array;
	}


	// 添加定时器
	void AddTimer(HeapTimer *timer)  throw (std::exception)
	{
		if (!timer) {
			return;
		}
		if (m_cur_size >= m_capacity) {
			Resize();
		}
		// 插入一个元素，当前堆大小加1，hole是新建空穴的位置
		int hole = m_cur_size++;
		int parent = 0;
		// 对从空穴到根节点的路径上的所有节点执行上滤操作
		for (; hole > 0; hole = parent) {
			parent = (hole - 1) / 2;
			if (m_array[parent]->m_expire <= timer->m_expire) {
				break;
			}
			m_array[hole] = m_array[parent];
		}
		m_array[hole] = timer;
	}

	// 删除定时器
	void DelTimer(HeapTimer *timer)
	{
		if (!timer) {
			return;
		}
		// 仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁
		// 这将节省真正删除该定时器造成的开销，但这样做容易使堆数组膨胀
		timer->m_callback = NULL;
	}

	// 获得堆顶的定时器
	HeapTimer* Top() const
	{
		if (Empty()) {
			return NULL;
		}
		return m_array[0];
	}

	// 删除堆顶定时器
	void PopTimer()
	{
		if (Empty()) {
			return;
		}
		if (m_array[0]) {
			delete m_array[0];
			m_array[0] = m_array[--m_cur_size];
			PercolateDown(0);  // 对新的堆顶元素执行下滤操作
		}
	}

	// 心搏函数
	void Tick()
	{
		HeapTimer *temp = m_array[0];
		time_t cur = time(NULL);
		while (!Empty()) {
			if (!temp) {
				break;
			}
			if (temp->m_expire > cur) {
				break;
			}
			if (m_array[0]->m_callback) {
				m_array[0]->m_callback(m_array[0]->m_user_data);
			}
			PopTimer();
			temp = m_array[0];
		}
	}

	bool Empty() const
	{
		return m_cur_size == 0;
	}

private:
	// 最小堆的下滤操作，它确保堆数组中以第hole个节点作为跟的子树拥有最小堆性质
	void PercolateDown(int hole)
	{
		HeapTimer *temp = m_array[hole];
		int child = 0;
		for (; ((hole * 2 + 1) <= (m_cur_size - 1)); hole = child) {
			child = hole * 2 + 1;
			if (child < (m_cur_size - 1) && m_array[child + 1]->m_expire < m_array[child]->m_expire) {
				++child;
			}
			if (m_array[child]->m_expire < temp->m_expire) {
				m_array[hole] = m_array[child];
			} else {
				break;
			}
		}
		m_array[hole] = temp;
	}

	// 将堆数组容量扩大1倍
	void Resize() throw (std::exception)
	{
		HeapTimer **temp = new HeapTimer*[2 * m_capacity];
		if (!temp) {
			throw std::exception();
		}
		for (int i = 0; i < 2 * m_capacity; ++i) {
			temp[i] = NULL;
		}
		m_capacity = 2 * m_capacity;
		for (int i = 0; i < m_cur_size; ++i) {
			temp[i] = m_array[i];
		}
		delete [] m_array;
		m_array = temp;
	}

private:
	HeapTimer **m_array;   // 堆数组
	int m_capacity;   // 堆数组的容量
	int m_cur_size;   // 堆数组当前包含元素的个数
};

#endif // MIN_HEAP
