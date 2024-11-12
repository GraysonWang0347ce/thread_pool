#include"Thread_Pool.h"
#include"Thread.h"
#include"Task_Base.h"
#include"Any.h"

#define THREAD_MAX_IDLE_TIME 60

void Thread_Pool::set_thread_pool_size(const size_t size)
{
	init_thread_size_ = size;
}

Thread_Pool::Thread_Pool(THREAD_POOL_MODE mode,
	unsigned int max_tasks)
	:mode_(mode),
	task_queue_threshold_(max_tasks),
	idle_thread_size_(0),
	thread_queue_threshold_(300)
{
	// using current cpu cores number to init thread_pool
#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	size_t core_num = sysinfo.dwNumberOfProcessors;

	cur_thread_num_ = core_num;

	//  std::cout << core_num << std::endl;
	set_thread_pool_size(core_num);
#endif

	is_started = false;
}

Thread_Pool::~Thread_Pool() {}

void Thread_Pool::set_mod(THREAD_POOL_MODE mode)
{
	if (started())
		return;
	
	mode_ = mode;
}

void Thread_Pool::start()
{
	if (started())
		return;

	is_started = true;

	// create and vectorize Threads
	for (int i = 0; i < init_thread_size_; ++i)
	{
		auto ptr = std::make_unique<Thread>((std::bind(&Thread_Pool::thread_handler, this, std::placeholders::_1)));
		// new Thread(std::bind(&Thread_Pool::thread_handler, this));
		// thread_pool_.emplace_back(std::move(ptr));

		auto thread_id = ptr->getid();
		thread_pool_.insert({ thread_id, std::move(ptr) });
	}

	// active all Threads
	for (auto& t : thread_pool_)
	{
		t.second->start();
		idle_thread_size_++;
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

	// if THREAD_POOL_MODE::CACHED is set
	if (mode_ == THREAD_POOL_MODE::MODE_CACHED
		&& task_num_ > idle_thread_size_
		&& cur_thread_num_ < thread_queue_threshold_)
	{
		while (cur_thread_num_ <= thread_queue_threshold_)
		{
			auto ptr = std::make_unique<Thread>((std::bind(&Thread_Pool::thread_handler, this, std::placeholders::_1)));
			// new Thread(std::bind(&Thread_Pool::thread_handler, this));
			auto thread_id = ptr->getid();
			thread_pool_.insert({ thread_id, std::move(ptr) });

			thread_pool_[thread_id]->start();
			cur_thread_num_++;
			idle_thread_size_++;
		}
	}

	return Result(task_sptr); //(int)ERROR_CODE::SUCCESS;
}

void Thread_Pool::set_max_thread(const unsigned int num)
{
	if (started()
		&& mode_ == THREAD_POOL_MODE::MODE_CACHED)
		thread_queue_threshold_ = num;
}

void Thread_Pool::thread_handler(std::thread::id tid)
{
	//std::cout << "Begin thread " << std::this_thread::get_id() << std::endl;
	//std::cout << "End thread " << std::this_thread::get_id() << std::endl;

	while (TRUE)
	{
		auto last_time = std::chrono::high_resolution_clock().now();

		std::shared_ptr<Task_Base> task;
		{
			std::unique_lock<std::mutex> lock(task_queue_mtx_);

			// if THREAD_POOL_MODE::CACHED 
			// to remove extra thread
			if (mode_ == THREAD_POOL_MODE::MODE_CACHED)
			{
				while (task_q_.size() > 0)
				{
					if (std::cv_status::timeout ==
						not_empty_.wait_for(lock,
							std::chrono::seconds(1)))
					{
						// timeout
						auto now = std::chrono::high_resolution_clock().now();
						auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - last_time);
						if (dur.count() >= THREAD_MAX_IDLE_TIME
							&& cur_thread_num_ > init_thread_size_)
						{
							// recycle extra threads
							thread_pool_.erase(tid);
							cur_thread_num_--;
							idle_thread_size_--;
							
							std::cout << "Thread: " << tid << "Eliminated" << std::endl;
							return;
						}
					}
				}
			}

			not_empty_.wait(lock,
				[&]()->bool {return task_q_.size() > 0; });

			idle_thread_size_--;

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

		// update the time when last thread returned
		last_time = std::chrono::high_resolution_clock().now();
		idle_thread_size_++;
	}
}
