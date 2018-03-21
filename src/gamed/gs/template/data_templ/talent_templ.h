#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TALENT_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TALENT_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 天赋模板
 */
class TalentTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(TalentTempl, TEMPL_TYPE_TALENT);
public:
	static const int kMaxVisibleNameLen	= UTF8_LEN(24);  // 最多支持24个汉字
	static const int kMaxDescriptionLen = UTF8_LEN(128); // 最多支持128个汉字
	static const int kMaxIconPathLen    = 512;           // 图标资源路径的最大长度
	static const int kMaxTalentLevel    = 51;            // 最大天赋等级
	static const int kMaxPropCount      = 16;

	//天赋类型
	enum TalentType
	{
		TYPE_INVALID,
		TYPE_REQUIRED,      // 必修天赋
		TYPE_OPTIONAL,      // 选修天赋
		TYPE_MAX,
	};

	//升级模式
	enum LvlUpMode
	{
		LVLUP_MODE_INVALID,
		LVLUP_MODE_MANUAL,  // 手动升级
		LVLUP_MODE_SYSTEM,  // 系统升级
		LVLUP_MODE_MAX,
	};

	//升级配置项
	struct LvlUpEntry
	{
		struct ItemEntry
		{
			TemplID item_id;
			int32_t item_count;
			NESTED_DEFINE(item_id, item_count);
		};

        // 0
		BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;  // 特定等级对应的显示名称
		BoundArray<uint8_t, kMaxDescriptionLen> description;   // 特定等级对应的描述
		SkillID   passive_skill_id;                            // 升级后获得的被动技能ID
		int32_t   ratio;                                       // 升级后对基础提升属性的提升比例(万分数)
		uint8_t   lvlup_mode;                                  // 升级方式：手动触发或系统触发

        // 5
		int32_t   money_need_on_lvlup;                         // 升级需要的游戏币
		int32_t   cash_need_on_lvlup;                          // 升级需要的元宝
		int32_t   score_need_on_lvlup;                         // 升级需要的学分
		BoundArray<ItemEntry, 2> items_need_on_lvlup;          // 升级需要的道具
        int32_t   adjust_scale_pet_skill_cd_time;              // 对宠物技能CD冷却的校正, 校正值为CD时间的万分比。

        // 10
        int32_t   adjust_value_pet_power_gen_speed;            // 对宠物能量恢复值的校正, 校正值为能量的绝对值。
        int32_t   adjust_value_cat_vision_gen_interval;        // 对玩家瞄类视觉恢复周期的校正，校正值为时间的绝对值。
        int32_t   adjust_value_cat_vision_gen_speed;           // 对玩家瞄类视觉恢复速度的校正，校正值为点数的绝对值。
        int32_t   adjust_scale_combat_award_exp;               // 对玩家战斗结束获得经验的校正，校正值为经验的万分比。
        int32_t   adjust_scale_combat_award_money;             // 对玩家战斗结束获得金钱的校正，校正值为金钱的万分比。

        // 15
        int32_t   adjust_value_player_dying_time;              // 对玩家战斗濒死持续时间的校正，校正值为濒死时间的绝对值。
        int32_t   adjust_skill_tree_id;                        // 对哪一个技能树进行校正。
        int32_t   adjust_value_skill_temp_level;               // 对技能树临时等级的校正，校正值为技能临时等级的绝对值。


		void Pack(shared::net::ByteBuffer& buf)
        {
            PACK_NESTED_VALUE(visible_name, description, passive_skill_id, ratio, lvlup_mode);
            PACK_NESTED_VALUE(money_need_on_lvlup, cash_need_on_lvlup, score_need_on_lvlup, items_need_on_lvlup, adjust_scale_pet_skill_cd_time);
            PACK_NESTED_VALUE(adjust_value_pet_power_gen_speed, adjust_value_cat_vision_gen_interval, adjust_value_cat_vision_gen_speed);
            PACK_NESTED_VALUE(adjust_scale_combat_award_exp, adjust_scale_combat_award_money, adjust_value_player_dying_time);
            PACK_NESTED_VALUE(adjust_skill_tree_id, adjust_value_skill_temp_level);
        }

        void UnPack(shared::net::ByteBuffer& buf)
        {
            UNPACK_NESTED_VALUE(visible_name, description, passive_skill_id, ratio, lvlup_mode);
            UNPACK_NESTED_VALUE(money_need_on_lvlup, cash_need_on_lvlup, score_need_on_lvlup, items_need_on_lvlup, adjust_scale_pet_skill_cd_time);
            UNPACK_NESTED_VALUE(adjust_value_pet_power_gen_speed, adjust_value_cat_vision_gen_interval, adjust_value_cat_vision_gen_speed);
            UNPACK_NESTED_VALUE(adjust_scale_combat_award_exp, adjust_scale_combat_award_money, adjust_value_player_dying_time);
            UNPACK_NESTED_VALUE(adjust_skill_tree_id, adjust_value_skill_temp_level);
        }
	};

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
	// 0
    BoundArray<uint8_t, kMaxDescriptionLen> description;    // 天赋描述
	BoundArray<uint8_t, kMaxIconPathLen>    icon_path;     // 图标资源路径
	BoundArray<PropType, kMaxPropCount>     inc_prop_base; // 天赋升级对玩家职业属性提升的基础值
	BoundArray<LvlUpEntry, kMaxTalentLevel> lvlup_table;   // 天赋升级表, 共51项, 天赋的最大等级为50级, 0级为初始等级
	                                                       // lvlup_table[level]表示从level升级到level+1级的条件
														   // lvlup_table[0]项的被动技能ID和属性提升比例无效, 置空.
	uint8_t type;      // 天赋类型
    // 5
    uint8_t init_open; // 初始开启状态


protected:
	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(description, icon_path, lvlup_table, inc_prop_base, type, init_open);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(description, icon_path, lvlup_table, inc_prop_base, type, init_open);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (type <= TYPE_INVALID || type >= TYPE_MAX)
			return false;

		for (size_t i = 0; i < inc_prop_base.size(); ++ i)
			if (inc_prop_base[i] < 0)
				return false;

		for (size_t i = 0; i < lvlup_table.size(); ++ i)
		{
			const LvlUpEntry& entry = lvlup_table[i];

			if (i > 0 && entry.passive_skill_id <= 0 && entry.ratio <= 0 && 
                entry.adjust_scale_pet_skill_cd_time <= 0 && 
                entry.adjust_value_pet_power_gen_speed <= 0 && 
                entry.adjust_value_cat_vision_gen_interval <= 0&&
                entry.adjust_value_cat_vision_gen_speed <= 0 &&
                entry.adjust_scale_combat_award_exp <= 0 &&
                entry.adjust_scale_combat_award_money <= 0 &&
                entry.adjust_value_player_dying_time <= 0 &&
                entry.adjust_skill_tree_id <= 0 &&
                entry.adjust_value_skill_temp_level <= 0)
				return false;

			if (entry.lvlup_mode <= LVLUP_MODE_INVALID ||
				entry.lvlup_mode >= LVLUP_MODE_MAX ||
				entry.money_need_on_lvlup < 0 ||
				entry.cash_need_on_lvlup < 0 ||
				entry.score_need_on_lvlup < 0)
				return false;

			for (size_t j = 0; j < entry.items_need_on_lvlup.size(); ++ j)
				if (entry.items_need_on_lvlup[j].item_id <= 0 ||
					entry.items_need_on_lvlup[j].item_count <= 0)
					return false;
		}

		return true;
	}
};

};

#endif
