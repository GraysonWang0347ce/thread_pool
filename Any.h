#pragma once
#include"stdafx.h"

class Task_Base;

class Any
{
public:
	Any() = default;
	~Any() = default;
	Any(Any&&) = default;
	Any& operator=(Any&&) = default;

	Any(const Any&) = delete;
	Any& operator = (const Any&) = delete;

	template<class T>
	Any(T data) : base_(std::make_unique<Derived<T>>(data)) {};

	template<class T>
	T cast()
	{
		Derived<T>* pd = dynamic_cast<Derived<T>*>(base_.get());
		if (pd == nullptr)
			throw "type unmatch!";
		return pd->data_;
	}
private:

	class Base
	{
	public:
		virtual ~Base() = default;
	};

	template<class T>
	class Derived : public Base
	{
	public:
		Derived(T data) : data_(data) {};
		T data_;
	};

	std::unique_ptr<Base> base_;
};

class Semaphore
{
public:
	Semaphore(int limit = 0) :res_limit_(limit) {};

	~Semaphore() = default;

	/*
		@brief get a semaphore resource
	*/
	void wait()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		cond_.wait(lock, [&]()->bool {
			return res_limit_ > 0;
			});

		res_limit_--;
	}

	/*
		@brief to add a semaphore resource
	*/
	void post()
	{
		std::unique_lock<std::mutex> lock(mtx_);
		res_limit_++;

		cond_.notify_all();
	}
private:
	std::mutex mtx_;
	std::condition_variable cond_;
	std::atomic_int res_limit_;
};

class Result
{
public:
	Result(std::shared_ptr<Task_Base> task_sp, bool is_valid = true);
	~Result() = default;

	Any get();

	void set_value(Any any);

private:
	Any any_; // storage the return of a thread
	Semaphore sem_;

	// points to task object aimed to get its return
	std::shared_ptr<Task_Base> task_;
	std::atomic_bool is_valid_; // if the return counts
};

