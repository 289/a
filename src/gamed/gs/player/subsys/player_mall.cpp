#include "player_mall.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/global/timer.h"
#include "gs/global/game_mall.h"
#include "gs/global/glogger.h"
#include "gs/global/dbgprt.h"
#include "gs/item/item.h"

namespace gamed
{

/*******************************PlayerMall***************************/
/*******************************PlayerMall***************************/
/*******************************PlayerMall***************************/
/*******************************PlayerMall***************************/

PlayerMall::PlayerMall(Player& player):
	PlayerSubSystem(SUB_SYS_TYPE_MALL, player)
{
	mall_cash_ = 0;
	mall_cash_used_ = 0;
	mall_cash_offset_ = 0;
	mall_order_id_ = 0;
	mall_order_id_saved_ = 0;

	SAVE_LOAD_REGISTER(common::PlayerMallData, PlayerMall::SaveToDB, PlayerMall::LoadFromDB);
}

PlayerMall::~PlayerMall()
{
	mall_orders_.clear();
}

bool PlayerMall::SaveToDB(common::PlayerMallData* pData)
{
	pData->mall_cash       = GetCash();
	pData->mall_cash_used  = GetCashUsed();

	pData->order_list.resize(mall_orders_.size());
	for (size_t i = 0; i < mall_orders_.size(); ++ i)
	{
		ASSERT(mall_orders_[i].goods_id > 0);
		common::PlayerMallData::OrderEntry& entry = pData->order_list[i];
		entry.goods_id    = mall_orders_[i].goods_id;
		entry.goods_count = mall_orders_[i].goods_count;
		entry.timestamp   = mall_orders_[i].timestamp;
	}

	return true;
}

bool PlayerMall::LoadFromDB(const common::PlayerMallData& data)
{
	mall_cash_ = data.mall_cash;
	mall_cash_used_ = data.mall_cash_used;

	mall_orders_.resize(data.order_list.size());
	for (size_t i = 0; i < data.order_list.size(); ++ i)
	{
		ASSERT(data.order_list[i].goods_id > 0);
		const common::PlayerMallData::OrderEntry& entry = data.order_list[i];
		mall_orders_[i].goods_id    = entry.goods_id;
		mall_orders_[i].goods_count = entry.goods_count;
		mall_orders_[i].timestamp   = entry.timestamp;
	}

	return true;
}

void PlayerMall::RegisterCmdHandler()
{
	//game-mall
	REGISTER_NORMAL_CMD_HANDLER(C2G::OpenMall, PlayerMall::CMDHandler_OpenMall);
	REGISTER_NORMAL_CMD_HANDLER(C2G::QueryMallGoodsDetail, PlayerMall::CMDHandler_QueryMallGoodsDetail);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MallShopping, PlayerMall::CMDHandler_MallShopping);
}

void PlayerMall::RegisterMsgHandler()
{
}

void PlayerMall::CMDHandler_OpenMall(const C2G::OpenMall& cmd)
{
	//手动刷新商城
	s_pGameMall->OnPlayerOpenMall();

	//每次打开商城检查已存订单是否过期
	DeleteExpiredMallOrder();

	///
	/// 打包商城数据
	///

	G2C::MallData packet;
	packet.cash_amount  = GetCash();
	packet.max_goods_id = s_pGameMall->GetMaxGoodsID();

	//获取销量前五名的商品
	std::vector<int32_t/*goods-id*/> top_sale_list;
	s_pGameMall->QueryTopSaleVolumeGoods(top_sale_list);
	packet.spec_class_goods_list = top_sale_list;

	//获取已购买记录
	for (size_t i = 0; i < mall_orders_.size(); ++ i)
	{
		G2C::MallData::OrderEntry entry;
		entry.goods_id = mall_orders_[i].goods_id;
		entry.goods_limit_buy = s_pGameMall->GetMaxBuyCount(entry.goods_id) - GetBuyCount(entry.goods_id);
		ASSERT(entry.goods_limit_buy >= 0);
		packet.order_list.push_back(entry);
	}

	//获取限售商品信息
	std::vector<playerdef::limit_sale_goods> limit_sale_goods_list;
	s_pGameMall->QueryLimitSaleGoods(limit_sale_goods_list);
	for (size_t i = 0; i < limit_sale_goods_list.size(); ++ i)
	{
		G2C::MallData::LimitSaleGoodsEntry entry;
		entry.goods_id = limit_sale_goods_list[i].goods_id;
		entry.goods_remains = limit_sale_goods_list[i].goods_remains;

		packet.limit_sale_goods_list.push_back(entry);
	}
	player_.sender()->SendCmd(packet);
}

void PlayerMall::CMDHandler_QueryMallGoodsDetail(const C2G::QueryMallGoodsDetail& cmd)
{
	playerdef::goods_detail detail;
	if (s_pGameMall->QueryGoodsDetail(cmd.goods_id, detail))
	{
		G2C::MallGoodsDetail packet;
		packet.goods_id      = detail.goods_id;
		packet.goods_remains = detail.goods_remains;
		if (detail.goods_limit_buy > 0)
		{
			packet.goods_limit_buy = detail.goods_limit_buy - GetBuyCount(cmd.goods_id);
			ASSERT(packet.goods_limit_buy > 0);
		}
		player_.sender()->SendCmd(packet);
	}
}

void PlayerMall::CMDHandler_MallShopping(const C2G::MallShopping& cmd)
{
	if (cmd.goods_id <= 0 || cmd.goods_count <= 0)
		return;

	if (!player_.HasSlot(Item::INVENTORY, 1))
		return; //包裹满

	int32_t goods_id      = cmd.goods_id;
	int32_t goods_count   = cmd.goods_count;
	int32_t has_buy_count = GetBuyCount(goods_id);
	int32_t max_buy_count = s_pGameMall->GetMaxBuyCount(goods_id);
	if (max_buy_count == 0)
		return; //禁止购买
	else if (max_buy_count > 0)
	{
		if ((has_buy_count+goods_count) > max_buy_count)
		{
			G2C::MallShopping_Re packet;
			packet.result = G2C::MallShopping_Re::MSR_EXCEED_MAX_BUY_COUNT;
			packet.goods_id = goods_id;
			packet.goods_count = goods_count;
			player_.sender()->SendCmd(packet);
			return; //超过限购个数
		}
	}

	int32_t cash_used = 0;
	int32_t goods_remains = -1; //初始值必须是-1,表示默认不限售
	int32_t goods_item_id = s_pGameMall->GetGoodsItemID(goods_id);
	int ret = s_pGameMall->PlayerDoShopping(&player_, goods_id, goods_count, goods_remains, cash_used);
	if (ret < 0)
	{
		//非法参数
		return;
	}
	else if (ret == 0)
	{
		//购买成功
		if (max_buy_count > 0)
		{
			//保存购买记录
			//只记录有购买个数限制的商品
			mall_order order;
			order.order_id    = ++ mall_order_id_;
			order.goods_id    = goods_id;
			order.goods_count = goods_count;
			order.timestamp   = g_timer->GetSysTime();
			mall_orders_.push_back(order);
		}

		//物品进包裹
		if (!player_.GainItem(goods_item_id, goods_count))
		{
			LOG_ERROR << "玩家包裹满，购买商城商品失败!!!" << "goods_id:" << goods_id << "item_id:" << goods_item_id << "goods_count:" << goods_count;
		}
		else
		{
			GLog::log("gmall_trade:roleid:%ld, order_id:%d, goods_id:%d, item_id:%d, item_count:%d, cash_used:%d, timestamp:%d",
						player_.role_id(), mall_order_id_, goods_id, goods_item_id, goods_count, cash_used, g_timer->GetSysTime());
		}

		//扣除元宝
		UseCash(cash_used);
	}
	else
	{
		__PRINTF("玩家%ld从商城购物失败, goods_id=%d, goods_count=%d, ret=%d", player_.role_id(), goods_id, goods_count, ret);
	}

	//购买失败
	G2C::MallShopping_Re packet;
	packet.result = ret;
	packet.goods_id = goods_id;
	packet.goods_count = goods_count;
	packet.goods_remains = goods_remains;
	packet.goods_limit_buy = max_buy_count - GetBuyCount(goods_id);
	player_.sender()->SendCmd(packet);
}

int32_t PlayerMall::GetBuyCount(int32_t goods_id) const
{
	int32_t count = 0;
	for (size_t i = 0; i < mall_orders_.size(); ++ i)
	{
		if (mall_orders_[i].goods_id == goods_id)
		{
			count += mall_orders_[i].goods_count;
		}
	}
	return count;
}

void PlayerMall::NotifyCashChange(int32_t old_cash) const
{
    int32_t delta = GetCash() - old_cash;
    if (delta >= 0)
    {
        G2C::GainCash packet;
        packet.amount = delta;
        player_.sender()->SendCmd(packet);
    }
    else
    {
        G2C::SpendCash packet;
        packet.cost = -delta;
	    player_.sender()->SendCmd(packet);
    }
}

void PlayerMall::PlayerGetCash() const
{
    G2C::GetOwnCash packet;
    packet.cur_cash = GetCash();
    player_.sender()->SendCmd(packet);
}

void PlayerMall::DeleteExpiredMallOrder()
{
	if (!mall_orders_.size())
		return;

	std::vector<playerdef::goods_refresh_time> list;
	for (size_t i = 0; i < mall_orders_.size(); ++ i)
	{
		playerdef::goods_refresh_time entry;
		entry.goods_id = mall_orders_[i].goods_id;
		list.push_back(entry);
	}

	s_pGameMall->QueryRefreshTimeStamp(list);
	ASSERT(list.size() == mall_orders_.size());

	MallOrderVec __list;
	__list.swap(mall_orders_);

	for (size_t i = 0; i < __list.size(); ++ i)
	{
		if (__list[i].timestamp >= list[i].timestamp)
		{
			mall_orders_.push_back(__list[i]);
		}
	}
}

/*上线时调用*/
void PlayerMall::SetCashInfo(int32_t cash, int32_t cash_used)
{
    mall_cash_ = cash;
    mall_cash_used_ = cash_used;
    mall_cash_offset_ = 0;
}

/*存盘时调用*/
void PlayerMall::GetCashInfo(int32_t& cash, int32_t& cash_used)
{
    cash = mall_cash_ + mall_cash_offset_;
    cash_used = mall_cash_used_ - mall_cash_offset_;
    mall_cash_offset_ = 0;
}

/*充值即时到账调用*/
void PlayerMall::SetCashTotal(int32_t cash_total)
{
    mall_cash_ = cash_total - mall_cash_used_;
}

/*获取当前可用点数*/
int32_t PlayerMall::GetCash() const
{
    return mall_cash_ + mall_cash_offset_;
}

/*获取已经使用点数*/
int32_t PlayerMall::GetCashUsed() const
{
    return mall_cash_used_ - mall_cash_offset_;
}

bool PlayerMall::NeedSave() const
{
    return mall_order_id_ != mall_order_id_saved_;
}

bool PlayerMall::CheckCash(int32_t cash) const
{
    return GetCash() >= cash;
}

void PlayerMall::UseCash(int32_t offset)
{
    // record old
    int32_t old_cash   = GetCash();
    // modify used
    mall_cash_offset_ -= offset;
    // to client
    NotifyCashChange(old_cash);
}

void PlayerMall::AddCash(int32_t cash_add)
{
    // record old
    int32_t old_cash   = GetCash();
    // modify total
    int32_t cash_total = mall_cash_ + mall_cash_used_ + cash_add;
    SetCashTotal(cash_total);
    // to client
    NotifyCashChange(old_cash);
}

PlayerMall::mall_order* PlayerMall::GetMallOrder(int32_t goods_id)
{
	for (size_t i = 0; i < mall_orders_.size(); ++ i)
    {
		if (mall_orders_[i].goods_id == goods_id) {
			return &(mall_orders_[i]);
        }
    }
	return NULL;
}

const PlayerMall::mall_order* PlayerMall::GetMallOrder(int32_t goods_id) const
{
	return GetMallOrder(goods_id);
}

} // namespace gamed
