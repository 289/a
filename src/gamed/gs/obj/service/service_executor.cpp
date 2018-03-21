#include "service_executor.h"

#include "gamed/client_proto/G2C_error.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gamed/client_proto/service_param.h"
#include "gamed/game_module/task/include/task.h"

#include "gs/item/item.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/player.h"
#include "gs/player/player_sender.h"
#include "gs/player/task_if.h"
#include "gs/template/data_templ/enhance_group_templ.h"
#include "gs/template/data_templ/templ_manager.h"


namespace gamed {

using namespace serviceDef;
using namespace dataTempl;

///
/// ServiceExecutor
///
ServiceExecutor::ServiceExecutor(int type)
	: type_(type)
{
}

ServiceExecutor::~ServiceExecutor()
{
}

bool ServiceExecutor::ServeRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	return SendRequest(pplayer, provider, buf, size);
}

bool ServiceExecutor::Serve(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	return OnServe(pplayer, provider, buf, size);
}

bool ServiceExecutor::CheckRequestParam(const void* buf, size_t size) const
{
	return CheckSrvParam(buf, size);
}


///
/// DeliverTaskExecutor
///
bool DeliverTaskExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	deliver_task param;
	if (!serviceDef::UnpackSrvParam(buf, size, param))
		return false;

	return true;
}

bool DeliverTaskExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	if (!CheckSrvParam(buf, size))
		return false;

	pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, buf, size);
	return true;
}

bool DeliverTaskExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	deliver_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	PlayerTaskIf task_if(pplayer);
	if (task::DeliverTask(&task_if, req.task_id) == 0)
	{
		__PRINTF("%ld接到任务..........task_id:%d", pplayer->role_id(), req.task_id);
	}
	return true;
}


///
/// RecycleTaskExecutor
///
bool RecycleTaskExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	recycle_task param;
	if (!serviceDef::UnpackSrvParam(buf, size, param))
		return false;

	return true;
}

bool RecycleTaskExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	if (!CheckSrvParam(buf, size))
		return false;

	pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, buf, size);
	return true;
}

bool RecycleTaskExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	recycle_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	PlayerTaskIf task_if(pplayer);
	task::DeliverAward(&task_if, req.task_id, req.choice);
	__PRINTF("%ld完成任务..........task_id:%d", pplayer->role_id(), req.task_id);
	return true;
}

///
/// ShopExecutor
///
bool ShopExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	shop_shopping param;
	if (!serviceDef::UnpackSrvParam(buf, size, param))
		return false;

	return true;
}

bool ShopExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	if (!CheckSrvParam(buf, size))
		return false;

	if (!pplayer->HasSlot(Item::INVENTORY, 1))
		return false;

	shop_shopping req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

    req.goods_remains = 0;
    req.goods_item_id = 0;
    req.player_money = pplayer->GetMoney();
    req.player_cash = pplayer->GetCash();
    req.player_score = pplayer->GetScore();
    req.money_cost = 0;
    req.cash_cost = 0;
    req.score_cost = 0;

    std::string out_buf;
    serviceDef::PackSrvParam(req, out_buf);


	//*(int32_t*)((char*)buf + offsetof(shop_shopping, goods_remains)) = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, goods_item_id)) = 0;
	//*(int64_t*)((char*)buf + offsetof(shop_shopping, player_money))  = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, player_cash))   = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, player_score))  = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, money_cost))    = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, cash_cost))     = 0;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, score_cost))    = 0;

	//shop_shopping req2;
	//ASSERT(serviceDef::UnpackSrvParam(buf, size, req2));

	//ShopProvider那边需要用到玩家的money和cash
	//*(int64_t*)((char*)buf + offsetof(shop_shopping, player_money))  = pplayer->GetMoney();
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, player_cash))   = pplayer->GetCash();
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, player_score))  = pplayer->GetScore();

	//shop_shopping req3;
	//ASSERT(serviceDef::UnpackSrvParam(buf, size, req3));

	pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, out_buf.c_str(), out_buf.size());
	//pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, buf, size);
	return true;
}

bool ShopExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	shop_shopping req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	//再次检查金钱和元宝
	if (pplayer->GetMoney() < req.money_cost)
	{
		__PRINTF("玩家金钱不足(%ld, %d)", pplayer->GetMoney(), req.money_cost);
		pplayer->sender()->ErrorMessage(G2C::ERR_NO_MONEY);
		return false;
	}

	if (pplayer->GetCash() < req.cash_cost)
	{
		__PRINTF("玩家元宝不足(%d, %d)", pplayer->GetCash(), req.cash_cost);
		pplayer->sender()->ErrorMessage(G2C::ERR_NO_CASH);
		return false;
	}

	if (!pplayer->CheckScore(req.score_cost))
	{
		__PRINTF("玩家学分不足(%ld, %d)", pplayer->GetScore(), req.score_cost);
		pplayer->sender()->ErrorMessage(G2C::ERR_NO_SCORE);
		return false;
	}

	//扣除金钱,元宝和学分
	if (req.money_cost > 0)
		pplayer->SpendMoney(req.money_cost);
	if (req.cash_cost > 0)
		pplayer->UseCash(req.cash_cost);
	if (req.score_cost > 0)
		pplayer->SpendScore(req.score_cost);

	//物品进包裹
	pplayer->GainItem(req.goods_item_id, req.goods_count);

	GLog::log("gshop_trade:roleid:%ld, item_id:%d, item_count:%d, cash_used:%d, money_used:%d, score_used:%d, timestamp:%d",
			pplayer->role_id(), req.goods_item_id, req.goods_count, req.cash_cost, req.money_cost, req.score_cost, g_timer->GetSysTime());

	G2C::ShopShopping_Re packet;
	packet.result = G2C::ShopShopping_Re::ERR_SUCCESS;
	packet.goods_id = req.goods_id;
	packet.goods_count = req.goods_count;
	packet.goods_remains = req.goods_remains;
	pplayer->sender()->SendCmd(packet);

	return true;
}


///
/// StorageTaskExecutor
///
bool StorageTaskExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	storage_task param;
	if (!serviceDef::UnpackSrvParam(buf, size, param))
		return false;

	return true;
}

bool StorageTaskExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	if (!CheckSrvParam(buf, size))
		return false;

	pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, buf, size);
	return true;
}

bool StorageTaskExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	storage_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	// 检查是不是这个任务库里的任务
	if (!task::IsStorageTask(req.storage_id, req.task_id))
	{
		__PRINTF("%ld发来错误请求，任务库:%d里没有任务:%d", 
				 pplayer->role_id(), req.storage_id, req.task_id);
		return false;
	}

	// 检查通过给玩家发任务
	PlayerTaskIf task_if(pplayer);
	if (task::DeliverTask(&task_if, req.task_id) == 0)
	{
		__PRINTF("%ld接到库任务:%d..........task_id:%d", 
				 pplayer->role_id(), req.storage_id, req.task_id);
	}
	return true;
}


///
/// EnhanceExecutor
///
bool EnhanceExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	do_enhance param;
	if (!serviceDef::UnpackSrvParam(buf, size, param))
		return false;

	return true;
}

bool EnhanceExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	if (!CheckSrvParam(buf, size))
		return false;

    // 没有可附魔的附魔位
    if (pplayer->GetUnProtectSlotNum() == 0)
        return false;

    do_enhance req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

    // 该附魔组中已经没有可用的附魔
    if (pplayer->RandSelectEnhanceId(req.enhance_gid) == 0)
        return false;

    const EnhanceGroupTempl* gtempl = s_pDataTempl->QueryDataTempl<EnhanceGroupTempl>(req.enhance_gid);
    // 优先消耗学分
    req.cost_type = gtempl->score == 0;
    req.player_own = req.cost_type == 0 ? pplayer->GetScore() : pplayer->GetMoney();
    req.protect_slot_num = pplayer->GetProtectSlotNum();

    std::string out_buf;
    serviceDef::PackSrvParam(req, out_buf);

	pplayer->SendMsg(GS_MSG_SERVICE_REQUEST, provider, type_, out_buf.c_str(), out_buf.size());
	return true;
}

bool EnhanceExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	do_enhance req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

    // 再次检查学分
    if (req.cost_type == 0)
    {
        if (!pplayer->CheckScore(req.enhance_cost))
        {
            __PRINTF("玩家学分不足(%ld, %d)", pplayer->GetScore(), req.enhance_cost);
            pplayer->sender()->ErrorMessage(G2C::ERR_NO_SCORE);
            return false;
        }
        else
        {
		    pplayer->SpendScore(req.enhance_cost);
        }
    }
    else
    {
        if (!pplayer->CheckMoney(req.enhance_cost))
        {
            __PRINTF("玩家金币不足(%ld, %d)", pplayer->GetMoney(), req.enhance_cost);
            pplayer->sender()->ErrorMessage(G2C::ERR_NO_MONEY);
            return false;
        }
        else
        {
		    pplayer->SpendMoney(req.enhance_cost);
        }
    }

	// 附魔服务
    pplayer->DoEnhance(req.enhance_gid, req.count);
	return true;
}


///
/// SimpleServiceExecutor
///
bool SimpleServiceExecutor::CheckSrvParam(const void* buf, size_t size) const
{
	return true;
}

bool SimpleServiceExecutor::SendRequest(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	return true;
}

bool SimpleServiceExecutor::OnServe(Player* pplayer, const XID& provider, const void* buf, size_t size) const
{
	return true;
}

} // namespace gamed
