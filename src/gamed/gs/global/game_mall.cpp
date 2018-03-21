#include "game_mall.h"
#include "timer.h"
#include "gmatrix.h"
#include "msg_obj.h"
#include "game_util.h"
#include "dbgprt.h"
#include "gs/player/player.h"
#include "gs/item/item_manager.h"
#include "gs/template/data_templ/mall_templ.h"
#include "gs/template/data_templ/mall_class_templ.h"
#include "gs/template/data_templ/templ_manager.h"

namespace gamed
{

/*******************************GMall*********************************/
/*******************************GMall*********************************/
/*******************************GMall*********************************/
/*******************************GMall*********************************/
GMall::GMall():
	goods_id_(0),
	hb_ticker_(0),
	init_done_(false)
{
}

GMall::~GMall()
{
}

void GMall::Initialize()
{
	std::vector<const dataTempl::MallTempl*> mall;
	s_pDataTempl->QueryDataTemplByType(mall);
	ASSERT(mall.size() == 1);

	const dataTempl::MallTempl* mall_tpl = mall[0];
	ASSERT(mall_tpl != NULL);

	for (size_t i = 0; i < mall_tpl->mall_class_list.size(); ++ i)
	{
		int32_t mall_class_templ_id = mall_tpl->mall_class_list[i];
		const dataTempl::MallClassTempl* __tpl = s_pDataTempl->QueryDataTempl<dataTempl::MallClassTempl>(mall_class_templ_id);
		ASSERT(__tpl != NULL);

		for (size_t cls = 0; cls < __tpl->goods_list.size(); ++ cls)
		{
			const dataTempl::MallClassTempl::GoodsEntry& entry = __tpl->goods_list[cls];

			MallGoods* goods = new MallGoods();
			int32_t id = ++ goods_id_;

			//设置基本信息
			goods->goods_id          = id;
			goods->item_id           = entry.item_id;
			goods->item_count        = entry.item_count;
			goods->item_remains      = entry.item_count;
			goods->pile_limit        = s_pItemMan->GetItemPileLimit(entry.item_id);
			goods->sale_price        = entry.sell_price;
			goods->recommand_level   = entry.recommand_level;
			goods->max_buy_count     = entry.max_buy_count;
			goods->discount_rate     = entry.discount;
			goods->refresh_rule      = entry.refresh_rule;
			goods->refresh_interval  = entry.refresh_interval;

			BuildTimeSlot(goods->sell_time, entry.sell_time);
			BuildTimeSlot(goods->discount_time, entry.discount_time);

			goods_map_[id] = goods;
		}
	}

	BuildReserveVolumeMap();
	BuildRefreshGoodsMap();
	init_done_ = true;
}

void GMall::Release()
{
	MallGoodsMap::iterator it = goods_map_.begin();
	for (; it != goods_map_.end(); ++ it)
	{
		MallGoods* obj = it->second;
		delete obj;
	}

	goods_map_.clear();
	sale_volume_map_.clear();
	reserve_volume_map_.clear();
	refresh_time_map_.clear();
}

void GMall::HeartbeatTick()
{
	if (++ hb_ticker_ >= TICK_PER_SEC * 60)
	{
		//为了节省资源，商品刷新的调整为每30秒一次
		//为了弥补刷新间隔大的问题，玩家打开商城时可能会手动刷新一次
		RefreshGoods();
		hb_ticker_ = 0;
	}
}

void GMall::OnPlayerOpenMall()
{
	if (hb_ticker_ >= 200)
	{
		//超过10秒未刷新则手动刷新商品
		RefreshGoods();
		hb_ticker_ = 0;
	}
}

int GMall::PlayerDoShopping(const Player* player, int32_t goods_id, int32_t goods_count, int32_t& goods_remains, int32_t& cash_cost)
{
	if (!init_done_)
		return -1;
	if (goods_id <= 0 || goods_count <= 0)
		return -1;

	MallGoods* goods = QueryGoods(goods_id);
	if (!goods) return -1;

	time_t now = g_timer->GetSysTime();

	//购买前检查
	if (!goods->sell_time.IsMatch(now))
		return ERR_NOT_IN_SELL_TIME;
	if (goods->pile_limit < goods_count)
		return ERR_EXCEED_ITEM_PILE_LIMIT;
	if (goods->max_buy_count > 0 && goods->max_buy_count < goods_count)
		return ERR_EXCEED_MAX_BUY_COUNT;

	//计算购买花费
	float discount = 1.0f;
	if (goods->discount_time.IsMatch(now))
		discount = goods->discount_rate;
	int cash_used = goods->sale_price * goods_count * discount;
	if (cash_used > player->GetCash())
		return ERR_NO_ENOUGH_CASH;

	GoodsReserveVolumeMap::iterator it = reserve_volume_map_.find(goods_id);
	if (it != reserve_volume_map_.end())
	{
		shared::RWLockWriteGuard keeper(reserve_volume_lock_);

		//检查商品个数是否够
		if (goods_count <= it->second)
		{
			it->second -= goods_count;
			ASSERT(it->second >= 0);
			goods_remains = it->second;
		}
		else
		{
			return ERR_NO_ENOUGH_GOODS;
		}
	}

	//更新销量排行榜
	UpdateSaleVolumeMap(goods_id, goods_count);

	cash_cost = cash_used;

	return ERR_SUCCESS;
}

bool GMall::QueryGoodsDetail(int32_t goods_id, playerdef::goods_detail& detail) const
{
	if (!init_done_) return false;

	memset(&detail, 0, sizeof(detail));
	const MallGoods* goods = QueryGoods(goods_id);
	if (!goods) return false;

	GoodsReserveVolumeMap::const_iterator it = reserve_volume_map_.find(goods_id);
	if (it == reserve_volume_map_.end())
	{
		__PRINTF("GMall: 请求非限售商品信息, goods_id=%d", goods_id);
		return false;
	}

	detail.goods_id = goods->goods_id;
	detail.goods_limit_buy = goods->max_buy_count;

	shared::RWLockReadGuard keeper(reserve_volume_lock_);
	detail.goods_remains = it->second;

	return true;
}

void GMall::QueryTopSaleVolumeGoods(std::vector<int32_t>& list) const
{
	if (!init_done_) return;

	shared::RWLockReadGuard keeper(sale_volume_lock_);

	GoodsSaleVolumeMap::const_iterator it = sale_volume_map_.begin();
	for (size_t i = 0; i < 5 && it != sale_volume_map_.end(); ++ it, ++ i)
	{
		list.push_back(it->second);
	}
}

void GMall::QueryLimitSaleGoods(std::vector<playerdef::limit_sale_goods>& list) const
{
	if (!init_done_) return;

	shared::RWLockReadGuard keeper(reserve_volume_lock_);

	list.resize(reserve_volume_map_.size());
	GoodsReserveVolumeMap::const_iterator it = reserve_volume_map_.begin();
	for (size_t i = 0; it != reserve_volume_map_.end(); ++ it, ++ i)
	{
		list[i].goods_id = it->first;
		list[i].goods_remains = it->second;
	}
}

void GMall::QueryRefreshTimeStamp(std::vector<playerdef::goods_refresh_time>& list) const
{
	if (!init_done_) return;

	shared::RWLockReadGuard refresh_time_keeper(refresh_time_lock_);

	for (size_t i = 0; i < list.size(); ++ i)
	{
		int32_t goods_id = list[i].goods_id;
		GoodsRefreshTimeMap::const_iterator it_time_map = refresh_time_map_.find(goods_id);
		ASSERT(it_time_map != refresh_time_map_.end());
		list[i].timestamp = it_time_map->second;
	}
}

void GMall::BuildReserveVolumeMap()
{
	shared::RWLockWriteGuard keeper(reserve_volume_lock_);

	MallGoodsMap::iterator it = goods_map_.begin();
	for (; it != goods_map_.end(); ++ it)
	{
		MallGoods* obj = it->second;
		if (obj->item_count < 0)
			continue;

		reserve_volume_map_[obj->goods_id] = obj->item_count;
	}
}

void GMall::BuildRefreshGoodsMap()
{
	shared::RWLockWriteGuard refresh_time_keeper(refresh_time_lock_);

	MallGoodsMap::iterator it = goods_map_.begin();
	for (; it != goods_map_.end(); ++ it)
	{
		MallGoods* obj = it->second;

		//判断是否需要刷新
		if (obj->refresh_rule == REFRESH_PER_DAY || obj->max_buy_count > 0)
		{
			refresh_time_map_[obj->goods_id] = g_timer->GetSysTime();
		}
	}
}

GMall::MallGoods* GMall::QueryGoods(int32_t goods_id)
{
	MallGoodsMap::iterator it = goods_map_.find(goods_id);
	if (it != goods_map_.end())
		return it->second;
	return NULL;
}

const GMall::MallGoods* GMall::QueryGoods(int32_t goods_id) const
{
	MallGoodsMap::const_iterator it = goods_map_.find(goods_id);
	if (it != goods_map_.end())
		return it->second;
	return NULL;
}

void GMall::UpdateSaleVolumeMap(int32_t goods_id, int32_t goods_count)
{
	shared::RWLockWriteGuard keeper(sale_volume_lock_);

	GoodsSaleVolumeMap::iterator it_map = sale_volume_map_.begin();
	for (; it_map != sale_volume_map_.end(); ++ it_map)
	{
		if (it_map->second == goods_id)
		{
			//已经有人购买过
			int32_t count = it_map->first + goods_count;
			sale_volume_map_.erase(it_map);
			sale_volume_map_.insert(std::pair<int32_t, int32_t>(count, goods_id));
			break;
		}
	}

	if (it_map == sale_volume_map_.end())
	{
		//第一次有人该商品
		sale_volume_map_.insert(std::pair<int32_t, int32_t>(goods_count, goods_id));
	}
}

void GMall::RefreshGoods()
{
	time_t now = g_timer->GetSysTime();
	std::vector<int32_t/*goods-id*/> goods_list;

	//查询哪些商品需要刷新
	{
		shared::RWLockWriteGuard refresh_time_keeper(refresh_time_lock_);

		GoodsRefreshTimeMap::iterator it = refresh_time_map_.begin();
		for (; it != refresh_time_map_.end(); ++ it)
		{
			int32_t goods_id = it->first;
			MallGoods* goods = QueryGoods(goods_id);
			if (goods->refresh_rule == REFRESH_PER_DAY)
			{
				time_t last_refresh_time = it->second;
				if (time_is_overday(last_refresh_time, now))
				{
					it->second = now;
					goods_list.push_back(it->first);
				}
			}
		};
	}

	//刷新商品的存量信息
	{
		shared::RWLockWriteGuard reserve_volume_keeper(reserve_volume_lock_);

		for (size_t i = 0; i < goods_list.size(); ++ i)
		{
			int32_t goods_id = goods_list[i];
			MallGoodsMap::const_iterator it_query_map = goods_map_.find(goods_id);
			ASSERT(it_query_map != goods_map_.end());

			//有刷新时间的商品或者出售数量有限制或者玩家购买个数有限制

			GoodsReserveVolumeMap::iterator it_reserve_volume = reserve_volume_map_.find(goods_id);
			if (it_reserve_volume != reserve_volume_map_.end())
			{
				ASSERT(it_query_map->second->item_count > 0);
				it_reserve_volume->second = it_query_map->second->item_count;
			}
			else
			{
				ASSERT(it_query_map->second->max_buy_count > 0);
			}
		}
	}
}

};
