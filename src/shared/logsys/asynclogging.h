#ifndef SHARED_LOGSYS_ASYNCLOGGING_H_
#define SHARED_LOGSYS_ASYNCLOGGING_H_

#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/blocking_queue.h"
#include "shared/base/countdownlatch.h"
#include "shared/base/mutex.h"
#include "shared/base/thread.h"

#include "shared/logsys/logstream.h"


namespace shared {

class AsyncLogging : noncopyable
{
public:
	AsyncLogging(const string& basename,
			     size_t rollSize,
			     int flushInterval = 3);

	~AsyncLogging();

	void Append(const char* logline, int len);

	void Start()
	{
		running_ = true;
		thread_.Start();
		latch_.Wait();
	}

	void Stop()
	{
		running_ = false;
		cond_.Notify();
		thread_.Join();
	}

private:

	// declare but not define, prevent compiler-synthesized functions
	AsyncLogging(const AsyncLogging&);  // ptr_container
	void operator=(const AsyncLogging&);  // ptr_container

	void threadFunc();
	static void* RegisterThreadFunc(void*);

	typedef shared::detail::FixedBuffer<shared::detail::kLargeBuffer> Buffer;

	//typedef boost::ptr_vector<Buffer> BufferVector;
	//typedef BufferVector::auto_type BufferPtr;
	
	typedef std::vector<Buffer*> BufferVector;
	typedef Buffer* BufferPtr;

	const int			flush_interval_;
	bool				running_;
	string				basename_;
	size_t				rollsize_;
	shared::Thread		thread_;
	shared::CountDownLatch latch_;
	shared::MutexLock	mutex_;
	shared::Condition	cond_;

	BufferPtr current_buffer_;
	BufferPtr next_buffer_;
	BufferVector buffers_;

	inline void move_buf_ptr(Buffer* dest, Buffer* src) 
	{ 
		dest = src; 
		src	 = NULL; 
	}
};

} // namespace shared

#endif  // SHARED_LOGSYS_ASYNCLOGGING_H_
