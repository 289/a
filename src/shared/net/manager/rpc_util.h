#ifndef SHARED_NET_MANAGER_RPC_UTIL_H
#define SHARED_NET_MANAGER_RPC_UTIL_H

namespace shared
{
namespace net
{

// 通用RPC错误码
// 要求各回复消息的0为成功，1为DB_BUSY
enum RpcErrorCode
{
	RPC_SUCC,
	RPC_DB_BUSY
};

inline int16_t GetRpcErrCode(int16_t msg_err, int32_t err)
{
	return msg_err != RPC_SUCC || err == rpc::kTimeout ? RPC_DB_BUSY : RPC_SUCC;
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_MANAGER_RPC_UTIL_H
