#ifndef THREAD_POOL
#define THREAD_POOL
#include<iostream>
#include<vector>
#include<queue>
#include<memory>
#include<atomic>
#include<mutex>
#include<condition_variable>
#include<functional>
#include<thread>
#include<unordered_map>


#ifdef _WIN32
	#include<Windows.h>

#endif

/*
	@brief MODE_FIXED, fixed number of threads;
	MODE_CACHED, varible number of threads
*/
enum class THREAD_POOL_MODE
{
	MODE_FIXED, /* fixed number of threads*/
	MODE_CACHED, /* varible number of threads*/
};

enum class ERROR_CODE
{
	SUCCESS,
	TIMEOUT,
};

#define ERR2INT(n) (int)(ERROR_CODE::n)

#endif