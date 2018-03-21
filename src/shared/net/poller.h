#ifndef SHARED_NET_POLLER_H_
#define SHARED_NET_POLLER_H_

#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/net/eventloop.h"

namespace shared {
namespace net {

class Channel;

/// 
/// Base class for IO Multiplexing
/// 
/// This class doesn't own the Channel objects.
class Poller : noncopyable
{
public: 
	typedef std::vector<Channel*> ChannelList;

	Poller(EventLoop* loop);
	virtual ~Poller();

	/// Polls the I/O events.
	/// Must be called in the loop thread.
	virtual int  Poll(int timeoutMs, ChannelList* active_channels) = 0;

	/// Changes the interested I/O events
	/// Must be called in the loop thread.
	virtual void UpdateChannel(Channel* channel) = 0;

	/// Remove the channel, when it destructs.
	/// Must be called in the loop thread.
	virtual void RemoveChannel(Channel* channel) = 0;

	static Poller* NewDefaultPoller(EventLoop* loop);

	void AssertInLoopThread()
	{
		owner_loop_->AssertInLoopThread();
	}

private:
	EventLoop* owner_loop_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_POLLER_H_
