#ifndef SHARED_NET_TCPCONNECTION_H_
#define SHARED_NET_TCPCONNECTION_H_

#include "shared/base/stringpiece.h"
#include "shared/base/types.h"
#include "shared/base/noncopyable.h"

#include "shared/net/buffer.h"
#include "shared/net/inetaddress.h"
#include "shared/net/eventloop.h"


namespace shared {
namespace net {

class Channel;
class Socket;

///
/// Tcp connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : noncopyable
{
public:
	typedef void (*WriteCompleteCallback)(const TcpConnection&);
	typedef void (*HighWaterMarkCallback)(const TcpConnection&, size_t); 

	/// Constructs a TcpConnection with a connected sockfd
	///
	/// User should not create this object.
	TcpConnection(EventLoop* loop,
			      int32_t connid,
				  const std::string& name,
				  int sockfd,
				  const InetAddress& localAddr,
				  const InetAddress& peerAddr);
	virtual ~TcpConnection();

	EventLoop*		get_loop() const   { return loop_; }
	bool			connected() const  { return kConnected == state_; }
	const std::string& name() const    { return name_; }
	const InetAddress& local_address() { return localaddr_; }
	const InetAddress& peer_address()  { return peeraddr_; }

	void Send(const void* message, size_t len, bool immediately = true);
	void Send(const StringPiece& message, bool immediately = true);
	void Send(Buffer* message, bool immediately = true); // this one will swap data
	void FlushSendBuffer();
	
	/**
	 * @brief Shutdown 
	 *    1.除了TcpClient及TcpServer，其他地方想关闭这个连接，调用该函数
	 *    2.NOT thread safe, no simultaneous calling
	 */
	void Shutdown();

	/**
	 * @brief OnConnected 
	 *    1.连接已建立，可以开始操作
	 */
	virtual void OnConnected(void);

	/**
	 * @brief OnRecv 
	 *    1.收到数据
	 * @param pbuf
	 *    1.receive data buffer 
	 */
	virtual void OnRecv(Buffer* pbuf);

	/**
	 * @brief OnClose 
	 *    1. 对端主动关闭（recv = 0），或者调用Shutdown()关闭该连接时调用OnClose, 或者连接出现error
	 *    2.表示连接已经关闭，OnClose需要删除本connection的对象
	 */
	virtual void OnClose(void);

	/**
	 * @brief OnDestroy 
	 *    1. 表示manager类正在关闭这个连接（不是已经关闭）,在loop线程可以保证
	 *       子类的OnDestroy里对该connection的操作有效，比如send数据给对端.
	 *    2. 该函数里不需要删除本connection的对象
	 *    3. destroyed by TcpClient or TcpServer, no need to delete connection obj in OnDestroy()
	 */
	virtual void OnDestroy(void); 

	/**
	 * @brief ConnectEstablished 
	 *    1.连接建立时由TcpClient,TcpServer调用
	 *    2.called when TcpServer accepts a new connection
	 *    3.should be called only once
	 */
	void ConnectEstablished();

	/**
	 * @brief ConnectDestroyed 
	 *    1.连接销毁时（包括主动和被动关闭）由TcpClient,TcpServer调用
	 *    2.called when TcpServer has removed me form its map
	 *    3.should be called only once 
	 */
	void ConnectDestroyed();


	void SetTcpNoDelay(bool on);

	void set_context(const void* context)
	{ context_ = context; }

	const void* get_context() const
	{ return context_; }

	void* get_mutable_context()
	{ return const_cast<void*>(context_); }

	void set_high_water_mark(size_t high_water_mark) { high_water_mark_ = high_water_mark; } 

	// set callback func
	void set_write_complete_cb(WriteCompleteCallback cb) { write_complete_cb_ = cb; }
	void set_high_water_mark_cb(HighWaterMarkCallback cb) { high_water_mark_cb_ = cb; }

	Buffer* get_input_buffer()
	{ return &input_buffer_; }

	inline int32_t get_conn_id() const { return conn_id_; }


  // for derived class
protected:
	EventLoop*	loop_;


private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void HandleRead();
	void HandleWrite();
	void HandleClose();
	void HandleError();

	static void* RegisterHandleRead(void*);
	static void* RegisterHandleWrite(void*);
	static void* RegisterHandleClose(void*);
	static void* RegisterHandleError(void*);

	static void  RegisterSendReady(void*);
	static void  RegisterShutdownInLoop(void*);

	void SendReadyInLoop();
	void SendInLoop(const StringPiece& message);
	void SendInLoop(const void* message, size_t len);
	void ShutdownInLoop();
	void SetState(StateE s) { state_ = s; }
	bool FlushWaitingBuffer();


private:
	const int32_t conn_id_; // connection id
    std::string	  name_;
	StateE		  state_;   // FIXME: use atomic variable
	// we don't expose those classes to client.
	Socket*		  socket_;
	Channel*	  channel_;

	InetAddress   localaddr_;
	InetAddress   peeraddr_;
	
	size_t		high_water_mark_;
	Buffer		input_buffer_;
	Buffer		output_buffer_; // FIXME: use list<Buffer> as output buffer.
	Buffer      waitingToSend_buffer_;
	const void* context_;
	MutexLock   waitingbuffer_mutex_;

	// FIXME: callback below run in QueueInLoop() is not safe
	WriteCompleteCallback write_complete_cb_;
	HighWaterMarkCallback high_water_mark_cb_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_TCPCONNECTION_H_
