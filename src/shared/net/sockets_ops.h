#ifndef SHARED_NET_SOCKETSOPS_H
#define SHARED_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace shared {
namespace net {
namespace sockets 
{
	/// Creates a non-blocking socket file descriptor,
	/// abort if any error.
	int CreateNonblockingOrDie();

	int  Connect(int sockfd, const struct sockaddr_in& addr);
	void BindOrDie(int sockfd, const struct sockaddr_in& addr);
	void ListenOrDie(int sockfd);
	int  Accept(int sockfd, struct sockaddr_in* addr);
	ssize_t Read(int sockfd, void *buf, size_t count);
	ssize_t Readv(int sockfd, const struct iovec *iov, int iovcnt);
	ssize_t Write(int sockfd, const void *buf, size_t count);
	void Close(int sockfd);
	void ShutdownWrite(int sockfd);

	void ToIpPort(char* buf, size_t size,
			const struct sockaddr_in& addr);
	void ToIp(char* buf, size_t size,
			const struct sockaddr_in& addr);
	void FromIpPort(const char* ip, uint16_t port,
			struct sockaddr_in* addr);

	int get_socket_error(int sockfd);

	struct sockaddr_in get_local_addr(int sockfd);
	struct sockaddr_in get_peer_addr(int sockfd);
	bool is_self_connect(int sockfd);

} // namespace sockets
} // namespace net
} // namespace shared

#endif  // MUDUO_NET_SOCKETSOPS_H
