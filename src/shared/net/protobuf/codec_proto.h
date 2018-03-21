#ifndef SHARED_NET_PROTOBUF_CODEC_PROTO_H_
#define SHARED_NET_PROTOBUF_CODEC_PROTO_H_

#include <vector>
#include <google/protobuf/message.h>

#include "shared/base/noncopyable.h"
#include "shared/base/types.h"
#include "shared/net/buffer.h"
#include "shared/net/tcpconnection.h"


namespace shared {
namespace net {

class RpcMessage;
class RoleMessage;

///
/// FIXME: merge with RpcCodec
///
class ProtobufCodec : noncopyable
{
public:
	enum ErrorCode
	{
		kNoError = 0,
		kInvalidLength,
		kCheckSumError,
		kInvalidNameLen,
		kUnknownMessageType,
		kParseError,
	};

	typedef google::protobuf::Message* MessagePtr;
	typedef google::protobuf::Message& MessageRef;

	typedef void (*ErrorCallback)(shared::net::TcpConnection*,
			                      shared::net::Buffer*,
								  ErrorCode);

	explicit ProtobufCodec(const ErrorCallback& errorCb = DefaultErrorCallback)
		: error_callback_(errorCb)
	{ }

	int ParseMessages(shared::net::TcpConnection* conn,
					  shared::net::Buffer* buf,
					  std::vector<MessagePtr>& vecMessagePtr);

	void Send(shared::net::TcpConnection* conn, 
			  const google::protobuf::Message& message)
	{
		// FIXME: serialize to TcpConnection::OutputBuffer();
		shared::net::Buffer buf;
		FillEmptyBuffer(&buf, message);
		conn->Send(&buf);
	}

	static void SendMsg(shared::net::TcpConnection* conn, const google::protobuf::Message& message);
	static void SendRPC(TcpConnection* conn, const RpcMessage& rpcmessage);

	static const std::string& ErrorCodeToString(ErrorCode errorCode);
	static void FillEmptyBuffer(shared::net::Buffer* buf, const google::protobuf::Message& message);
	static google::protobuf::Message* CreateMessage(const std::string& type_name);
	static MessagePtr Parse(const char* buf, int len, ErrorCode* errorCode);

private:
	static void DefaultErrorCallback(shared::net::TcpConnection*,
			                         shared::net::Buffer*,
								     ErrorCode);
	ErrorCallback error_callback_;

	const static int kHeaderLen		= sizeof(int32_t);
	const static int kNamelenLen    = sizeof(int32_t);
	const static int kCheckSumLen   = sizeof(int32_t);
	const static int kMinMessageLen = kHeaderLen + 2 + kCheckSumLen; // nameLen + typeName + checkSum
	const static int kMaxMessageLen = 64*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_PROTOBUF_CODEC_PROTO_H_
