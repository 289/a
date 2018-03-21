#ifndef SHARED_NET_CONNECTOR_H_
#define SHARED_NET_CONNECTOR_H_

#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/callback_bind.h"
#include "shared/net/inetaddress.h"


namespace shared {
namespace net {

class Channel;
class EventLoop;

class Connector : noncopyable
{
public:
	typedef void (*NewConnectionCallback)(void* pdata, int sockfd);
	typedef bind::Callback<void (int)> ErrorCallback;

	Connector(EventLoop* loop,const InetAddress& serverAddr);
	~Connector();

	void	Start();	// can be called in any thread
	void	Restart();	// must be called in loop thread
	void	Stop();		// can be called in any thread

	void    EnableRetry() { retry_ = true; }
	void	DisableRetry() { retry_ = false;}

	void	set_new_connection_callback(const NewConnectionCallback& cb, void* pdata)
	{ 
		new_connect_callback_	= cb; 
		pcb_data_				= pdata;
	}

	void    set_error_callback(const ErrorCallback& cb)
	{
		error_cb_ = cb;
	}
	
	const InetAddress& server_address() const { return server_addr_; }


private:
	enum States { kDisconnected, kConnecting, kConnected };
	static const int kMaxRetryDelayMs;
	static const int kInitRetryDelayMs;

	static void* RegisterHandleWrite(void*);
	static void* RegisterHandleError(void*);
	static void  RegisterResetChannel(void*);
	static void  RegisterStartInLoop(void*);
	static void  RegisterStopInLoop(void*);

	void	set_state(States s) { state_ = s; }
	void	StartInLoop();
	void    StopInLoop();
	void	Connect();
	void	Connecting(int sockfd);
	void	HandleWrite();
	void	HandleError();
	void	Retry(int sockfd);
	int		RemoveAndResetChannel();
	void	ResetChannel();
	void    ErrorHappen();

	EventLoop*		loop_;
	InetAddress		server_addr_;
	bool			connect_; // atomic
	States			state_; // FIXME: use atomic variable
	Channel*		pchannel_;
	int				retry_delay_ms_;
	NewConnectionCallback new_connect_callback_;
	void*			pcb_data_;
	bool            retry_;
	ErrorCallback   error_cb_;

	std::vector<Channel*> channel_to_delete_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_CONNECTOR_H_
