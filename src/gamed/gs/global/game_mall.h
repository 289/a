#ifndef GAMED_GS_GLOBAL_GAME_MALL_H_
#define GAMED_GS_GLOBAL_GAME_MALL_H_

#include <set>
#include <map>
#include <vector>
#include "time_slot.h"
#include "gs/player/player_def.h"

#include "shared/base/mutex.h"
#include "shared/base/rwlock.h"
#include "shared/base/singleton.h"

namespace gamed
{

class Player;

/**
 * @class GMall
 * @brief 游戏商城
 */
class GMall : public shared::Singleton<GMall>
{
	friend class shared::Singleton<GMall>;
public:
	/*商品信息*/
	struct MallGoods
	{
		int32_t goods_id;           //商品ID(全局唯一)
		int32_t item_id;            //物品ID
		int32_t item_count;         //物品总个数
		int32_t item_remains;       //剩余数量
		int32_t pile_limit;         //堆叠上限
		int32_t sale_price;         //出售价格
		int32_t max_buy_count;      //玩家购买数量限制(>0:限购;-1:无限制)
		int16_t recommand_level;    //推荐等级
		int16_t refresh_rule;       //刷新规则
		int32_t refresh_interval;   //刷新时间间隔(单位:min)
		TimeSlot sell_time;         //出售生效时间
		TimeSlot discount_time;     //折扣生效时间
		float discount_rate;        //折扣(0-1.0f)
	};

private:
	/**
	 * 错误码顺序与G2C::MallShopping_Re里面的错误码顺序保持一致
	 */
	enum ERR_MALL
	{
		ERR_SUCCESS,                // 购买成功
		ERR_NO_ENOUGH_CASH,         // 玩家身上的元宝不足
		ERR_NO_ENOUGH_GOODS,        // 商城的商品数量不足
		ERR_NOT_IN_SELL_TIME,       // 商品出售时间未到
		ERR_EXCEED_ITEM_PILE_LIMIT, // 购买数量超过物品堆叠上限
		ERR_EXCEED_MAX_BUY_COUNT,   // 购买数量超过商品限购上限
	};

	/*商品刷新规则*/
	enum REFRESH_RULE
	{
		REFRESH_INVALID,
		REFRESH_NOT_TAKE_EFFECT, // 不刷新
		REFRESH_PER_DAY,         // 每天凌晨刷新
		REFRESH_PER_INTERVAL,    // 特定时间间隔刷新
		REFRESH_MAX,
	};

	struct goods_finder
	{
		int32_t goods_id;
		goods_finder(int32_t id): goods_id(id) {}
		bool operator() (const MallGoods* goods) const
		{
			return goods->goods_id == goods_id;
		}
	};

	struct greater_comp
	{
		bool operator() (int32_t lhs, int32_t rhs) const
		{
			return lhs >= rhs;
		}
	};

private:
	typedef std::map<int32_t/*goods-id*/, MallGoods*> MallGoodsMap;
	MallGoodsMap goods_map_; //商品查询表

	typedef std::map<int32_t/*goods-id*/, int32_t/*reserve*/> GoodsReserveVolumeMap;
	GoodsReserveVolumeMap reserve_volume_map_; //限售商品存量表
	mutable shared::RWLock reserve_volume_lock_;

	typedef std::multimap<int32_t/*goods-count*/, int32_t/*goods-id*/, greater_comp> GoodsSaleVolumeMap;
	GoodsSaleVolumeMap sale_volume_map_; //销售排行榜
	mutable shared::RWLock sale_volume_lock_;

	typedef std::map<int32_t/*goods-id*/, time_t/*last-refresh-time*/> GoodsRefreshTimeMap;
	GoodsRefreshTimeMap refresh_time_map_; //需要刷新的商品列表
	mutable shared::RWLock refresh_time_lock_;

	int32_t goods_id_;    // 商品ID分配器
	int32_t hb_ticker_;   // 心跳计数
	bool    init_done_;   // 商城初始化完成

public:
    static inline GMall* GetInstance() {
		return &(get_mutable_instance());
	}
	
	void Initialize();
	void Release();
	void HeartbeatTick();
	void OnPlayerOpenMall();
	int  PlayerDoShopping(const Player* player, int32_t goods_id, int32_t goods_count, int32_t& goods_remains, int32_t& cash_cost);

	int32_t GetMaxGoodsID() const;
	int32_t GetGoodsItemID(int32_t goods_id) const;
	int32_t GetMaxBuyCount(int32_t goods_id) const;

	bool QueryGoodsDetail(int32_t goods_id, playerdef::goods_detail& detail) const;
	void QueryTopSaleVolumeGoods(std::vector<int32_t/*goods-id*/>& list) const;
	void QueryLimitSaleGoods(std::vector<playerdef::limit_sale_goods>& list) const;
	void QueryRefreshTimeStamp(std::vector<playerdef::goods_refresh_time>& list) const;

protected:
    GMall();
	~GMall();

private:
	void BuildReserveVolumeMap();
	void BuildRefreshGoodsMap();
	void UpdateSaleVolumeMap(int32_t goods_id, int32_t goods_count);
	void RefreshGoods();
	MallGoods* QueryGoods(int32_t goods_id);
	const MallGoods* QueryGoods(int32_t goods_id) const;
};

#define s_pGameMall GMall::GetInstance()

///
/// inline func
///
inline int32_t GMall::GetMaxGoodsID() const
{
	return goods_id_;
}

inline int32_t GMall::GetGoodsItemID(int32_t goods_id) const
{
	const MallGoods* obj = QueryGoods(goods_id);
	if (!obj) return 0;
	return obj->item_id;
}

inline int32_t GMall::GetMaxBuyCount(int32_t goods_id) const
{
	const MallGoods* obj = QueryGoods(goods_id);
	if (!obj) return 0;
	return obj->max_buy_count;
}

}; // namespace gamed

#endif // GAMED_GS_GLOBAL_GAME_MALL_H_
