#ifndef SHARED_NET_NETWORK_H
#define SHARED_NET_NETWORK_H

#include <stdint.h>

namespace shared
{
namespace net
{

class EventLoop;
class SessionManager;
class InetAddress;

class NetWork
{
public:
	static void Init(EventLoop* ploop);
	static void Server(SessionManager* pmanager);
	static void Client(SessionManager* pmanager);
	static void Server(SessionManager* pmanager, const char* ip, int32_t port);
	static void Client(SessionManager* pmanager, const char* ip, int32_t port);

	static void RunLoop();
	static void QuitLoop();

	static EventLoop* GetEventLoop();

private:
	static void RegisterNewConnection(void* pdata, int32_t managerid, int32_t sockfd, const InetAddress& peeraddr);
	static void RegisterNewConnection(void* pdata, int32_t sockfd);
	static InetAddress GetManagerAddr(SessionManager* pmanager);
	static void AddServer(SessionManager* pmanager, const InetAddress& addr);
	static void AddClient(SessionManager* pmanager, const InetAddress& addr);

private:
	static EventLoop* s_peventloop_;
};


} // net
} // namespace shared

#endif // SHARED_NET_NETWORK_H
