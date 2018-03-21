#ifndef GAMED_RPC_AUCTION_BID_H_
#define GAMED_RPC_AUCTION_BID_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/auction_bid.pb.h"


namespace gamed {

class AuctionBidProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(AuctionBidProxyStub, masterRpc, AuctionBidProxy, HandleAuctionBidProxy);
};

} // namespace gamed

#endif // GAMED_RPC_AUCTION_BID_H_
