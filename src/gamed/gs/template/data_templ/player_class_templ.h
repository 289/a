#ifndef GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_CLASS_TEMPL_H_ 
#define GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_CLASS_TEMPL_H_

#include "base_datatempl.h"
#include "gconfig/player_lvlup_exp_config.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 *    1.玩家职业模板
 */
class PlayerClassTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(PlayerClassTempl, TEMPL_TYPE_PLAYER_CLASS_TEMPL);
public:
	static const int kMaxVisibleNameLen    = UTF8_LEN(16); // 最多支持16个中文字符
	static const int kMaxIconSrcLen        = 512;
	static const int kMaxModelSrcLen       = 512;
	static const int kMaxFullPortrait      = 512;
	static const int kMaxIdentifyPicSrcLen = 512;
	static const int kMaxInitialSkill      = 10;
	static const int kMaxAvailableSkill    = 14;
	static const int kMaxScriptNameLen     = 32;

	struct PropertyEntity
	{
		BoundArray<uint8_t, kMaxVisibleNameLen> name;   // 游戏里属性的显示名
		int8_t  sync_rule;                              // 属性同步规则,表示最大值发生变化时对当前值的影响,只对属性1-3生效
		int32_t value;                                  // 0表示该职业玩家没有此属性
		int32_t levelup_inc;                            // 玩家升级时该属性的增加值，0表示升级时不提高此属性
		NESTED_DEFINE(name, sync_rule, value, levelup_inc);
	};

	inline void set_templ_id(TemplID id) { templ_id = id; }

	enum GEN_POWER_TYPE
	{
		GPT_INVALID,
		GPT_DEFAULT,         //默认恢复类型
		GPT_ATTACK_WARRIOR,  //攻战类
		GPT_DEFENSE_WARRIOR, //防守类
		GPT_MAGE,            //法师类
		GPT_TECHNICIAN,      //技工类
		GPT_MAX,
	};


public:
	// 0
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;        // 游戏里怪物的显示名
	BoundArray<uint8_t, kMaxIconSrcLen>     cls_icon_src;        // 角色的职业图标资源
	BoundArray<uint8_t, kMaxModelSrcLen>    cls_model_src;       // 角色的职业模型资源
	uint16_t       role_cls;                                     // 玩家职业类型
	uint16_t       the_highest_level;                            // 该职业所能升到的最高等级，0表示不限制

	// 5
	BoundArray<uint8_t, kMaxScriptNameLen>  combat_value_script;   // 战斗力计算脚本，脚本名字必须是英文的，默认值为none.lua
	BoundArray<PropertyEntity, MAX_PROP_NUM>  properties;  // 角色基础属性
    BoundArray<TemplID, kMaxInitialSkill> initial_skill_trees;     // 初始技能树列表
    BoundArray<TemplID, kMaxAvailableSkill> skill_trees_base;      // 基础可用技能树列表(职业初始阶段使用)
    BoundArray<TemplID, kMaxAvailableSkill> skill_trees_extend;    // 扩展可用技能树列表(职业二转后才可以使用)

	//10
	SkillID        normal_attack_id;                               // 普攻id
	BoundArray<uint8_t, kMaxFullPortrait>      full_portrait;      // 全身像，立绘
	BoundArray<uint8_t, kMaxIdentifyPicSrcLen> identify_pic_src;   // 图鉴资源，如头像
	int8_t         power_gen_type;                                 // 技能消耗属性恢复类型
	int8_t         combat_body_size; // 战斗体型，默认值0，客户端根据数值到脚本里读取对应的x、y


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(visible_name, cls_icon_src, cls_model_src, combat_value_script, the_highest_level);
		MARSHAL_TEMPLVALUE(role_cls, properties, initial_skill_trees, skill_trees_base, skill_trees_extend);
		MARSHAL_TEMPLVALUE(normal_attack_id, full_portrait, identify_pic_src, power_gen_type, combat_body_size);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(visible_name, cls_icon_src, cls_model_src, combat_value_script, the_highest_level);
		UNMARSHAL_TEMPLVALUE(role_cls, properties, initial_skill_trees, skill_trees_base, skill_trees_extend);
		UNMARSHAL_TEMPLVALUE(normal_attack_id, full_portrait, identify_pic_src, power_gen_type, combat_body_size);
	}

	bool HasEnding(const std::string& fullstring, const std::string& ending) const
	{
		if (fullstring.length() >= ending.length())
		{
			return (0 == fullstring.compare(fullstring.length() - ending.length(),
                                            ending.length(),
                                            ending));
		}

		return false;
	}

	bool CheckCombatValueScript() const
	{
		std::string ending         = ".lua";
		std::string cv_script_name = combat_value_script.to_str();
		if (!HasEnding(cv_script_name, ending))
			return false;

		std::string tmpstr = cv_script_name.substr(0, cv_script_name.length() - ending.length());
		for (size_t i = 0; i < tmpstr.length(); ++i)
		{
			char c = tmpstr[i];
			if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') 
			  || (c >= '0' && c <= '9') || (c == '_'))
				continue;
			else
				return false;
		}

		return true;
	}

	virtual bool OnCheckDataValidity() const
	{
		if ((int)the_highest_level > PlayerLvlUpExpConfig::kMaxLevelCount)
			return false;

		if ((int)properties.size() != MAX_PROP_NUM)
			return false;

		// 检查速度是否合法，0.2米每秒 ~ 6米每秒
		//if (properties[15] < 200 || properties[15] > 6000)
			//return false;

		if (normal_attack_id <= 0)
			return false;

		if (!CheckCombatValueScript())
			return false;

		for (size_t i = 0; i < initial_skill_trees.size(); ++ i)
			if (initial_skill_trees[i] <= 0)
				return false;

		for (size_t i = 0; i < skill_trees_base.size(); ++ i)
			if (skill_trees_base[i] < 0)
				return false;

		for (size_t i = 0; i < skill_trees_extend.size(); ++ i)
			if (skill_trees_extend[i] < 0)
				return false;

		if (skill_trees_base.size() != skill_trees_extend.size())
			return false;

		if (power_gen_type <= GPT_INVALID || power_gen_type >= GPT_MAX)
			return false;

		if (combat_body_size < 0)
			return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_CLASS_TEMPL_H_
