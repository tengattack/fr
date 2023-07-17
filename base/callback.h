
#ifndef BASE_CALLBACK_H_
#define BASE_CALLBACK_H_
#pragma once

#include <functional>

namespace base {

typedef void (* ClosureFunction)(void *);

class Closure {
public:
	typedef std::function<void(void)> FUNC;
	Closure(FUNC func);
	void Run();

protected:
	FUNC func_;
};

Closure Bind(ClosureFunction func, void *param);

}  // namespace base

#endif  // BASE_CALLBACK_H_