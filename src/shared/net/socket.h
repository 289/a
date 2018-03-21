#ifndef SHARED_NET_SOCKET_H_
#define SHARED_NET_SOCKET_H_

#include "shared/base/noncopyable.h"

namespace shared {
namespace net {
///
/// TCP networking
///

class InetAddress;

///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delegated to OS.
class Socket : noncopyable
{
public: 
	explicit Socket(int sockfd)
		: sockfd_(sockfd),
		  is_closed_(false)
	{ }

	~Socket();

	int fd() const { return sockfd_; }

	/// abort if address in use
	void BindAddress(const InetAddress& localaddr);
	/// abort if address in use
	void Listen();

	/// On success, returns a non-negative integer that is
	/// a descriptor for the accepted socket, which has been
	/// set to non-blocking and close-on-exec. *peeraddr is assigned.
	/// On error, -1 is returned, and *peeraddr is untouched.
	int Accept(InetAddress* peeraddr);

	void ShutdownWrite();

	/// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
	void SetTcpNoDelay(bool on);

	/// Enable/disable SO_REUSEADDR
	void SetReuseAddr(bool on);

	/// Enable/disable SO_KEEPALIVE
	void SetKeepAlive(bool on);

	// Thread unsafe
	void Close();

private:
	const int	sockfd_;
	bool		is_closed_;
};


} // namespace net
} // namespace shared


#endif // SHARED_NET_SOCKET_H_
