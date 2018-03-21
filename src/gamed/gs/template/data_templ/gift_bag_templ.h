#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_GIFT_BAG_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_GIFT_BAG_TEMPL_H__

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @class GiftBagTempl
 * @brief 礼包模板
 */
class GiftBagTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(GiftBagTempl, TEMPL_TYPE_GIFT_BAG);
public:
	static const int kMaxErrorMsgLen = UTF8_LEN(16);

	struct ItemEntry
	{
		int32_t item_id;
		int32_t item_num;
		int32_t probability;
        int32_t valid_time;
		NESTED_DEFINE(item_id, probability, valid_time);
	};

    struct ItemNeededEntry
    {
        int32_t item_id;
        int32_t item_num;
    };

    inline void set_templ_id(TemplID id) { templ_id = id; }

public:
    int32_t probability;
    bool success_disappear;
    bool faliure_disappear;
    std::vector<ItemNeededEntry> item_needed;
    int32_t money_needed;
    std::vector<ItemEntry> item_gift;


protected:
	virtual void OnMarshal()
	{
		//MARSHAL_TEMPLVALUE(consume_on_use, consume_item_id, consume_item_count, task_list, error_msg);
        //MARSHAL_TEMPLVALUE(cooldown_data, forbid_map_type, specify_map);
	}

	virtual void OnUnmarshal()
	{
		//UNMARSHAL_TEMPLVALUE(consume_on_use, consume_item_id, consume_item_count, task_list, error_msg);
        //UNMARSHAL_TEMPLVALUE(cooldown_data, forbid_map_type, specify_map);
	}

	virtual bool OnCheckDataValidity() const
	{
		/*if (consume_item_id < 0 ||
            consume_item_count < 0)
			return false;

		for (size_t i = 0; i < task_list.size(); ++ i)
		{
			const TaskEntry& task = task_list[i];
			if (task.task_id <= 0 ||
				task.probability <= 0 ||
				task.probability > 10000)
				return false;
		}

        if (!cooldown_data.is_valid())
            return false;

        if (forbid_map_type != FMT_NONE && 
            forbid_map_type != FMT_NONNORMAL)
            return false;

        if (specify_map.map_id > 0)
        {
            if (specify_map.min_coord.x == specify_map.max_coord.x &&
                specify_map.min_coord.y == specify_map.max_coord.y)
                return false;

            if (specify_map.min_coord.x > specify_map.max_coord.x ||
                specify_map.min_coord.y > specify_map.max_coord.y)
                return false;
        }*/

		return true;
	}
};

}; // namespace dataTempl

#endif
