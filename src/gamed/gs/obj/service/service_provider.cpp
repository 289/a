#include <time.h>
#include "service_provider.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/G2C_proto.h"
#include "gamed/client_proto/G2C_error.h"
#include "gamed/client_proto/service_param.h"
#include "gamed/game_module/task/include/task.h"

#include "gs/item/item_manager.h"
#include "gs/template/data_templ/enhance_group_templ.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/shop_templ.h"

#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"
#include "gs/global/game_util.h"
#include "gs/global/message.h"
#include "gs/global/msg_obj.h"
#include "gs/obj/object.h"
#include "gs/netmsg/send_to_link.h"


namespace gamed {

using namespace serviceDef;

///
/// ServiceProvider
///
ServiceProvider::ServiceProvider(int type)
	: type_(type),
	  pobj_(NULL),
	  is_inited_(false)
{
}

ServiceProvider::~ServiceProvider()
{
}

void ServiceProvider::SendCmd(int64_t roleid, int32_t link_id, int32_t client_sid, PacketRef packet) const
{
	NetToLink::SendS2CGameData(link_id, roleid, client_sid, packet);
}

void ServiceProvider::SendMessage(int message, const XID& target, const void* buf, size_t size) const
{
	pobj_->SendMsg(message, target, type_, buf, size);
}

void ServiceProvider::SendMessage(int message, const XID& target, int error, const void* buf, size_t size) const
{
	pobj_->SendMsg(message, target, error, buf, size);
}

void ServiceProvider::SendServiceContent(int64_t roleid, int32_t link_id, int32_t client_sid, const void *buf, size_t size) const
{
	G2C::NpcServiceContent packet;
	packet.npc_id = pobj_->object_id();
	packet.service_type = type_;
	if (buf && size > 0)
	{
		packet.content.resize(size);
		memcpy(packet.content.data(), buf, size);
	}
	SendCmd(roleid, link_id, client_sid, packet);
}

bool ServiceProvider::Init(WorldObject* obj, const serviceDef::ServiceTempl* param_tmpl)
{
	ASSERT(obj != NULL);

	pobj_ = obj;
	if (!OnInit(param_tmpl))
	{
		LOG_ERROR << "ServiceProvider::Init() error! service_type:" << param_tmpl->GetType();
		return false;
	}

	is_inited_ = true;
	return true;
}

void ServiceProvider::PayService(const XID& player, const void* buf, size_t size)
{
	ASSERT(is_inited_);
	TryServe(player, buf, size);
}

void ServiceProvider::GetServiceContent(const XID& player, int link_id, int client_sid)
{
	GetContent(player, link_id, client_sid);
}

///
/// DeliverTaskProvider
///
bool DeliverTaskProvider::OnInit(const serviceDef::ServiceTempl* param)
{
	const DeliverTaskSrvTempl* ptempl = dynamic_cast<const DeliverTaskSrvTempl*>(param);
	if (ptempl == NULL) 
		return false;

	for (size_t i = 0; i < ptempl->deliver_task_list.size(); ++i)
	{
		deliver_task_set_.insert(ptempl->deliver_task_list[i]);
	}
	return true;
}

void DeliverTaskProvider::TryServe(const XID& player, const void* buf, size_t size)
{
	deliver_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

    int32_t top_taskid = task::GetTopTaskID(req.task_id);
	TaskIDSet::iterator it = deliver_task_set_.find(top_taskid);
	if (it != deliver_task_set_.end())
	{
		SendMessage(GS_MSG_SERVICE_DATA, player, buf, size);
		__PRINTF("DeliverTaskProvider TryServe Success task_id:%d", req.task_id);
	}
}


///
/// RecycleTaskProvider
///
bool RecycleTaskProvider::OnInit(const serviceDef::ServiceTempl* param_tmpl)
{
	const RecycleTaskSrvTempl* ptempl = dynamic_cast<const RecycleTaskSrvTempl*>(param_tmpl);
	if (ptempl == NULL)
		return false;

	for (size_t i = 0; i < ptempl->recycle_task_list.size(); ++i)
	{
		recycle_task_set_.insert(ptempl->recycle_task_list[i]);
	}
	return true;
}

void RecycleTaskProvider::TryServe(const XID& player, const void* buf, size_t size)
{
	recycle_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	TaskIDSet::iterator it = recycle_task_set_.find(req.task_id);
	if (it != recycle_task_set_.end())
	{
		SendMessage(GS_MSG_SERVICE_DATA, player, buf, size);
		__PRINTF("RecycleTaskProvider TryServe Success task_id:%d", req.task_id);
	}
}


///
/// ShopProvider
///
bool ShopProvider::OnInit(const serviceDef::ServiceTempl* param_templ)
{
	const ShopSrvTempl* ptempl = dynamic_cast<const ShopSrvTempl*>(param_templ);
	if (ptempl == NULL)
		return false;

	int32_t shop_id = ptempl->shop_id;
	const dataTempl::ShopTempl* tpl = s_pDataTempl->QueryDataTempl<dataTempl::ShopTempl>(shop_id);
	if (!tpl)
		return false;

	for (size_t i = 0; i < tpl->class_list.size(); ++ i)
	{
		const ShopTempl::ClassEntry& cls_entry = tpl->class_list[i];

		for (size_t j = 0; j < cls_entry.goods_list.size(); ++ j)
		{
			const ShopTempl::GoodsEntry& goods_entry = cls_entry.goods_list[j];

			Goods goods;
			goods.item_id           = goods_entry.item_id;
			goods.init_item_count   = goods_entry.init_item_count;
			goods.item_remains      = goods_entry.init_item_count;
			goods.sell_price        = goods_entry.sell_price;
			goods.refresh_rule      = goods_entry.refresh_rule;
			goods.refresh_interval  = goods_entry.refresh_interval;
			goods.refresh_timestamp = g_timer->GetSysTime();

			const ItemDataTempl* item_templ = s_pDataTempl->QueryDataTempl<ItemDataTempl>(goods_entry.item_id);
			ASSERT(item_templ);
			goods.pile_limit = item_templ->pile_limit;

			goods_list_.push_back(goods);

			if (goods.init_item_count > 0)
			{
				++ dyn_goods_count_;
			}
			if (goods.refresh_rule == REFRESH_PER_DAY)
			{
				refresh_list_.push_back(goods_list_.size()-1);
			}
		}
	}

	coin_type_ = tpl->coin_type;
	total_goods_count_ = goods_list_.size();
	BuildTimeSlot(open_time_, tpl->open_time);
	return true;
}

void ShopProvider::TryServe(const XID& player, const void* buf, size_t size)
{
	shop_shopping req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	int32_t goods_idx = req.goods_id;
	int32_t goods_count = req.goods_count;
	if (goods_count < 0 || goods_idx < 0 || goods_idx >= total_goods_count_)
		return;

	Goods& goods = goods_list_[goods_idx];
	if (goods_count > goods.pile_limit)
		return;

#define SEND_SHOPPING_FAILED(err) \
		{ \
			msg_shop_shopping_failed msg; \
			msg.err_code = err; \
			msg.goods_id = goods_idx; \
			msg.goods_count = goods_count; \
			msg.goods_remains = goods.item_remains; \
			SendMessage(GS_MSG_SHOP_SHOPPING_FAILED, player, 0, &msg, sizeof(msg)); \
		} 
	if (!open_time_.IsMatch(g_timer->GetSysTime()))
	{
		SEND_SHOPPING_FAILED(G2C::ShopShopping_Re::ERR_NO_OPEN);
		return;
	}
	if (goods.item_remains == 0)
	{
		SEND_SHOPPING_FAILED(G2C::ShopShopping_Re::ERR_GOODS_SELL_OUT);
		return;
	}
	if (goods.item_remains > 0 && goods_count > goods.item_remains)
	{
		SEND_SHOPPING_FAILED(G2C::ShopShopping_Re::ERR_NO_ENOUGH_GOODS);
		return;
	}
#undef SEND_SHOPPING_FAILED

	int32_t cash_cost  = 0;
	int32_t money_cost = 0;
	int32_t score_cost = 0;
	if (coin_type_ == MT_CASH)
		cash_cost = goods.sell_price * goods_count;
	if (coin_type_ == MT_MONEY)
		money_cost = goods.sell_price * goods_count;
	if (coin_type_ == MT_SCORE)
		score_cost = goods.sell_price * goods_count;
	if (cash_cost <= 0 && money_cost <= 0 && score_cost <= 0)
		return;

	if (money_cost > req.player_money)
	{
		SendMessage(GS_MSG_SERVICE_ERROR, player, G2C::ERR_NO_MONEY, buf, size);
		return;
	}
	if (cash_cost > req.player_cash)
	{
		SendMessage(GS_MSG_SERVICE_ERROR, player, G2C::ERR_NO_CASH, buf, size);
		return;
	}
	if (score_cost > req.player_score)
	{
		SendMessage(GS_MSG_SERVICE_ERROR, player, G2C::ERR_NO_SCORE, buf, size);
		return;
	}

	if (goods.item_remains > 0)
	{
		//如果扣钱失败,这里将出错了!!!
		goods.item_remains -= goods_count;
	}

    req.goods_remains = goods.item_remains;
    req.goods_item_id = goods.item_id;
    req.money_cost = money_cost;
    req.cash_cost = cash_cost;
    req.score_cost = score_cost;

    std::string out_buf;
    serviceDef::PackSrvParam(req, out_buf);
	SendMessage(GS_MSG_SERVICE_DATA, player, out_buf.c_str(), out_buf.size());
	//购物花费记在这里, 到ShopExecutor那边再扣
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, goods_remains)) = goods.item_remains;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, goods_item_id)) = goods.item_id;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, money_cost))    = money_cost;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, cash_cost))     = cash_cost;
	//*(int32_t*)((char*)buf + offsetof(shop_shopping, score_cost))    = score_cost;

	//SendMessage(GS_MSG_SERVICE_DATA, player, buf, size);

	__PRINTF("ShopProvider TryServe Success item_id:%d item_count:%d", goods.item_id, goods_count);
}

void ShopProvider::GetContent(const XID& player, int link_id, int client_sid)
{
	UpdateGoods();

	G2C::NpcServiceContent::shop_srv_content content;
	content.max_goods_id = goods_list_.size();
	content.dyn_goods_count = dyn_goods_count_;
	for (size_t i = 0; i < goods_list_.size(); ++ i)
	{
		const Goods& goods = goods_list_[i];
		if (goods.init_item_count > 0)
		{
			G2C::NpcServiceContent::shop_srv_content::goods_entry entry;
			entry.goods_id = i;
			entry.goods_remains = goods.item_remains;
			content.dyn_goods_list.push_back(entry);
		}
	}

	shared::net::ByteBuffer buffer;
	buffer << *const_cast<G2C::NpcServiceContent::shop_srv_content*>(&content);

	SendServiceContent(player.id, link_id, client_sid, buffer.contents(), buffer.size());
}

void ShopProvider::UpdateGoods()
{
	time_t now = g_timer->GetSysTime();
	for (size_t i = 0; i < refresh_list_.size(); ++ i)
	{
		Goods& goods = goods_list_[refresh_list_[i]];
		if (time_is_overday(goods.refresh_timestamp, now))
		{
			goods.refresh_timestamp = now;
			goods.item_remains = goods.init_item_count;
		}
	}
}


///
/// StorageTaskProvider
///
bool StorageTaskProvider::OnInit(const serviceDef::ServiceTempl* param_tmpl)
{
	const StorageTaskSrvTempl* ptempl = dynamic_cast<const StorageTaskSrvTempl*>(param_tmpl);
	if (ptempl == NULL || ptempl->task_storage_id <= 0) 
		return false;

	task_storage_id_ = ptempl->task_storage_id;
	return true;
}

void StorageTaskProvider::TryServe(const XID& player, const void* buf, size_t size)
{
	storage_task req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

	if (req.storage_id == task_storage_id_)
	{
		// 该任务库里是否有这个任务在Executor里检查
		SendMessage(GS_MSG_SERVICE_DATA, player, buf, size);
		__PRINTF("StorageTaskProvider TryServe Success storage_id:%d, task_id:%d", 
				req.storage_id, req.task_id);
	}
}


///
/// EnhanceProvider
///
static int32_t CalcResetTime(const EnhanceGroupTempl* gtempl)
{
    if (gtempl->reset_type == ENHANCE_RESET_INTERVAL)
    {
        return g_timer->GetSysTime() + gtempl->reset_time;
    }
    else if (gtempl->reset_type == ENHANCE_RESET_TIME)
    {
        time_t now = g_timer->GetSysTime();
        struct tm date;
#ifdef PLATFORM_WINDOWS
        localtime_s(&date, &now);
#else // !PLATFORM_WINDOWS
        localtime_r(&now, &date);
#endif // PLATFORM_WINDOWS

        int8_t wday = 0, hour = 0, min = 0;
        gtempl->ExtractTime(wday, hour, min);
        int32_t delta = 0;
        if (wday == -1)
        {
            delta = 1;
        }
        else if (wday != -1 && date.tm_wday <= wday)
        {
            delta = wday - date.tm_wday;
        }
        else if (wday != -1 && date.tm_wday > wday)
        {
            delta = 8 - date.tm_wday;
        }
        date.tm_mday += delta;
        date.tm_hour = hour;
        date.tm_min = min;
        date.tm_sec = 0;
        return mktime(&date);
    }
    else
    {
        return -1;
    }
}

bool EnhanceProvider::OnInit(const serviceDef::ServiceTempl* param_tmpl)
{
	const EnhanceSrvTempl* ptempl = dynamic_cast<const EnhanceSrvTempl*>(param_tmpl);
	if (ptempl == NULL || ptempl->enhance_gid <= 0) 
		return false;

	enhance_gid_ = ptempl->enhance_gid;

    const EnhanceGroupTempl* gtempl = s_pDataTempl->QueryDataTempl<EnhanceGroupTempl>(enhance_gid_);
    count_ = gtempl->count;
    reset_time_ = CalcResetTime(gtempl);
	return true;
}

void EnhanceProvider::TryServe(const XID& player, const void* buf, size_t size)
{
	do_enhance req;
	ASSERT(serviceDef::UnpackSrvParam(buf, size, req));

    if (req.enhance_gid != enhance_gid_)
    {
        return;
    }

    // 如果玩家已知开着界面那么不会走下面的GetContent来更新附魔的信息
    // 所以在尝试附魔之前需要再进行一次更新
    UpdateEnhanceInfo();

    if (count_ == 0)
    {
        msg_enhance_failed msg;
        msg.err_code = G2C::EnhanceRe::ERR_ENHANCE_NO_COUNT;
        SendMessage(GS_MSG_ENHANCE_FAILED, player, 0, &msg, sizeof(msg));
        return;
    }

    const EnhanceGroupTempl* gtempl = s_pDataTempl->QueryDataTempl<EnhanceGroupTempl>(req.enhance_gid);
    req.enhance_cost = req.cost_type ? gtempl->money : gtempl->score;
    req.enhance_cost *= (1 + 0.2 * req.protect_slot_num);
    if (req.enhance_cost < 0)
    {
        return;
    }
    if (req.enhance_cost > req.player_own)
    {
        int8_t err = req.cost_type == 0 ? G2C::ERR_NO_SCORE : G2C::ERR_NO_MONEY;
        SendMessage(GS_MSG_SERVICE_ERROR, player, err, buf, size);
        return;
    }

    if (count_ > 0)
    {
        --count_;
    }

    std::string out_buf;
    req.count = count_;
    serviceDef::PackSrvParam(req, out_buf);

    SendMessage(GS_MSG_SERVICE_DATA, player, out_buf.c_str(), out_buf.size());
    __PRINTF("EnhanceProvider TryServe Success enhance_gid:%d", req.enhance_gid);
}

void EnhanceProvider::GetContent(const XID& player, int link_id, int client_sid)
{
    UpdateEnhanceInfo();

    G2C::NpcServiceContent::enhance_srv_content content;
    content.count = count_;
    content.reset_time = reset_time_;

    shared::net::ByteBuffer buffer;
    buffer << *const_cast<G2C::NpcServiceContent::enhance_srv_content*>(&content);

	SendServiceContent(player.id, link_id, client_sid, buffer.contents(), buffer.size());
}

void EnhanceProvider::UpdateEnhanceInfo()
{
	int32_t now = g_timer->GetSysTime();
    if (now > reset_time_)
    {
        const EnhanceGroupTempl* gtempl = s_pDataTempl->QueryDataTempl<EnhanceGroupTempl>(enhance_gid_);
        reset_time_ = CalcResetTime(gtempl);
        count_ = gtempl->count;
    }
}


///
/// SimpleServiceProvider
///
bool SimpleServiceProvider::OnInit(const serviceDef::ServiceTempl* param_tmpl)
{
	return true;
}

void SimpleServiceProvider::TryServe(const XID& player, const void* buf, size_t size)
{
}

} // namespace gamed
