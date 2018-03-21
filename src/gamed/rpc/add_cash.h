#ifndef GAMED_RPC_ADD_CASH_H_
#define GAMED_RPC_ADD_CASH_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/add_cash.pb.h"


namespace gamed {

class AddCashProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(AddCashProxyStub, masterRpc, AddCashProxy, HandleAddCashProxy);
};

} // namespace gamed

#endif // GAMED_RPC_ADD_CASH_H_
