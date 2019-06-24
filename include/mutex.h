
/**
 * 对linux下的mutex进行c++11的封装
 * */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <pthread.h>

namespace Pthread
{

/*禁止拷贝基类*/
class UnCopyable
{
public:
	UnCopyable()
	{}
private:
	UnCopyable(UnCopyable&);
	UnCopyable& operator=(UnCopyable&);
};

/*接口类DestMutex*/
class DestMutex 
{
public:
	virtual ~DestMutex()
	{
	}

	virtual int lock() = 0; // 上锁
	virtual int trylock() = 0; // 尝试上锁
	virtual int unlock() = 0; // 解锁
};

/*Mutex的实现类*/
class MyMutex : public DestMutex
{
public:
	MyMutex();
	~MyMutex();
	inline int lock();
	inline int trylock();
	inline int unlock();

private:
	pthread_mutex_t m_mutex;
};

/*使用对象进行管理mutex锁的生命周期*/
class MyGuard
{
public:
	MyGuard(MyMutex &mutex)
			: m_mutex(mutex)
	{
		m_mutex.lock();
	}

	~MyGuard()
	{
		m_mutex.unlock();
	}

private:
	MyMutex m_mutex;
};

} // namespace Pthread

#endif // MUTEX_H_
