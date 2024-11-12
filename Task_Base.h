#pragma once
#include"stdafx.h"

class Result;
class Any;
/*
	@brief Base class of tasks submitted to thread pool,
	override the run() func when submitted
*/
class Task_Base
{
public:
	Task_Base();
	~Task_Base() = default;
	/*
		@brief override it to startup a new task
	*/
	virtual Any run() = 0;
	void exec();

	void set_result(Result* res);

private:
	Result* res_;
};

