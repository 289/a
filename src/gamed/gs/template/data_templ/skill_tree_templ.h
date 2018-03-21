#ifndef GAMED_GS_TEMPLATE_DATATEMPL_SKILL_TREE_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_SKILL_TREE_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class SkillTreeTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(SkillTreeTempl, TEMPL_TYPE_SKILL_TREE);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxVisibleNameLen     = UTF8_LEN(12); // 最多支持12个汉字
	static const int kMaxIconPathLen        = 512;          // 最大图标资源路径长度
	static const int kMaxBriefDataLen       = UTF8_LEN(64); // 简要说明文字的最大长度
	static const int kMaxDetailDataLen      = UTF8_LEN(256);// 详细说明文字的最大长度
	static const int kMaxSkillTreeLevel     = 50;           // 技能树等级上限

	/*等级信息*/
	struct BaseInfo
	{
		BoundArray<uint8_t, kMaxVisibleNameLen>  visible_name;
		BoundArray<uint8_t, kMaxIconPathLen>     icon_file_path;
		BoundArray<uint8_t, kMaxBriefDataLen>    brief_description;
		BoundArray<uint8_t, kMaxDetailDataLen>   detail_description;
		NESTED_DEFINE(visible_name, icon_file_path, brief_description, detail_description);
	};

	struct LvlUpEntry
	{
		/*升级模式*/
		enum LvlUpMode
		{
			LVLUP_MODE_INVALID,
			LVLUP_MODE_AUTO,    //自动升级
			LVLUP_MODE_MANUAL,  //手动升级
			LVLUP_MODE_TRIGGER, //外界触发
			LVLUP_MODE_MAX,
		};

		/*升级条件*/
		struct LvlUpCond
		{
			uint8_t lvlup_mode;    //升级模式
			int16_t cls_lvl_limit; //职业等级限制,0表示无限制
			int32_t money_need;    //升级消耗金钱数
			int32_t score_need;    //升级消耗学分数
			int32_t cash_need;     //升级消耗元宝数
			int32_t item1_need;    //生效消耗道具1
			int32_t item1_count;   //升级消耗道具1个数
			int32_t item2_need;    //升级消耗道具2
			int32_t item2_count;   //升级消耗道具2个数
			NESTED_DEFINE(lvlup_mode, cls_lvl_limit, money_need, score_need, cash_need, item1_need, item1_count, item2_need, item2_count);
		};

		BaseInfo  info;            //当前等级的信息
		SkillID   skill_id;        //当前等级调用的技能
		LvlUpCond lvlup_cond;      //当前等级的升级条件
		NESTED_DEFINE(info, skill_id, lvlup_cond);
	};

	enum SKILL_TYPE
	{
		ST_INVALID,
		ST_ACTIVE,  //主动技能
		ST_PASSIVE, //被动技能
		ST_MAX,
	};

    enum SHOW_TYPE
    {
        SHOW_NONE,      // 无
        SHOW_SINGLE,    // 单体技能
        SHOW_MULTI,     // 群体技能
        SHOW_SUPPORT,   // 辅助技能
        SHOW_HEAL,      // 治疗技能
    };

	int8_t skill_type;
	int8_t show_info;
	int32_t demon_skill_id;
	BoundArray<LvlUpEntry, kMaxSkillTreeLevel+1> lvlup_table; //技能升级表, 共51项, 技能树最大等级为50级, 
	                                                          //如果处于level级, 则升级到下一级的条件为lvlup_table[level].lvlup_cond, 当前技能未lvlup_table[level].skill_id;
															  //第0项表示从0到1级的条件，第49项表示从40到50级的条件，第50项只有50级时的调用技能.

	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(skill_type, show_info, demon_skill_id, lvlup_table);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(skill_type, show_info, demon_skill_id, lvlup_table);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (skill_type <= ST_INVALID || skill_type >= ST_MAX)
			return false;

		if (show_info < SHOW_NONE || show_info > SHOW_HEAL)
			return false;

		if (demon_skill_id < 0)
			return false;

		if (lvlup_table.empty())
			return false;

		for (int i = 0; i < (int)lvlup_table.size(); ++ i)
		{
			const LvlUpEntry& entry = lvlup_table[i];

			if (i == 0)
			{
				if (entry.skill_id != 0)
					return false;
				if (!CheckLevelUpEntry(entry))
					return false;
			}
			else if (i > 0 && i < kMaxSkillTreeLevel)
			{
				if (entry.skill_id <= 0)
					return false;
				if (!CheckLevelUpEntry(entry))
					return false;
			}
			else if (i == kMaxSkillTreeLevel)
			{
				if (entry.skill_id <= 0)
					return false;
				if (entry.lvlup_cond.lvlup_mode != 0 ||
					entry.lvlup_cond.cls_lvl_limit != 0 ||
					entry.lvlup_cond.money_need != 0 ||
					entry.lvlup_cond.score_need != 0 ||
					entry.lvlup_cond.cash_need != 0 ||
					entry.lvlup_cond.item1_need != 0 ||
					entry.lvlup_cond.item1_count != 0 ||
					entry.lvlup_cond.item2_need != 0 ||
					entry.lvlup_cond.item2_count != 0)
					return false;
			}
		}

		return true;
	}

	bool CheckLevelUpEntry(const LvlUpEntry& entry) const
	{
		if (entry.lvlup_cond.lvlup_mode <= 0 ||
			entry.lvlup_cond.cls_lvl_limit < 0 ||
			entry.lvlup_cond.money_need < 0 ||
			entry.lvlup_cond.score_need < 0 ||
			entry.lvlup_cond.cash_need < 0 ||
			entry.lvlup_cond.item1_need < 0 ||
			entry.lvlup_cond.item1_count < 0 ||
			entry.lvlup_cond.item2_need < 0 ||
			entry.lvlup_cond.item2_count < 0)
		{
			return false;
		}
		return true;
	}
};

};

#endif
