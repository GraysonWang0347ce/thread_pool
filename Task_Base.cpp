#include "Task_Base.h"
#include "Any.h"

Task_Base::Task_Base()
{
	res_ = nullptr;
}

void Task_Base::exec()
{
	if (res_ != nullptr)
		res_->set_value(run());
}

void Task_Base::set_result(Result* res)
{
	res_ = res;
}
