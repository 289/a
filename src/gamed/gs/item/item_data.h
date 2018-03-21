#ifndef __GAMED_GS_ITEM_ITEM_DATA_H__
#define __GAMED_GS_ITEM_ITEM_DATA_H__

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace gamed {

struct itemdata 
{
	int32_t id;				//模板ID
	int16_t index;          //物品栏位
	int32_t count;			//物品数量
	int32_t pile_limit;		//堆叠上限
	int32_t proc_type;		//处理方式
	int32_t recycle_price;	//卖店价(回收价格)
	int32_t expire_date;	//过期日期，<=0表示永不失效
	int16_t item_cls;       //物品分类
	std::string content;    //物品动态数据

	itemdata():
		id(0),
		index(-1),
		count(0),
		pile_limit(0),
		proc_type(0),
		recycle_price(0),
		expire_date(0),
		item_cls(0)
	{
	}

	void Clone(itemdata& data) const
	{
		data = *this;
	}
};

} // namespace gamed

#endif // __GAMED_GS_ITEM_ITEM_DATA_H__
