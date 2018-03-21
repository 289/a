#ifndef GAMED_GS_PLAYER_MASTER_RPC_H_
#define GAMED_GS_PLAYER_MASTER_RPC_H_

#include "shared/net/protobuf/rpc/rpc_service.h"

#include "player_def.h"


namespace shared {
namespace net {
	class Buffer;
} // namespace net
} // namespace shared;


namespace masterRpc {
	class SavePlayerDataProxyResponse;
	class ChangeNameProxyRequest;
	class ChangeNameProxyResponse;
    class AuctionStartProxyResponse;
    class AuctionCancelProxyResponse;
    class AuctionBuyoutProxyResponse;
    class AuctionBidProxyResponse;
    class AddCashProxyResponse;
} // namespace masterRpc


namespace gamed {

using namespace shared::net;

class Player;
class MasterRpcProxy
{
public:
	MasterRpcProxy(RoleID roleid, Player& player);
	~MasterRpcProxy();

	void    TimeoutHeartbeat(time_t cur_time);
	void    RpcMessageProc(const shared::net::RpcMessage& msg);

	// rpc implement func
	void    SavePlayerData(const shared::net::Buffer& playerdata);
	void    LogoutSaveData(const shared::net::Buffer& playerdata);
	void    ChangeName(playerdef::NameType name_type, const std::string& name);
	void    TaskChangeName(int32_t taskid, playerdef::NameType name_type, const std::string& name);

	// callback func
	void    SavePlayerDataCB(const masterRpc::SavePlayerDataProxyResponse& response,
			                 shared::net::RPCErrorCode err, 
							 const ServiceStub* pstub);

	void    LogoutSaveDataCB(const masterRpc::SavePlayerDataProxyResponse& response,
  			                 shared::net::RPCErrorCode err, 
							 const ServiceStub* pstub);

	void    ChangeNameCB(const masterRpc::ChangeNameProxyResponse& response,
			             shared::net::RPCErrorCode err,
						 const ServiceStub* pstub);

	// invoke when error happen
	void    LogoutWithoutSaveData();

    // auction rpc
    void    AuctionItem(const playerdef::AuctionItemData& data);
    void    AuctionItemCB(const masterRpc::AuctionStartProxyResponse& response,
                          shared::net::RPCErrorCode err,
                          const ServiceStub* pstub);

    void    AuctionCancel(int64_t auction_id);
    void    AuctionCancelCB(const masterRpc::AuctionCancelProxyResponse& response,
                            shared::net::RPCErrorCode err,
                            const ServiceStub* pstub);

    void    AuctionBuyout(int64_t auction_id, int32_t currency_type, int32_t price);
    void    AuctionBuyoutCB(const masterRpc::AuctionBuyoutProxyResponse& response,
                            shared::net::RPCErrorCode err,
                            const ServiceStub* pstub);

    void    AuctionBid(int64_t auction_id, int32_t currency_type, int32_t price);
    void    AuctionBidCB(const masterRpc::AuctionBidProxyResponse& response,
                         shared::net::RPCErrorCode err,
                         const ServiceStub* pstub);

    // cash
    void    AddCashByGame(int32_t cash);
    void    AddCashByGameCB(const masterRpc::AddCashProxyResponse& response,
                            shared::net::RPCErrorCode err,
                            const ServiceStub* pstub);


private:
	bool GetChangeNameRequest(playerdef::NameType name_type, 
			                  const std::string& name, 
							  masterRpc::ChangeNameProxyRequest& request);
	void DoChangeName(playerdef::NameType name_type, const std::string& name, int32_t taskid);


private:
	Player&    player_;
	int        db_save_error_;
	shared::net::RpcClient rpc_client_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_MASTER_RPC_H_
