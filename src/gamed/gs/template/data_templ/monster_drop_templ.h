#ifndef GAMED_GS_TEMPLATE_DATATEMPL_DROP_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_DROP_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class MonsterDropTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(MonsterDropTempl, TEMPL_TYPE_MONSTER_DROP_TEMPL);
public:
	struct drop_entry
	{
		int32_t item_id;    //掉落物品ID
		int32_t item_count; //掉落物品个数
		int32_t drop_prob;  //掉落概率(万分数)
		bool unique_drop;   //唯一掉落
		NESTED_DEFINE(item_id, item_count, drop_prob, unique_drop);
	};

	inline void set_templ_id(TemplID id) { templ_id = id; }

public:
	int32_t drop_times;
	BoundArray<drop_entry, 64> drop_table;

protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(drop_times, drop_table);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(drop_times, drop_table);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (drop_times <= 0) return false;

		int unique_drop_item_count = 0;
		int drop_prob_sum = 0;
		for (size_t i = 0; i < drop_table.size(); ++ i)
		{
			const drop_entry& entry = drop_table[i];
			if (entry.item_id <= 0 ||
				entry.item_count <= 0 ||
				entry.drop_prob <= 0 ||
				entry.drop_prob > 10000)
			{
				return false;
			}
			if (entry.unique_drop)
			{
				++ unique_drop_item_count;
			}
			drop_prob_sum += entry.drop_prob;
		}

		// 全部为唯一性掉落时的检查
		int __count = unique_drop_item_count;
		if (__count == (int)drop_table.size() && __count < drop_times)
			return false;

		// 归一化检查
		if (drop_table.size() && drop_prob_sum != 10000)
			return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_DROP_TEMPL_H_
