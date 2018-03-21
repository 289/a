#ifndef GAMED_GS_TEMPLATE_DATATEMPL_MALL_CLASS_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_MALL_CLASS_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class MallClassTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(MallClassTempl, TEMPL_TYPE_MALL_CLASS);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxVisibleNameLen      = 64;  // 商城显示名称的最大长度
	static const int kMaxClassGoodsCount     = 50;  // 单个商城分类中商品的最大个数
	static const int kMaxIconResourcePathLen = 512; // 商品图标路径的最大长度
	static const int kMaxDescriptionLen      = 256; // 商品描述的最大长度

	/*商品刷新规则*/
	enum REFRESH_RULE
	{
		REFRESH_INVALID,
		REFRESH_EMPTY,    // 不刷新
		REFRESH_PER_DAY,  // 每天凌晨刷新
		REFRESH_MAX,
	};

	/*分类商品信息*/
	struct GoodsEntry
	{
		TemplID item_id;             // 商品对应的物品ID
		int32_t item_count;          // 商品初始个数(-1表示不限售)
		int16_t recommand_level;     // 商品推荐等级(0-5)
		int32_t sell_price;          // 商品出售价格
		TimeSegment sell_time;       // 商品出售时间段
		float   discount;            // 商品折扣
		TimeSegment discount_time;   // 折扣生效时间段
		int16_t refresh_rule;        // 商品刷新规则
		int32_t refresh_interval;    // 商品刷新间隔(单位:min)
		int32_t max_buy_count;       // 玩家最大购买数量(-1表示不限购)
		BoundArray<uint8_t, kMaxDescriptionLen> description;
		BoundArray<uint8_t, kMaxIconResourcePathLen> icon_path;
		NESTED_DEFINE(item_id, item_count, recommand_level, sell_price, sell_time, discount, discount_time, refresh_rule, refresh_interval, max_buy_count, description, icon_path);
	};


public:
// 0
	BoundArray<uint8_t, kMaxVisibleNameLen>   visible_name;
	BoundArray<GoodsEntry, kMaxClassGoodsCount> goods_list;


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, goods_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(visible_name, goods_list);
	}
	
	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < goods_list.size(); ++ i)
		{
			const GoodsEntry& entry = goods_list[i];

			if (entry.item_id <= 0)
				return false;
			if (entry.recommand_level < 0)
				return false;
			if (entry.sell_price <= 0)
				return false;
			if (entry.sell_time.is_valid && !CheckTimeSegment(entry.sell_time))
				return false;
			if (entry.discount < 0.0f || entry.discount > 1.0f)
				return false;
			if (entry.discount_time.is_valid && !CheckTimeSegment(entry.discount_time))
				return false;
			if (entry.max_buy_count < -1)
				return false;
			if (entry.refresh_rule <= REFRESH_INVALID || entry.refresh_rule >= REFRESH_MAX)
				return false;
			if (entry.refresh_rule == REFRESH_PER_DAY)
			{
				if (entry.item_count <= 0 && entry.max_buy_count <= 0)
					return false;
			}
		}

		return true;
	}
};

}; // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_MALL_CLASS_TEMPL_H_
