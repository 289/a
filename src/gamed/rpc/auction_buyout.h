#ifndef GAMED_RPC_AUCTION_BUYOUT_H_
#define GAMED_RPC_AUCTION_BUYOUT_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/auction_buyout.pb.h"


namespace gamed {

class AuctionBuyoutProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(AuctionBuyoutProxyStub, masterRpc, AuctionBuyoutProxy, HandleAuctionBuyoutProxy);
};

} // namespace gamed

#endif // GAMED_RPC_AUCTION_BUYOUT_H_
