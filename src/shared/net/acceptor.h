#ifndef SHARED_NET_ACCEPTOR_H_
#define SHARED_NET_ACCEPTOR_H_

#include "shared/base/noncopyable.h"
#include "shared/net/socket.h"
#include "shared/net/channel.h"

namespace shared { 
namespace net {

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : noncopyable
{
public:
	typedef void (*NewConnectionCallback)(void* pdata, int acceptorid, int sockfd, const InetAddress& peerAddr);

	Acceptor(EventLoop* loop, const InetAddress& listenAddr);
	~Acceptor();

	void set_new_connection_callback(const NewConnectionCallback& cb, void* pdata)
	{ 
		new_connection_callback_	= cb; 
		pcb_data_					= pdata;
	}

	bool Listenning() const { return listenning_; }
	void Listen();

	// for TcpServer
	int	 index() const { return index_; }
	void set_index(int idx) { index_ = idx; }

private:
	void HandleRead();

	static void* RegisterHandleRead(void*);

	EventLoop*		loop_;
	Socket			accept_socket_;
	Channel			accept_channel_;
	NewConnectionCallback new_connection_callback_;
	bool			listenning_;
	int				idleFd_;
	int				index_; //for TcpServer
	void*			pcb_data_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_ACCEPTOR_H_
