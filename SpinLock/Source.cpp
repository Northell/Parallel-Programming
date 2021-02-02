#include <atomic>
#include <iostream>
#include <vector>
#include "Timer.h"
#include <immintrin.h>
#include <thread>



class SpinLock
{
	std::atomic_flag flag; 
public:
	void lock()
	{
		while (flag.test_and_set(std::memory_order_acquire))
			;
	}
	bool try_lock()
	{
		return !flag.test_and_set(std::memory_order_acquire);
	}
	void unlock()
	{
		flag.clear(std::memory_order_release);
	}
};




SpinLock locker;
std::vector<int> source(10000);

void SetValue(SpinLock& locker, std::vector<int>& source, std::size_t index, int value)
{
	locker.lock();
	source[index] = value;
	locker.unlock();
}

void MainThreadLoop(int startIndex, int step)
{
	for (size_t i = startIndex; i < source.size(); i += step)
	{
		SetValue(locker, source, i, i + 1);
	}
}


int main() {

	SpinLock spinlock;
	
	
	
	for (size_t i = 0; i < source.size(); ++i)
	{
		source[i] = i;
	}

	for (int threadCounts = 1; threadCounts < 15; ++threadCounts)
	{
		std::vector<std::thread> threads(threadCounts);

		Timer timer;
		for (int i = 0; i < threadCounts; ++i)
		{
			threads.push_back(std::thread(MainThreadLoop, i, threadCounts));
		}

		for (auto& t : threads)
		{
			if (t.joinable())
				t.join();
		}
		std::cout << "Threads count: " << threadCounts << " Works time: " << timer.elapsed() << std::endl;
	}

	system("pause");
	
	
	
	return 0;
}