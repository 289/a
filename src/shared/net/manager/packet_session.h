#ifndef SHARED_NET_PACKET_SESSION_H
#define SHARED_NET_PACKET_SESSION_H

#include "shared/net/packet/codec_packet.h"
#include "shared/net/packet/dispatcher_packet.h"
#include "session.h"

namespace shared
{
namespace net
{

class Security;
class TimedTask;

class PacketSession : public Session
{
public:
	PacketSession(EventLoop* loop, int32_t sid, int32_t sockfd, SessionManager* manager);
	virtual ~PacketSession();
	void ChangeStatus(int32_t status);
	void RegisterTimeoutStatus(int32_t status, int32_t timeout_sec);
	void SendProtocol(ProtoPacket& packet);
	void SendRawMsg(uint16_t cmd_type, const char* buf, int32_t len, bool immediately = true);

	inline const Buffer& session_key() const;
protected:
	virtual void OnRecv(Buffer* pbuf);
	virtual void OnStartupRegisterMsgHandler() = 0;

	virtual void PacketDefaultHandler(const PacketRef msg);
	virtual void ClearPacketVec();
	virtual void OnConnected();
	virtual void OnClose();

	void SetSecurity();
	virtual int32_t GetInitStatus() const = 0;
	virtual void SessionTimeout(const TimedTask* task);
private:
	virtual void StartupRegisterMsgHandler();
	void DestroySecurity();
protected:
	ProtoPacketCodec codec_;
	ProtoPacketDispatcher dispatcher_; 

	typedef std::vector<ProtoPacketCodec::PacketPtr> PacketPtrVec;
	PacketPtrVec vec_packets_;

	Buffer session_key_;
private:
	Security* isecurity_;
	Security* osecurity_;
	Buffer dec_buffer_;
	uint64_t timer_taskid_;
	typedef std::map<int32_t/*status*/, int32_t/*timeout_sec*/> TimeoutStatus;
	TimeoutStatus timeout_status_;
};

inline const Buffer& PacketSession::session_key() const
{
	return session_key_;
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_SESSION_H
