#ifndef GAMED_RPC_AUCTION_START_H_
#define GAMED_RPC_AUCTION_START_H_

#include "shared/net/protobuf/rpc/rpc_util.h"
#include "shared/net/protobuf/rpc/rpc_stub.h"
#include "common/rpc/gen/masterd/auction_start.pb.h"


namespace gamed {

class AuctionStartProxyStub : public shared::net::ServiceStub
{
	DEFINE_RPC_STUB(AuctionStartProxyStub, masterRpc, AuctionStartProxy, HandleAuctionStartProxy);
};

} // namespace gamed

#endif // GAMED_RPC_AUCTION_START_H_
