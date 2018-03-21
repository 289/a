#ifndef SHARED_NET_INETADDRESS_H_
#define SHARED_NET_INETADDRESS_H_

#include <netinet/in.h>

#include "shared/base/copyable.h"
#include "shared/base/stringpiece.h"


namespace shared {
namespace net {

///
/// Wrapper of sockaddr_in.
//
//This is an POD interface class
class InetAddress : public shared::copyable
{
public:
	/// Constructs an endpoint with given port number.
	/// Mostly used in TcpServer listening.
	explicit InetAddress(uint16_t port);

	/// Constructs an endpoint with given ip and port.
	/// @c ip should be "1.2.3.4"
	InetAddress(const StringPiece& ip, uint16_t port);

	/// Constructs an endpoint with given struct @c sockaddr_in
	/// Mostly used when accepting new connections
	InetAddress(const struct sockaddr_in& addr)
		: addr_(addr)
	{ }

	/// Mostly used when accepting new connections
	InetAddress(const InetAddress& rhs)
		: addr_(rhs.addr_)
	{ }

	std::string ToIp() const;
	std::string ToIpPort() const;
	std::string ToHostPort() const __attribute__ ((deprecated))
	{ return ToIpPort(); }

	// default copy/assignment are Okay
	
	const struct sockaddr_in& get_sockaddr_inet() const { return addr_; }
	void set_sockaddr_inet(const struct sockaddr_in& addr) { addr_ = addr; }

	uint32_t ip_net_endian() const { return addr_.sin_addr.s_addr; }
	uint16_t port_net_endian() const { return addr_.sin_port; }

	/// static func
	static bool CheckIpAddress(const char* ip_str);

private: 
	struct sockaddr_in addr_;
};


} // namespace net
} // namespace shared


#endif // SHARED_NET_INETADDRESS_H_
