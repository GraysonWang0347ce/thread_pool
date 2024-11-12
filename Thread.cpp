#include "Thread.h"

Thread::Thread(thread_handler th):th_(th)
{
	thread_id_ = std::this_thread::get_id();
}

Thread::~Thread(){}

void Thread::start()
{
	std::thread t(th_, thread_id_);
	t.detach();
}
