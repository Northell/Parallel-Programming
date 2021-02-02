#define _ENABLE_ATOMIC_ALIGNMENT_FIX 8;

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

#include "Timer.h"



template<typename T>
class LockFreeStack //Treiberstack
{
	
private:
	struct node;
		
	struct counted_node_ptr
	{
		int external_count;
		node* ptr;

	};

	struct node
	{
		std::shared_ptr<T> data;
		std::atomic<int> internal_count;
		counted_node_ptr next;
		node(T const& data_):internal_count(0) {
			this->data = data;//( std::shared_ptr<Bar>(new Bar), foo())
		}

	};
	

	std::atomic<counted_node_ptr> head;

	void increase_head_count(counted_node_ptr& old_counter)
	{
		counted_node_ptr new_counter;

		do
		{
			new_counter = old_counter;
			++new_counter.external_count;
		} while (!head.compare_exchange_strong(old_counter, new_counter, 
												std::memory_order_acquire,
												std::memory_order_relaxed));

		old_counter.external_count = new_counter.external_count;
	}

public:
	~LockFreeStack()
	{
		while (pop());
	}

	void push(T const& data)
	{
		counted_node_ptr new_node;
		new_node.ptr = new node(data);
		new_node.external_count = 1;
		new_node.ptr->next = head.load(std::memory_order_relaxed);
		while (!head.compare_exchange_weak(new_node.ptr->next, new_node, 
											std::memory_order_release,
											std::memory_order_relaxed));

	}

	std::shared_ptr<T> pop()
	{
		counted_node_ptr old_head = head.load(std::memory_order_relaxed);
		for (;;)
		{
			increase_head_count(old_head);
			node* const ptr = old_head.ptr;
			if (!ptr)
			{
				return std::shared_ptr<T>();
			}
			if (head.compare_exchange_strong(old_head, ptr->next, std::memory_order_relaxed))
			{
				std::shared_ptr<T> res;
				res.swap(ptr->data);
				int const count_increase = old_head.external_count - 2;
				if (ptr->internal_count.fetch_add(count_increase,
					std::memory_order_release) == -count_increase)
				{
					delete ptr;
				}
				return res;
			}
			else if (ptr->internal_count.fetch_add(-1, std::memory_order_relaxed) == 1)
			{
				ptr->internal_count.load(std::memory_order_acquire);
				delete ptr;
			}
		}
	}
	
	


};




LockFreeStack<int> stack;
std::vector<int> source(10000);

void setValue(int value)
{
	stack.push(value);
}

int main() {

	



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
			threads.push_back(std::thread(setValue, i));
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