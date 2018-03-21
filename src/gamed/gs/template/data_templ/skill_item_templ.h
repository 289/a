#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_SKILL_ITEM_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_SKILL_ITEM_TEMPL_H__

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @class SkillItemTempl
 * @brief 技能物品模板
 */
class SkillItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(SkillItemTempl, TEMPL_TYPE_SKILL_ITEM);
public:
    static const int kMaxBuffCount = 8;

	inline void set_templ_id(TemplID id) { templ_id = id; }
    
    // 0
    int32_t cls_level_limit;      // 使用时对玩家的等级限制
    int32_t money_need_on_use;    // 使用时需要消耗多少游戏币
    int32_t cash_need_on_use;     // 使用时需要消耗多少人民币
	TemplID item_need_on_use;     // 使用时需要消耗的道具ID
    int32_t item_count_on_use;    // 使用时需要消耗多少道具

    // 5
	bool    consume_on_use;       // 使用后是否销毁
	BoundArray<BuffID, kMaxBuffCount> buffs_addon; // 物品BUFF列表
    ItemCoolDownData cooldown_data; // 冷却数据


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(cls_level_limit, money_need_on_use, cash_need_on_use, item_need_on_use, item_count_on_use);
		MARSHAL_TEMPLVALUE(consume_on_use, buffs_addon, cooldown_data);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(cls_level_limit, money_need_on_use, cash_need_on_use, item_need_on_use, item_count_on_use);
		UNMARSHAL_TEMPLVALUE(consume_on_use, buffs_addon, cooldown_data);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (cls_level_limit < 0 ||
            cls_level_limit > 60 ||
            money_need_on_use < 0 ||
            cash_need_on_use < 0 ||
            item_need_on_use < 0 ||
            item_count_on_use < 0)
        {
            return false;
        }

        for (size_t i = 0; i < buffs_addon.size(); ++ i)
        {
            if (buffs_addon[i] <= 0)
                return false;
        }

        if (!cooldown_data.is_valid())
            return false;

		return true;
	}
};

}; // namespace dataTempl

#endif
