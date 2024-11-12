#pragma once
#include"stdafx.h"

class Thread;
class Task_Base;
enum class THREAD_POOL_MODE;
class Result;


class Thread_Pool
{
public:
	Thread_Pool(THREAD_POOL_MODE mode=THREAD_POOL_MODE::MODE_FIXED,
				unsigned int max_tasks = 1024);
	~Thread_Pool();

	void set_mod(THREAD_POOL_MODE mode);

	/*
		@brief To start the pool using params in constructor
	*/
	void start();

	void set_task_queue_threshold(const unsigned int th);

	/*
		@brief submit the specific task to the thread pool
		@return 0 on success 
		1 on timeout
	*/
	Result submit_task(std::shared_ptr<Task_Base> task_sptr);

	void set_thread_pool_size(const size_t size);

	Thread_Pool(const Thread_Pool&) = delete;
	Thread_Pool& operator=(const Thread_Pool&) = delete;

private:
	void thread_handler();

private:
	// thread lists
	std::vector<std::unique_ptr<Thread>> thread_pool_;

	// initial count of threads(usually amount of cpu cores)
	size_t init_thread_size_;
	
	// task queue
	std::queue<std::shared_ptr<Task_Base>> task_q_;
	
	std::atomic_uint task_num_; 

	// maximum amount of task_queue
	unsigned int task_queue_threshold_;

	std::mutex task_queue_mtx_;

	// to show the task_queue's status
	std::condition_variable not_empty_; 
	std::condition_variable not_full_;

	THREAD_POOL_MODE mode_;
};


 