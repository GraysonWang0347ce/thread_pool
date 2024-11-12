#pragma once
#include"stdafx.h"

class Thread
{
public:
	using thread_handler = std::function<void()>;

	explicit Thread(thread_handler th);
	~Thread();
	/*
		@brief Start this Thread 
	*/
	void start();

private:
	thread_handler th_;
};

