#include "ThreadPool.h"
#include <iostream>
#include <utility>

ThreadPool::ThreadPool(unsigned int size) :
	Stop(false)
{
	for (unsigned int i=0; i<size; ++i)
	{
		Pool.push_back(SThread(new std::thread(&ThreadPool::ThreadFunction, this)));
	}
}

ThreadPool::~ThreadPool()
{
	std::unique_lock<std::mutex> lock(Mutex);
	Stop=true;
	ThreadHole.notify_all();
	lock.unlock();
}

void ThreadPool::ThreadFunction()
{
	std::unique_lock<std::mutex> lock(Mutex);
	while (!Stop)
	{
		while (Queue.empty())
			ThreadHole.wait(lock);
		SFunction f=std::move(Queue.front());
		Queue.pop();
		lock.unlock();

		f();

		lock.lock();
	}
	lock.unlock();
}
