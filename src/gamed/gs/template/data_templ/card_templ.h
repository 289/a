#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_CARD_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_CARD_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief: 卡牌模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class CardTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(CardTempl, TEMPL_TYPE_CARD);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

    struct skill_entry
    {
        int32_t skill_group_id;     // 提升的技能组
        int32_t tmp_skill_level;    // 提升的临时等级
        NESTED_DEFINE(skill_group_id, tmp_skill_level);
    };

	// 0
	int16_t level;                                  // 卡牌等级,默认为1
	int16_t card_group_id;                          // 本卡牌所属的卡牌组ID
    int32_t money_need_on_consume;                  // 被消耗时需要的金币
	int32_t card_gain_on_lvlup;                     // 升级后获得卡牌的ID

	// 5
    int32_t exp_need_on_lvlup;                      // 升级需要的经验值
    int32_t exp_offer_on_consume;                   // 被消耗时提供的经验值
	SkillID addon_skill_id;                         // 附加技能ID
	BoundArray<uint8_t, 256>  addon_skill_desp;     // 附加技能描述
	BoundArray<PropType, MAX_PROP_NUM> addon_props; // 附加属性

    // 10
    std::vector<skill_entry> lvlup_skills;     // 提升的技能临时等级

protected:
	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(level, card_group_id, money_need_on_consume, card_gain_on_lvlup);
		MARSHAL_TEMPLVALUE(exp_need_on_lvlup, exp_offer_on_consume, addon_skill_id, addon_skill_desp, addon_props, lvlup_skills);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(level, card_group_id, money_need_on_consume, card_gain_on_lvlup);
		UNMARSHAL_TEMPLVALUE(exp_need_on_lvlup, exp_offer_on_consume, addon_skill_id, addon_skill_desp, addon_props, lvlup_skills);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (level <= 0 ||
			card_group_id < 0 ||
            money_need_on_consume < 0 ||
			card_gain_on_lvlup < 0 ||
            exp_need_on_lvlup < 0 ||
            exp_offer_on_consume < 0 ||
			addon_skill_id < 0 ||
            lvlup_skills.size() > 20)
			return false;

        for (size_t i = 0; i < lvlup_skills.size(); ++i)
        {
            const skill_entry& entry = lvlup_skills[i];
            if (entry.skill_group_id < 0 || entry.tmp_skill_level < 0)
            {
                return false;
            }
        }

		return true;
	}
};

}; // namespace dataTempl

#endif //  _GAMED_GS_TEMPLATE_DATA_TEMPL_CARD_TEMPL_H_
