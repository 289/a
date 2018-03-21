#include "master_rpc.h"

#include "shared/net/protobuf/rpc/rpc_service.h"
#include "gamed/rpc/save_playerdata.h"
#include "gamed/rpc/change_name.h"
#include "gamed/rpc/auction_start.h"
#include "gamed/rpc/auction_cancel.h"
#include "gamed/rpc/auction_buyout.h"
#include "gamed/rpc/auction_bid.h"
#include "gamed/rpc/add_cash.h"

#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/netio/netio_if.h"
#include "gs/netmsg/send_to_master.h"
#include "gs/scene/world_man.h"

#include "player.h"
#include "player_sender.h"


namespace gamed {

using namespace shared::net;
using namespace masterRpc;

#define RPCCALL_REQUEST(RpcPrefix, request, callback) \
	ServiceStub* pstub = rpc_client_.CreateStub<RpcPrefix##Stub>(request); \
	if (NULL == pstub) return; \
	rpc_client_.RpcCall<RpcPrefix##Response>(NetIO::GetMasterSidById(player_.master_id()), pstub, callback);

#define RPCCALL_STUB(RpcPrefix, pstub, callback) \
	if (NULL == pstub) return; \
	rpc_client_.RpcCall<RpcPrefix##Response>(NetIO::GetMasterSidById(player_.master_id()), pstub, callback);


///
/// runnable task
///
class LogoutTask : public RunnablePool::Task
{
public:
	LogoutTask(RoleID roleid) 
		: role_id_(roleid) 
	{ }

	virtual void Run()
	{
		// 找到player，并执行下线操作
		Player* pPlayer = Gmatrix::FindPlayerFromMan(role_id_);
		if (NULL == pPlayer)
			return;

		PlayerLockGuard lock(pPlayer);
		if (!pPlayer->IsActived())
			return;

		// 必须是离线状态
		ASSERT(pPlayer->state().IsOffline()) 

		// 从地图中删除
		WorldManager* pManager = Gmatrix::FindWorldManager(pPlayer->world_id(), pPlayer->world_tag());
		if (pManager)
		{
			pManager->RemovePlayer(pPlayer);
		}
		else
		{
			__PRINTF("玩家%ld不在world中，player的world_id:%d", pPlayer->role_id(), pPlayer->world_id());
		}

		Gmatrix::RemovePlayerFromMan(pPlayer);
		Gmatrix::FreePlayer(pPlayer);
	}

private:
	RoleID role_id_;
};


SHARED_STATIC_ASSERT((int8_t)playerdef::NT_FIRST_NAME  == (int8_t)masterRpc::NAME_FIRST);
SHARED_STATIC_ASSERT((int8_t)playerdef::NT_MIDDLE_NAME == (int8_t)masterRpc::NAME_MID);
SHARED_STATIC_ASSERT((int8_t)playerdef::NT_LAST_NAME   == (int8_t)masterRpc::NAME_LAST);
SHARED_STATIC_ASSERT((int8_t)playerdef::NT_MAX         == (int8_t)masterRpc::NAME_TYPE_MAX);


///
/// member func
///
MasterRpcProxy::MasterRpcProxy(RoleID roleid, Player& player)
	: player_(player),
	  db_save_error_(0),
	  rpc_client_(roleid, gmatrixdef::RPCCALL_TYPE_PLAYER, s_pMasterRpcTunnel)
{
}

MasterRpcProxy::~MasterRpcProxy()
{
}

void MasterRpcProxy::TimeoutHeartbeat(time_t cur_time)
{
	shared::Timestamp now;
	now = shared::AddTimestampSecs(now, cur_time);
	rpc_client_.TimeoutHeartbeat(now);
}

void MasterRpcProxy::RpcMessageProc(const shared::net::RpcMessage& msg)
{
	rpc_client_.OnRecvRpcMessage(msg);
}

void MasterRpcProxy::SavePlayerData(const shared::net::Buffer& playerdata)
{
	SavePlayerDataProxyRequest save_player;
    save_player.set_userid(player_.user_id());
	save_player.set_roleid(player_.role_id());
	save_player.set_content(playerdata.peek(), playerdata.ReadableBytes());
	save_player.set_reason(SavePlayerDataProxyRequest::PLAYER_HEARTBEAT);

	RPCCALL_REQUEST(SavePlayerDataProxy, save_player, BIND_MEM_CB(&MasterRpcProxy::SavePlayerDataCB, this));
}

void MasterRpcProxy::SavePlayerDataCB(const SavePlayerDataProxyResponse& response,
			                          RPCErrorCode err, 
								      const ServiceStub* pstub)
{
	if (rpc::kNoError == err)
	{
		if (SavePlayerDataProxyResponse::NO_ERROR == response.error()) 
		{
			db_save_error_ = 0;
		}
		else 
		{
			++db_save_error_;
			GLog::log("玩家:%ld 心跳Rpc存盘错误ErrorCode:%d", player_.role_id(), (int)response.error());
		}
	}
	else // Timeout or other error
	{
		++db_save_error_;
		GLog::log("玩家:%ld 心跳Rpc存盘超时db_save_error：%d", player_.role_id(), db_save_error_);
	}

	if (db_save_error_ >= 3)
	{
		player_.PlayerLogout(LT_DB_SAVE_ERROR);
	}
}

void MasterRpcProxy::LogoutSaveData(const shared::net::Buffer& playerdata)
{
	SavePlayerDataProxyRequest save_player;
    save_player.set_userid(player_.user_id());
	save_player.set_roleid(player_.role_id());
	save_player.set_content(playerdata.peek(), playerdata.ReadableBytes());
	save_player.set_reason(SavePlayerDataProxyRequest::PLAYER_LOGOUT);

	RPCCALL_REQUEST(SavePlayerDataProxy, save_player, BIND_MEM_CB(&MasterRpcProxy::LogoutSaveDataCB, this));
}

void MasterRpcProxy::LogoutSaveDataCB(const masterRpc::SavePlayerDataProxyResponse& response,
  			                          RPCErrorCode err, 
							          const ServiceStub* pstub)
{
	// 如果发生错误，先记log，然后执行后面的下线操作
	if (rpc::kNoError != err)
	{
		GLog::log("玩家:%ld Logout - Rpc存盘错误ErrorCode:%d", player_.role_id(), err);
		if (SavePlayerDataProxyResponse::NO_ERROR != response.error()) 
		{
			GLog::log("玩家:%ld Logout - 存盘失败response.error:%d", player_.role_id(), response.error());
		}
		else
		{
			GLog::log("玩家:%ld Logout - 存盘失败response.error:%d", player_.role_id(), response.error());
		}
	}
	else if (rpc::kTimeout == err)
	{
		LOG_ERROR << "player:" << player_.role_id() << " savedata to DB timeout!";
		GLog::log("玩家:%ld Logout - Rpc存盘超时!", player_.role_id());
	}
	
	RunnablePool::AddTask(new LogoutTask(player_.role_id()));
}

void MasterRpcProxy::LogoutWithoutSaveData()
{
	RunnablePool::AddTask(new LogoutTask(player_.role_id()));
}

bool MasterRpcProxy::GetChangeNameRequest(playerdef::NameType name_type, 
			                              const std::string& name, 
							              masterRpc::ChangeNameProxyRequest& request)
{
	if (name_type != playerdef::NT_FIRST_NAME &&
		name_type != playerdef::NT_MIDDLE_NAME && 
		name_type != playerdef::NT_LAST_NAME)
	{
		LOG_ERROR << "改名rpc传入参数不正确！";
		return false;
	}

	masterRpc::NameType mtype = (masterRpc::NameType)((int8_t)name_type);
	request.set_roleid(player_.role_id());
	request.set_type(mtype);
	request.set_name(name);
	return true;
}

void MasterRpcProxy::ChangeName(playerdef::NameType name_type, const std::string& name)
{
	DoChangeName(name_type, name, 0);
}

void MasterRpcProxy::TaskChangeName(int32_t taskid, playerdef::NameType name_type, const std::string& name)
{
	ASSERT(taskid > 0);
	DoChangeName(name_type, name, taskid);
}

void MasterRpcProxy::DoChangeName(playerdef::NameType name_type, const std::string& name, int32_t taskid)
{
	masterRpc::ChangeNameProxyRequest change_name;
	if (!GetChangeNameRequest(name_type, name, change_name))
	{
		__PRINTF("GetChangeNameRequest() return false! task_id=%d", taskid);
		return;
	}

	ChangeNameProxyStub* pstub = dynamic_cast<ChangeNameProxyStub*>(rpc_client_.CreateStub<ChangeNameProxyStub>(change_name));
	ASSERT(pstub);
	pstub->task_id = taskid;

	RPCCALL_STUB(ChangeNameProxy, pstub, BIND_MEM_CB(&MasterRpcProxy::ChangeNameCB, this));
}

void MasterRpcProxy::ChangeNameCB(const masterRpc::ChangeNameProxyResponse& response,
			                      shared::net::RPCErrorCode err,
						          const ServiceStub* pstub)
{
	if (rpc::kNoError == err)
	{
		player_.sender()->ChangeNameRe(response, false);
		
		// success
		if (response.error() == masterRpc::ChangeNameProxyResponse::NO_ERROR)
		{
			const ChangeNameProxyStub* pcn_stub = dynamic_cast<const ChangeNameProxyStub*>(pstub);
			ASSERT(pcn_stub);
			player_.ChangeNameSuccess((int8_t)(response.type()), response.name(), pcn_stub->task_id);
		}
	}
	else if (rpc::kTimeout == err)
	{
		player_.sender()->ChangeNameRe(response, true);
	}
	else
	{
		LOG_ERROR << "rpc::RPCErrorCode 错误！";
	}
}

void MasterRpcProxy::AuctionItem(const AuctionItemData& data)
{
    AuctionStartProxyRequest auction_start;
    auction_start.set_seller(player_.role_id());
    auction_start.set_currency_type(data.currency_type);
    auction_start.set_buyout_price(data.buyout_price);
    auction_start.set_bid_price(data.bid_price);
    auction_start.set_item_id(data.item_id);
    auction_start.set_item_name(data.item_name);
    auction_start.set_item_info(data.item_info);

    RPCCALL_REQUEST(AuctionStartProxy, auction_start, BIND_MEM_CB(&MasterRpcProxy::AuctionItemCB, this));
}

void MasterRpcProxy::AuctionItemCB(const masterRpc::AuctionStartProxyResponse& response,
                                   shared::net::RPCErrorCode err,
                                   const ServiceStub* pstub)
{
    const AuctionStartProxyStub* tmpstub = dynamic_cast<const AuctionStartProxyStub*>(pstub);
    ASSERT(tmpstub);

	if (rpc::kNoError == err)
    {
        if (response.error() == AuctionStartProxyResponse::NO_ERROR)
        {
            player_.AuctionItemResult(0, response.auction_id(), tmpstub->get_request());
        }
        else
        {
            player_.AuctionItemResult(1, response.auction_id(), tmpstub->get_request());
        }
    }
    else if (rpc::kTimeout == err)
	{
        player_.AuctionItemResult(2, response.auction_id(), tmpstub->get_request());
	}
	else
	{
		LOG_ERROR << "rpc::RPCErrorCode 错误！code:" << err;
	}
}

void MasterRpcProxy::AuctionCancel(int64_t auction_id)
{
    AuctionCancelProxyRequest auction_cancel;
    auction_cancel.set_seller(player_.role_id());
    auction_cancel.set_auction_id(auction_id);

    RPCCALL_REQUEST(AuctionCancelProxy, auction_cancel, BIND_MEM_CB(&MasterRpcProxy::AuctionCancelCB, this));
}

void MasterRpcProxy::AuctionCancelCB(const masterRpc::AuctionCancelProxyResponse& response,
                                     shared::net::RPCErrorCode err,
                                     const ServiceStub* pstub)
{
    const AuctionCancelProxyStub* tmpstub = dynamic_cast<const AuctionCancelProxyStub*>(pstub);
    ASSERT(tmpstub);
    const AuctionCancelProxyRequest* request = tmpstub->get_request();

    if (rpc::kNoError == err)
    {
        player_.AuctionCancelResult(response.error(), request->auction_id(), &response);
    }
    else if (rpc::kTimeout == err)
	{
        player_.AuctionCancelResult(-1, request->auction_id(), &response);
	}
	else
	{
		LOG_ERROR << "rpc::RPCErrorCode 错误！code:" << err;
	}
}

void MasterRpcProxy::AuctionBuyout(int64_t auction_id, int32_t currency_type, int32_t price)
{
    AuctionBuyoutProxyRequest auction_buyout;
    auction_buyout.set_auction_id(auction_id);
    auction_buyout.set_buyer(player_.role_id());
    auction_buyout.set_currency_type(currency_type);
    auction_buyout.set_price(price);

    RPCCALL_REQUEST(AuctionBuyoutProxy, auction_buyout, BIND_MEM_CB(&MasterRpcProxy::AuctionBuyoutCB, this));
}

void MasterRpcProxy::AuctionBuyoutCB(const masterRpc::AuctionBuyoutProxyResponse& response,
                                     shared::net::RPCErrorCode err,
                                     const ServiceStub* pstub)
{
    const AuctionBuyoutProxyStub* tmpstub = dynamic_cast<const AuctionBuyoutProxyStub*>(pstub);
    ASSERT(tmpstub);
    const AuctionBuyoutProxyRequest* request = tmpstub->get_request();

    if (rpc::kNoError == err)
    {
        player_.AuctionBuyoutResult(response.error(), request);
    }
    else if (rpc::kTimeout == err)
    {
        player_.AuctionBuyoutResult(-1, request);
    }
    else
	{
		LOG_ERROR << "rpc::RPCErrorCode 错误！code:" << err;
	}
}

void MasterRpcProxy::AuctionBid(int64_t auction_id, int32_t currency_type, int32_t price)
{
    AuctionBidProxyRequest auction_bid;
    auction_bid.set_auction_id(auction_id);
    auction_bid.set_bidder(player_.role_id());
    auction_bid.set_currency_type(currency_type);
    auction_bid.set_price(price);

    RPCCALL_REQUEST(AuctionBidProxy, auction_bid, BIND_MEM_CB(&MasterRpcProxy::AuctionBidCB, this));
}

void MasterRpcProxy::AuctionBidCB(const masterRpc::AuctionBidProxyResponse& response,
                                  shared::net::RPCErrorCode err,
                                  const ServiceStub* pstub)
{
    const AuctionBidProxyStub* tmpstub = dynamic_cast<const AuctionBidProxyStub*>(pstub);
    ASSERT(tmpstub);
    const AuctionBidProxyRequest* request = tmpstub->get_request();

    if (rpc::kNoError == err)
    {
        player_.AuctionBidResult(response.error(), response.cur_bid_price(), response.cur_bidder(), request);
    }
    else if (rpc::kTimeout == err)
    {
        player_.AuctionBidResult(-1, response.cur_bid_price(), response.cur_bidder(), request);
    }
    else
	{
		LOG_ERROR << "rpc::RPCErrorCode 错误！code:" << err;
	}
}

void MasterRpcProxy::AddCashByGame(int32_t cash)
{
    if (cash <= 0)
    {
        LOG_ERROR << "游戏过程中获得的元宝小于等于0? roleid:" << player_.role_id();
        return;
    }

    AddCashProxyRequest add_cash_req;
    add_cash_req.set_roleid(player_.role_id());
    add_cash_req.set_cash(cash);

    RPCCALL_REQUEST(AddCashProxy, add_cash_req, BIND_MEM_CB(&MasterRpcProxy::AddCashByGameCB, this));

    GLog::log("玩家 %ld 游戏内AddCashByGame获得元宝, cash_add=%d", player_.role_id(), cash);
}

void MasterRpcProxy::AddCashByGameCB(const masterRpc::AddCashProxyResponse& response,
                                     shared::net::RPCErrorCode err,
                                     const ServiceStub* pstub)
{
    const AddCashProxyStub* tmpstub = dynamic_cast<const AddCashProxyStub*>(pstub);
    ASSERT(tmpstub);
    const AddCashProxyRequest* req = tmpstub->get_request();

    if (rpc::kNoError == err)
    {
        if (response.error() == AddCashProxyResponse::NO_ERROR)
        {
            player_.AddCash(req->cash());
        }
        else
        {
            GLog::log("玩家%ld：游戏内获得元宝rpc失败，master返回的errcode=%ld，cash=%d", 
                    player_.role_id(), response.error(), req->cash());
        }
    }
    else if (rpc::kTimeout == err)
    {
        GLog::log("玩家%ld：游戏内获得元宝rpc超时失败，cash=%d", player_.role_id(), req->cash());
    }
    else
    {
		LOG_ERROR << "rpc::RPCErrorCode 错误！code:" << err;
    }
}

} // namespace gamed
