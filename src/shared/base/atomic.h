#ifndef SHARED_BASE_ATOMIC_H_
#define SHARED_BASE_ATOMIC_H_

#include <stdint.h>

#include "shared/base/noncopyable.h"

namespace shared {

template<typename T>
class AtomicIntegerT : noncopyable
{
public:
	AtomicIntegerT()
		: value_(0)
	{
	}

	T get()
	{
		return __sync_val_compare_and_swap(&value_, 0, 0);
	}

	T get_and_add(T x)
	{
		return __sync_fetch_and_add(&value_, x);
	}

	T add_and_get(T x)
	{
		return get_and_add(x) + x;
	}

	T increment_and_get()
	{
		return add_and_get(1);
	}

	T decrement_and_get()
	{
		return add_and_get(-1);
	}

	void add(T x)
	{
		get_and_add(x);
	}

	void increment()
	{
		increment_and_get();
	}

	void decrement()
	{
		decrement_and_get();
	}

	T get_and_set(T newValue)
	{
		return __sync_lock_test_and_set(&value_, newValue);
	}

private:
	volatile T value_;
};

// signed
typedef AtomicIntegerT<int32_t> AtomicInt32;
typedef AtomicIntegerT<int64_t> AtomicInt64;

// unsigned
typedef AtomicIntegerT<uint32_t> AtomicUInt32;
typedef AtomicIntegerT<uint64_t> AtomicUInt64;
}

#endif // SHARED_BASE_ATOMIC_H_
