#ifndef GAMED_RPC_SAVE_PLAYERDATA_H_
#define GAMED_RPC_SAVE_PLAYERDATA_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/save_playerdata.pb.h"


namespace gamed {

using namespace shared::net;

class SavePlayerDataProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(SavePlayerDataProxyStub, masterRpc, SavePlayerDataProxy, HandleSavePlayerDataProxy);
};

} // namespace gamed

#endif // GAMED_RPC_SAVE_PLAYERDATA_H_
