#include "stdafx.h"
#include "Thread_Pool.h"
#include "Task_Base.h"
#include "Any.h"

class MyTask : public Task_Base
{
public:
	Any run() override
	{
		std::cout << "This thread id:" << std::this_thread::get_id() << std::endl;

		// std::this_thread::sleep_for(std::chrono::seconds(2));
		return Any(2);
	}
};

int main()
{
	Thread_Pool pool;
	pool.start();

	pool.submit_task(std::make_shared<MyTask>());

	std::this_thread::sleep_for(std::chrono::seconds(5));
	return 0;
}