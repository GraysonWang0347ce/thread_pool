#include "Any.h"
#include "Task_Base.h"

Result::Result(std::shared_ptr<Task_Base> task_sp, bool is_valid)
{
	task_ = task_sp;
	is_valid_ = is_valid;

	task_->set_result(this);
}

Any Result::get()
{
	if (!is_valid_)
		return "";

	sem_.wait();
	return std::move(any_);
}

void Result::set_value(Any any)
{
	this->any_ = std::move(any);

	sem_.post();
}
