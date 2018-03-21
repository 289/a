#ifndef SHARED_NET_POLLER_EPOLL_POLLER_H_
#define SHARED_NET_POLLER_EPOLL_POLLER_H_

#include <map>
#include <vector>

#include "shared/net/poller.h"

struct epoll_event;

namespace shared {
namespace net {

///
/// IO Multiplexing with epoll(4)
///
class EpollPoller : public Poller
{
public:
	EpollPoller(EventLoop* loop);
	virtual ~EpollPoller();

	virtual int  Poll(int timeoutMs, ChannelList* active_channels);
	virtual void UpdateChannel(Channel* channel);
	virtual void RemoveChannel(Channel* channel);

private:
	static const int kInitEventListSize = 16;

	void	FillActiveChannels(int num_events, ChannelList* active_channels) const;
	void	Update(int operation, Channel* channel);

	typedef std::vector<struct epoll_event> EventList;
	typedef std::map<int, Channel*> ChannelMap;

	int			epollfd_;
	EventList	events_;
	ChannelMap	channels_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_POLLER_EPOLL_POLLER_H_
