#ifndef SHARED_NET_TCPCLIENT_H_
#define SHARED_NET_TCPCLIENT_H_

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"
#include "shared/base/atomic.h"
#include "shared/net/tcpconnection.h"
#include "shared/net/connector.h"


namespace shared {
namespace net {

class Connector;

class TcpClient 
{
public:
	typedef int32_t (*NextConnIdCallback)();

	TcpClient(EventLoop* loop,
			  const InetAddress& server_addr,
			  const std::string& name,
			  NextConnIdCallback next_conn_id_cb = NULL);
	virtual ~TcpClient(); // force out-line dtor

	void Connect();
	void Stop(); // disconnect tcpconnection, and stop connector

	TcpConnection* connection() const
	{
		MutexLockGuard lock(mutex_);
		return connection_;
	}

	void Disconnect();
	bool Retry() const;
	void EnableRetry();

	// Not thread safe, but in loop
	virtual void NewConnection(int sockfd);
	// Not thread safe, but in loop
	virtual void RemoveConnection(TcpConnection* conn);


protected:
	int32_t GetNextConnId();


protected:
	EventLoop*		   loop_;
	const std::string  name_;	
	// always in loop thread
	AtomicInt32		   next_conn_id_;
	mutable MutexLock  mutex_;
	TcpConnection*	   connection_; // @Guarded By mutex_
	Connector*		   connector_;
	bool			   retry_; // atomic
	bool			   connect_; //atomic
	NextConnIdCallback next_conn_id_cb_;


private:
	static void RegisterNewConnection(void* pdata, int sockfd);
	static void RegisterRemoveConnInQueueLoop(void*);

	void DeleteConnObject();

	std::vector<TcpConnection*> conn_to_be_deleted_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_TCPCLIENT_H_
