#ifndef SHARED_NET_PACKET_CODEC_PACKET_H_
#define SHARED_NET_PACKET_CODEC_PACKET_H_

#include "shared/base/noncopyable.h"

#include "shared/net/buffer.h"
#include "shared/net/tcpconnection.h"
#include "shared/net/packet/proto_packet.h"


namespace shared {
namespace net {

class ProtoPacket;

class ProtoPacketCodec : noncopyable
{
public:
	const static int kHeaderLen		= sizeof(int32_t);
	const static int kPacketTypeLen = sizeof(uint16_t);
	const static int kCheckSumLen   = sizeof(int32_t);
	const static int kMinPacketLen  = kPacketTypeLen + kCheckSumLen; // type_number + checkSum(为数据部分长度，不包含前面的包头)
	const static int kMaxPacketLen  = 512*1024; // 512k

	enum ErrorCode
	{
		kNoError = 0,
		kInvalidLength,
		kCheckSumError,
		kInvalidPacketType,
		kUnknownPacketType,
		kParseError,
		kUnmarshalError,
		kMarshalError,
	};

	typedef ProtoPacket* PacketPtr;

	typedef void (*ErrorCallback)(shared::net::TcpConnection*,
			                      shared::net::Buffer*,
								  ErrorCode);

	explicit ProtoPacketCodec(const ErrorCallback& errorCb = DefaultErrorCallback)
		        : error_callback_(errorCb), use_defaultpacket_(false)
	{ }

	int ParsePackets(shared::net::TcpConnection* conn,
			          shared::net::Buffer* buf,
			          std::vector<PacketPtr>& vecPacketPtr);

	static bool MarshalPacket(shared::net::ProtoPacket &packet);
	static PacketPtr UnmarshalPacket(uint16_t cmd_type_no, const char* buf, int len);

	static void Send(shared::net::TcpConnection* conn,
			  shared::net::ProtoPacket& packet);
	// for linkserver: transmit packets between client and gs(or master)
	void SendWithoutMarshal(shared::net::TcpConnection* conn,
		                    shared::net::ProtoPacket& packet);
	static void Send(shared::net::TcpConnection* conn, uint16_t cmd_type_no, const char* buf, int len);


	static const std::string& ErrorCodeToString(ErrorCode errorCode);
	static void FillEmptyBuffer(shared::net::Buffer* buf, const shared::net::ProtoPacket& packet);
	static void FillEmptyBuffer(shared::net::Buffer* packet_buf, uint16_t cmd_type_no, const char* buf, int len);
	virtual PacketPtr Parse(const char* buf, int len, ErrorCode* errorCode);

	void SetServerType(PacketManager::ServerType type)
	{
		type_ = type;
	}
	void UseDefaultPacket(bool use)
	{
		use_defaultpacket_ = use;
	}

protected:
	static PacketPtr CreateProtoPacket(ProtoPacket::Type typenum);
	int32_t      AsInt32(const char* buf);
	int16_t      AsInt16(const char* buf);


private:
	static void DefaultErrorCallback(shared::net::TcpConnection*,
			                         shared::net::Buffer*,
								     ErrorCode);
	ErrorCallback error_callback_;
	bool use_defaultpacket_;
	PacketManager::ServerType type_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_CODEC_PACKET_H_
