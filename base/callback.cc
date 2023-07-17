
#include "callback.h"

namespace base {

Closure::Closure(FUNC func)
	: func_(func)
{
};

void Closure::Run()
{
	func_();
}

Closure Bind(ClosureFunction func, void *param)
{
	Closure closure(std::bind(func, param));
	return closure;
}

}