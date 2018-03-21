#ifndef SHARED_NET_CHANNEL_H_
#define SHARED_NET_CHANNEL_H_

#include <string>
#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"

namespace shared {
namespace net {

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : noncopyable
{
public: 
	typedef void* (*EventCallback)(void*);
	typedef void* (*ReadEventCallback)(void*);

	Channel(EventLoop* loop, int fd);
	~Channel();

	void HandleEvent();

	void set_read_callback(const ReadEventCallback& cb, void* pdata)
	{
		read_callback_.func		= cb;
		read_callback_.pdata_	= pdata;
	}
	void set_write_callback(const EventCallback& cb, void* pdata)
	{
		write_callback_.func	= cb;
		write_callback_.pdata_	= pdata;
	}
	void set_close_callback(const EventCallback& cb, void* pdata)
	{
		close_callback_.func	= cb;
		close_callback_.pdata_	= pdata;
	}
	void set_error_callback(const EventCallback& cb, void* pdata)
	{
		error_callback_.func	= cb;
		error_callback_.pdata_	= pdata;
	}

	int		fd() const { return fd_; }
	int		events() const { return events_; }
	void	set_revents(int revt) { revents_ = revt; }
	bool	is_none_event() const { return events_ == kNoneEvent; }

	void	EnableReading() { events_ |= kReadEvent; Update(); }
	void	EnableWriting() { events_ |= kWriteEvent; Update(); }
	void	DisableWriting() { events_ &= ~kWriteEvent; Update(); }
	void	DisableAll() { events_ = kNoneEvent; Update(); }
	bool	is_writing() const { return events_ & kWriteEvent; }

	// for Poller
	int		index() { return index_; }
	void	set_index(int idx) { index_ = idx; } 

	// for debug
    std::string	ReventsToString() const;

	void	DoNotLogHup() { loghup_ = false; }

	void	Remove();
	EventLoop* owner_loop() { return loop_; }

	// use to lock HandleEvent()
	void TieMutex(MutexLock& mutex) { mutex_ = &mutex; }

private:
	class ReadEventCallbackWrapper
	{
	public:
		ReadEventCallbackWrapper()
			: func(NULL),
			  pdata_(NULL)
		{ }

		ReadEventCallback func;
		void* pdata_;
	};

	class EventCallbackWrapper
	{
	public:
		EventCallbackWrapper()
			: func(NULL),
			  pdata_(NULL)
		{ }

		EventCallback func;
		void* pdata_;
	};

private:
	void	Update();
	void	HandleEventWithGuard();

	ReadEventCallbackWrapper	read_callback_;
	EventCallbackWrapper		write_callback_;
	EventCallbackWrapper		close_callback_;
	EventCallbackWrapper		error_callback_;

	static const int kNoneEvent;
	static const int kReadEvent;
	static const int kWriteEvent;

	EventLoop*	loop_;
	const int	fd_;
	int			events_;
	int			revents_;
	int			index_;
	bool		loghup_;
	bool		event_handling_;
	MutexLock*	mutex_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_CHANNEL_H_
