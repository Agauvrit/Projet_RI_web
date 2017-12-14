#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>

class ThreadPool
{
private:
	struct SThread
	{
	public:
		SThread(std::thread *_t) : t(_t) {}
		SThread(SThread &&st) : t(st.t) { st.t=nullptr; }
		~SThread() { if (t) { t->join(); delete(t); } }

	private:
		SThread(SThread &st) {}

	private:
		std::thread *t;
	};

	struct StorageFunction
	{
		virtual void Execute()=0;
	};

	struct SFunction
	{
	public:
		SFunction(StorageFunction *_f) : func(_f) {}
		SFunction(SFunction &&sf) : func(sf.func) { sf.func=nullptr; }
		~SFunction() { delete(func); }
		inline void operator()() { func->Execute(); }

	private:
		SFunction(SFunction &sf) {}

	private:
		StorageFunction *func;
	};

	template<class F>
	struct StorageFunctionImpl : public StorageFunction
	{
		StorageFunctionImpl(const F &func) : func(func) {}
		inline void operator()() { Execute(); }
		inline void Execute() { func(); }

	private:
		const F &func;
	};

public:
	ThreadPool(unsigned int size);
	~ThreadPool();
	template<class F> void Execute(const F &func);

private:
	void ThreadFunction();

private:
	std::vector<SThread> Pool;
	std::queue<SFunction> Queue;
	bool Stop;
	std::mutex Mutex;
	std::condition_variable ThreadHole;
};

template<class F>
inline void ThreadPool::Execute(const F &func)
{
	std::lock_guard<std::mutex> lock(Mutex);
	Queue.push(SFunction(new StorageFunctionImpl<F>(func)));
	ThreadHole.notify_one();
}
