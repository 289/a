#ifndef GAMED_GS_TEMPLATE_DATATEMPL_MONSTER_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_MONSTER_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class MonsterTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(MonsterTempl, TEMPL_TYPE_MONSTER_TEMPL);
public:
	static const int kMaxVisibleNameLen     = UTF8_LEN(16); // 最多支持16个中文字符
	static const int kMaxTitleLen           = UTF8_LEN(16);
	static const int kMaxFullPortrait       = 512;
	static const int kMaxModelSrcLen        = 512;
	static const int kMaxIdentifyPicSrcLen  = 512;
	static const int kSkillGroupCount       = 3;
	static const int kMaxSkillDeclare       = 16;

	struct SkillPair
	{
		int32_t    skill_id;
		int32_t    probability; // 万分数
		NESTED_DEFINE(skill_id, probability);
	};

    struct CounterInfo
    {
        enum OpType
        {
            OT_NONE = 0, // 无操作
            OT_INC,      // 增加
            OT_DEC,      // 减少
            OT_ASS,      // assignment赋值
        };

        CounterInfo() : optype(OT_NONE), tid(0), value(0) { }

        int8_t  optype; // 操作类型，对应上面的OpType枚举
        int32_t tid;    // 计数器的模板id
        int32_t value;  // 对该计数器的修改值，必须是0或正数
        NESTED_DEFINE(optype, tid, value);
    };

	enum VisibleType
	{
		VT_COMPLEX = 1, // 综合
		VT_WARRIOR,		// 战士
		VT_MAGE,        // 法师
		VT_ARCHER,      // 射手
		VT_BOSS,        // boss
	};

    enum LevelDisplayMode
    {
        LDM_NORMAL = 0,    // 正常显示等级 
        LDM_NOT_SHOW,      // 不显示等级
        LDM_QUESTION_MARK, // 显示成问号
    };

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
// 0 ---- 0~3 only used in client
	BoundArray<uint8_t, kMaxVisibleNameLen>    visible_name;       // 游戏里怪物的显示名
	BoundArray<uint8_t, kMaxTitleLen>          title;              // 头衔，名字上方的显示
	BoundArray<uint8_t, kMaxModelSrcLen>       title_pic;          // 头衔图片，名字上方的显示
	BoundArray<uint8_t, kMaxModelSrcLen>       model_src;          // 模型资源
	BoundArray<uint8_t, kMaxIdentifyPicSrcLen> identify_pic_src;   // 图鉴资源，如头像

// 5 ---- used in both client and server
	LevelType      level;                // 等级，默认值是1
	uint8_t        faction;              // 阵营
	BoundArray<int32_t, MAX_PROP_NUM> properties;
	SkillID        normal_attack_id;     // 普攻id
	BoundArray<SkillPair, kSkillGroupCount> skill_group; // 没有归一化，小于10000不归一化

// 10
	AIEventID      ai_strategy_id;       // 策略id(用于找到对应的lua脚本)，默认值0表示没有ai策略，有值则必须与templ_id一致
	int32_t        killed_exp;           // 被击杀后，击杀者获得的经验
	int32_t        killed_money;         // 被击杀后，击杀者获得的金钱
	int32_t        normal_drop_prob;     // 普通掉落概率(万分数)
	TemplID        normal_droptable_id;  // 普通掉落包ID

// 15
	TemplID        special_droptable_id; // 特殊掉落包ID
	int32_t        visible_type;         // 怪物的显示类别, 对应枚举VisibleType
	int8_t         info_display_pos;     // 头顶信息显示位置，默认为0         
	int8_t         combat_body_size;     // 战斗体型，默认值0，客户端根据数值到脚本里读取对应的x、y
	int8_t         gender;               // 性别，默认值0表示男，1表示女，对应basetempl里的GENDER_TYPE枚举

// 20
	BoundArray<int32_t, kMaxSkillDeclare> skill_declare;    // 声明战斗中使用到的技能（多为boss怪），客户端加载资源时使用
    int8_t         level_display_mode;   // 怪物等级的显示方式，对应枚举LevelDisplayMode
    BoundArray<uint8_t, kMaxModelSrcLen>  underfoot_effect; // 怪物脚下光效，默认为空
    int8_t         cat_vision;           /* 喵类视觉可见等级，只有玩家的喵类视觉等级大于等于该值，
	                                      * 才能看见该monster，默认值0，最大值32
								          * */
    int8_t         show_boss_alert;      // 是否显示boss警告提示，默认值0，表示不显示，非零表示显示

// 25
	BoundArray<uint8_t, kMaxFullPortrait> full_portrait;    // 全身像，立绘
    int32_t        world_boss_award_tid; // 世界BOSS奖励的模板id，默认值0，表示没有boss奖励
    int8_t         cat_vision_hint;      // 是否提示玩家使用喵类视觉功能
    int32_t        cat_vision_exp;       // 喵类视觉的经验，默认值0，表示没有经验
    CounterInfo    global_counter;       // 对全局计数器的修改

// 30
    CounterInfo    player_counter;       // 对玩家计数器的修改


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, title, title_pic, model_src, identify_pic_src);
		MARSHAL_TEMPLVALUE(level, faction, properties, normal_attack_id, skill_group);
		MARSHAL_TEMPLVALUE(ai_strategy_id, killed_exp, killed_money, normal_drop_prob, normal_droptable_id);
		MARSHAL_TEMPLVALUE(special_droptable_id, visible_type, info_display_pos, combat_body_size, gender);
        MARSHAL_TEMPLVALUE(skill_declare, level_display_mode, underfoot_effect, cat_vision, show_boss_alert);
        MARSHAL_TEMPLVALUE(full_portrait, world_boss_award_tid, cat_vision_hint, cat_vision_exp, global_counter);
        MARSHAL_TEMPLVALUE(player_counter);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(visible_name, title, title_pic, model_src, identify_pic_src);
		UNMARSHAL_TEMPLVALUE(level, faction, properties, normal_attack_id, skill_group);
		UNMARSHAL_TEMPLVALUE(ai_strategy_id, killed_exp, killed_money, normal_drop_prob, normal_droptable_id);
		UNMARSHAL_TEMPLVALUE(special_droptable_id, visible_type, info_display_pos, combat_body_size, gender);
        UNMARSHAL_TEMPLVALUE(skill_declare, level_display_mode, underfoot_effect, cat_vision, show_boss_alert);
        UNMARSHAL_TEMPLVALUE(full_portrait, world_boss_award_tid, cat_vision_hint, cat_vision_exp, global_counter);
        UNMARSHAL_TEMPLVALUE(player_counter);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (model_src.size() <= 0)
            return false;

		if (level <= 0)
			return false;

		if ((int)properties.size() != MAX_PROP_NUM)
			return false;

		if (normal_attack_id <= 0)
			return false;

		if (killed_exp < 0 || killed_money < 0)
			return false;

		for (size_t i = 0; i < skill_group.size(); ++i)
		{
			if (skill_group[i].skill_id <= 0 || skill_group[i].probability <= 0)
				return false;
		}

		if (normal_drop_prob < 0)
			return false;

		// ai script
		if (ai_strategy_id < 0)
		{
			return false;
		}
		else if (ai_strategy_id > 0 && ai_strategy_id != templ_id)
		{
			return false;
		}

		if ( visible_type != VT_COMPLEX && visible_type != VT_WARRIOR 
		  && visible_type != VT_MAGE && visible_type != VT_ARCHER 
		  && visible_type != VT_BOSS)
		{
			return false;
		}

		if (info_display_pos < 0)
			return false;

		if (combat_body_size < 0)
			return false;

		if (gender != GT_MALE && gender != GT_FEMALE)
			return false;

		for (size_t i = 0; i < skill_declare.size(); ++i)
		{
			if (skill_declare[i] <= 0)
				return false;
		}

        if (level_display_mode != LDM_NORMAL && 
            level_display_mode != LDM_NOT_SHOW && 
            level_display_mode != LDM_QUESTION_MARK)
        {
            return false;
        }

        if (cat_vision < 0 || cat_vision > 32)
			return false;

        if (world_boss_award_tid < 0)
            return false;

        if (cat_vision_hint != 0 && cat_vision_hint != 1)
            return false;

        if (cat_vision_exp < 0)
            return false;

        if (global_counter.tid > 0 && global_counter.value < 0)
            return false;

        if (player_counter.tid > 0 && player_counter.value < 0)
            return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_MONSTER_TEMPL_H_
