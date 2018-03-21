#ifndef GAMED_RPC_CHANGE_NAME_H_
#define GAMED_RPC_CHANGE_NAME_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/change_name.pb.h"


namespace gamed {

using namespace shared::net;

class ChangeNameProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(ChangeNameProxyStub, masterRpc, ChangeNameProxy, HandleChangeNameProxy);
public:
	int32_t task_id; // 0表示不是任务改名
};

} // namespace gamed

#endif // GAMED_RPC_CHANGE_NAME_H_
