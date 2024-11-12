#pragma once
#include"stdafx.h"

class Thread
{
public:
	using thread_handler = std::function<void(std::thread::id)>;

	Thread(thread_handler th);
	~Thread();
	/*
		@brief Start this Thread 
	*/
	void start();

	std::thread::id getid() const { return thread_id_; };

private:
	thread_handler th_;
	std::thread::id thread_id_;
};

