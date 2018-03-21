#ifndef SHARED_NET_TCPSERVER_H_
#define SHARED_NET_TCPSERVER_H_

#include <map>

#include "shared/base/types.h"
#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/atomic.h"
#include "shared/net/tcpconnection.h"

namespace shared {
namespace net {

class Acceptor;
class EventLoop;

///
/// TCP server, supports single-threaded models. TODO:thread-pool in subclass
///
/// This is an interface class, so don't expose too much details
class TcpServer : noncopyable
{
public:
	TcpServer(EventLoop* loop);
	virtual ~TcpServer(); 

	const std::string acceptor_hostport(int acceptorId);
	const std::string acceptor_name(int acceptorId); 

	/// Start the server if it's not listenning 
	/// 
	/// It's harmless to call it multiple times.
	/// Thread unsafe
	void Start();

	/// Add listen address
	///
	/// Can call many times, but must be called before Start()
	/// Thread safe
	void AddListenAddress(int accept_identify, const InetAddress& listenAddr, const std::string& nameArg);
	bool RemoveListen(int accept_identify);

	virtual void NewConnection(int acceptor_identify,int sockfd, const InetAddress& peerAddr);
	virtual void OnStart() { }

	// delete conn object in queueloop
	void RemoveConnection(TcpConnection* conn);


// for derived class
protected:
	EventLoop*				loop_; // only one loop in a TcpServer
	AtomicInt32				next_conn_id_;


private:
	class HostListenAcceptorInfo
	{
	  public:
		HostListenAcceptorInfo(int identify,
				               const InetAddress& addr,
							   const std::string& name,
							   Acceptor* acceptor)
			: acceptor_identify_(identify),
			  addr_(addr),
			  name_(name),
			  pacceptor_(acceptor)
		{ }

		int			acceptor_identify_;
		InetAddress addr_;
        std::string	name_;
		Acceptor*	pacceptor_;
	};


private:
	static void RegisterNewConnection(void*, int, int, const InetAddress&);
	static void RegisterRemoveConnInQueueLoop(void*);

	void DeleteConnObject();

	typedef std::map<std::string, TcpConnection*>		ConnectionMap;
	typedef std::map<int, HostListenAcceptorInfo*>	HostListenAcceptorMap;

	bool					started_;
	ConnectionMap			connections_;
	HostListenAcceptorMap	acceptors_;

	MutexLock				mutex_;
	std::vector<TcpConnection*> conn_to_be_deleted_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_TCPSERVER_H_
