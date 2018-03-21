#ifndef SHARED_BASE_THREAD_H_
#define SHARED_BASE_THREAD_H_

#include <pthread.h>

#include "shared/base/types.h"
#include "shared/base/noncopyable.h"
#include "shared/base/atomic.h"

namespace shared {

class Thread : noncopyable
{
public:
	typedef void* (*ThreadFunc)(void*);

	explicit	Thread(const ThreadFunc, void* pdata, const std::string& name = std::string());
	~Thread();

	void		Start();
	int			Join(); // return pthread_join()

	bool		is_started() const  { return started_; }
	pid_t		tid() const         { return tid_; }
	const std::string& name() const { return name_; }

	static int	num_created() { return numCreated_.get(); }

private:
	static void* StartThread(void* thread);
	void		RunInThread();

	bool		started_;
	bool        joined_;
	pthread_t	pthreadId_;
	pid_t		tid_;
	ThreadFunc	func_;
	void*		pdata_;
    std::string	name_;

	static AtomicInt32 numCreated_;
};

}

#endif // SHARED_BASE_THREAD_H_
