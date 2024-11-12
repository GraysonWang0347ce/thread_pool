#include "Thread.h"

Thread::Thread(thread_handler th):th_(th)
{}

Thread::~Thread(){}

void Thread::start()
{
	std::thread t(th_);
	t.detach();
}
