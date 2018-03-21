#ifndef GAMED_RPC_AUCTION_CANCEL_H_
#define GAMED_RPC_AUCTION_CANCEL_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/auction_cancel.pb.h"


namespace gamed {

class AuctionCancelProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(AuctionCancelProxyStub, masterRpc, AuctionCancelProxy, HandleAuctionCancelProxy);
};

} // namespace gamed

#endif // GAMED_RPC_AUCTION_CANCEL_H_
