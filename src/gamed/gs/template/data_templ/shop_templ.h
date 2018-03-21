#ifndef GAMED_GS_TEMPLATE_DATATEMPL_SHOP_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_SHOP_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

class ShopTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(ShopTempl, TEMPL_TYPE_SHOP);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxVisibleNameLen = 64;  // 商店显示名称的最大长度
	static const int kMaxIconPathLen    = 512; // 商店图标路径的最大长度
	static const int kMaxClassNumber    = 10;  // 商店的最大分类数
	static const int kMaxGoodsNumber    = 32;  // 单个分类的最大商品数

	enum MONEY_TYPE
	{
		MT_INVALID,
		MT_MONEY, //游戏币
		MT_CASH,  //人民币
		MT_SCORE, //学分
		MT_MAX,
	};

	enum REFRESH_RULE
	{
		REFRESH_INVALID,
		REFRESH_EMPTY,    // 不刷新
		REFRESH_PER_DAY,  // 每天凌晨刷新
		REFRESH_MAX,
	};

	struct GoodsEntry
	{
		int32_t item_id;          // 商品对应的物品ID
		int32_t init_item_count;  // 初始商品数量(0表示无商品可售;-1表示不限制)
		int32_t sell_price;       // 商品出售价格
		int16_t refresh_rule;     // 商品刷新规则
		int16_t refresh_interval; // 商品刷新间隔
		NESTED_DEFINE(item_id, init_item_count, sell_price, refresh_rule, refresh_interval);
	};

	struct ClassEntry
	{
		BoundArray<uint8_t, kMaxVisibleNameLen> visible_name; //商品分类显示名
		BoundArray<uint8_t, kMaxIconPathLen>    icon_path;    //商品分类图标路径
		BoundArray<GoodsEntry, kMaxGoodsNumber> goods_list;   //商品分类的商品列表
		NESTED_DEFINE(visible_name, icon_path, goods_list);
	};


public:
// 0
	BoundArray<uint8_t, kMaxVisibleNameLen> shop_visible_name;
	BoundArray<uint8_t, kMaxIconPathLen> shop_icon_path;
	uint8_t coin_type;//商店货币类型
	TimeSegment open_time;
	BoundArray<ClassEntry, kMaxClassNumber> class_list; 


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(shop_visible_name, shop_icon_path, coin_type, open_time, class_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(shop_visible_name, shop_icon_path, coin_type, open_time, class_list);
	}
    
	virtual bool OnCheckDataValidity() const
	{
		if (coin_type <= MT_INVALID || coin_type >= MT_MAX)
			return false;

		if (open_time.is_valid && !CheckTimeSegment(open_time))
			return false;

		for (size_t i = 0; i < class_list.size(); ++ i)
		{
			const ClassEntry& cls_entry = class_list[i];

			for (size_t j = 0; j < cls_entry.goods_list.size(); ++ j)
			{
				const GoodsEntry& entry = cls_entry.goods_list[j];

				if (entry.item_id <= 0)
					return false;
				if (entry.sell_price <= 0)
					return false;
				if (entry.refresh_rule < REFRESH_INVALID || entry.refresh_rule >= REFRESH_MAX)
					return false;
				if (entry.refresh_rule == REFRESH_PER_DAY && entry.init_item_count <= 0)
					return false;
			}
		}
		return true;
	}
};

}; // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_SHOP_TEMPL_H_

