
#ifndef BASE_DEFER_PTR_H_
#define BASE_DEFER_PTR_H_
#pragma once

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <functional>
#include <vector>

#include "base/compiler_specific.h"

class defer_ptr {
public:

	// The element type
	typedef std::function<void()> C;

	// Constructor.  Defaults to initializing with NULL.
	// There is no way to create an uninitialized scoped_ptr.
	// The input parameter must be allocated with new.
	explicit defer_ptr(C p = NULL) : ptr_(p) { }

	// Destructor.  If there is a C object, delete it.
	// We don't need to test ptr_ == NULL because C++ does that for us.
	~defer_ptr() {
		if (ptr_) {
			ptr_();
		}
	}

	// Accessors to get the owned object.
	C get() const { return ptr_; }

	// Swap two defer pointers.
	void swap(defer_ptr& p2) {
		ptr_.swap(p2.ptr_);
	}

	// Release a pointer.
	// The return value is the current pointer held by this object.
	// If this object holds a NULL pointer, the return value is NULL.
	// After this operation, this object will hold a NULL pointer,
	// and will not own the object any more.
	C release() WARN_UNUSED_RESULT {
		C retVal = ptr_;
		ptr_ = NULL;
		return retVal;
	}

private:
	C ptr_;

	bool operator==(defer_ptr const& p2) const;
	bool operator!=(defer_ptr const& p2) const;

	// Disallow evil constructors
	defer_ptr(const defer_ptr&);
	void operator=(const defer_ptr&);
};

class defer_array {
public:

	// Constructor.
	explicit defer_array() { }

	~defer_array() {
		while (!ptrs_.empty()) {
			delete ptrs_.back();
			ptrs_.pop_back();
		}
	}

	void push(defer_ptr::C p = NULL) {
		ptrs_.push_back(new defer_ptr(p));
	}

	defer_ptr::C pop() {
		defer_ptr *d = ptrs_.back();
		ptrs_.pop_back();
		defer_ptr::C _t = d->release();
		delete d;
		return _t;
	}

private:
	std::vector<defer_ptr *> ptrs_;

	bool operator==(defer_array const& p2) const;
	bool operator!=(defer_array const& p2) const;

	// Disallow evil constructors
	defer_array(const defer_array&);
	void operator=(const defer_array&);
};

#define DEFER_INIT() defer_array _da;
#define DEFER(exp) _da.push([]{ exp; });
#define DEFER_CATCH(exp, c) _da.push([c]{ exp; });

#endif  // BASE_DEFER_PTR_H_
