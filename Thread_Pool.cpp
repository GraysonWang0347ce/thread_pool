#include"Thread_Pool.h"
#include"Thread.h"
#include"Task_Base.h"
#include"Any.h"

void Thread_Pool::set_thread_pool_size(const size_t size)
{
	init_thread_size_ = size;
}

Thread_Pool::Thread_Pool(THREAD_POOL_MODE mode,
	unsigned int max_tasks)
	:mode_(mode),
	task_queue_threshold_(max_tasks)
{
	// using current cpu cores number to init thread_pool
#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	size_t core_num = sysinfo.dwNumberOfProcessors;

	//  std::cout << core_num << std::endl;
	set_thread_pool_size(core_num);
#endif

}

Thread_Pool::~Thread_Pool() {}

void Thread_Pool::set_mod(THREAD_POOL_MODE mode)
{
	mode_ = mode;
}

void Thread_Pool::start()
{
	// create and vectorize Threads
	for (int i = 0; i < init_thread_size_; ++i)
	{
		auto ptr = std::make_unique<Thread>((std::bind(&Thread_Pool::thread_handler, this)));
		// new Thread(std::bind(&Thread_Pool::thread_handler, this));
		thread_pool_.emplace_back(std::move(ptr));
	}

	// active all Threads
	for (auto& t : thread_pool_)
	{
		t->start();
	}
}

void Thread_Pool::set_task_queue_threshold(const unsigned int th)
{
	task_queue_threshold_ = th;
}

Result Thread_Pool::submit_task(std::shared_ptr<Task_Base> task_sptr)
{
	std::unique_lock<std::mutex> lock(task_queue_mtx_);

	if (!not_full_.wait_for(lock,
		std::chrono::seconds(1),
		[&]()->bool {
			return task_q_.size() < task_queue_threshold_;
		}))
	{
		// timeout
		std::cerr << "Task Queue is full, sumbit error" << std::endl;
		return Result(task_sptr,false);
	}

	task_q_.emplace(task_sptr);
	task_num_++;

	not_empty_.notify_all();

	return Result(task_sptr); //(int)ERROR_CODE::SUCCESS;
}

void Thread_Pool::thread_handler()
{
	//std::cout << "Begin thread " << std::this_thread::get_id() << std::endl;
	//std::cout << "End thread " << std::this_thread::get_id() << std::endl;

	while (TRUE)
	{
		std::shared_ptr<Task_Base> task;
		{
			std::unique_lock<std::mutex> lock(task_queue_mtx_);

			not_empty_.wait(lock,
				[&]()->bool {return task_q_.size() > 0; });

			task = task_q_.front();
			task_q_.pop();
			task_num_--;

			if (task_q_.size() > 0)
				not_empty_.notify_all();
		}

		not_full_.notify_all();

		if (task != nullptr)
		{
			// task->run();
			task->exec();
		}
	}
}
