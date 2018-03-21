#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_MONSTER_GROUP_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_MONSTER_GROUP_H_

#include "base_extratempl.h"


namespace extraTempl {

class MonsterGroupTempl : public BaseExtraTempl
{
	DECLARE_EXTRATEMPLATE(MonsterGroupTempl, TEMPL_TYPE_MONSTER_GROUP_TEMPL);
public:
	static const int kMaxMonsterCount = 10; // 一个怪物组最多支持填写10只怪物

	enum GroupType
	{
		GT_RANDOM = 0,
		GT_FIXED  = 1,
	};

	/**
	 * @brief MonsterInfo is copyable struct
	 */
	struct MonsterInfo
	{
		DataTemplID    monster_tid;      // 怪物在数据模板里的id
		int16_t        monster_gen_prob; // 该怪物的生成概率，单位万分数。默认值为0，即不会遇到该怪物
		uint8_t        position;         // 怪物在战斗场景中的站位，可配内容为：0、1、2、3、4，默认值为0，即随机站位
		bool           is_unique;        // 在一场战斗中，是否只出现一只。默认为false，即出现的数量不限

		NESTED_DEFINE(monster_tid, monster_gen_prob, position, is_unique);

		MonsterInfo()
			: monster_tid(-1), monster_gen_prob(0), position(0), is_unique(false)
		{ }

		inline bool CheckValidity() const
		{
			if (monster_tid < 0)
				return false;
			if (!check_prob_valid(monster_gen_prob))
				return false;
			if (position < 0 || position > 4)
				return false;
			return true;
		}
	};

	inline void set_templ_id(TemplID id) { templ_id = id; }
	virtual std::string TemplateName() const { return "monster_group"; }


public:
	// 0
	uint8_t group_type;           // 对应GroupType枚举，怪物组出现规则分为：随机组合，固定组合。默认为随机组合
	uint8_t team_revise;          // 是否支持组队修正，在怪物组为随机组合时生效，1为支持，0为不支持
	int16_t one_monster_prob;     // 1只怪的概率，单位万分数。默认值为0，即不会遇到一只怪物
	int16_t two_monsters_prob;    // 2只怪的概率，单位万分数。默认值同上
	int16_t three_monsters_prob;  // 3只怪的概率，单位万分数。默认值同上

	// 5
	int16_t four_monsters_prob;   // 4只怪的概率，单位万分数。默认值同上
	BoundArray<MonsterInfo, kMaxMonsterCount> monster_list; // 怪物的具体信息
	int32_t combat_start_hint_id; // 连续战斗时的出场提示
	int32_t combat_common_hint_id;// 连续战斗时的常态提示
	TemplID next_monster_group;   // 连续战斗时下一波怪物的怪物组ID

	// 10
	int32_t sneak_attack_probability;   // 怪物组中怪物偷袭玩家的概率(百分制)
	int32_t sneak_attacked_probability; // 怪物组中怪物被玩家偷袭的概率(百分制)

protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(group_type, team_revise, one_monster_prob, two_monsters_prob, three_monsters_prob);
		MARSHAL_TEMPLVALUE(four_monsters_prob, monster_list, combat_start_hint_id, combat_common_hint_id, next_monster_group);
		MARSHAL_TEMPLVALUE(sneak_attack_probability, sneak_attacked_probability);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(group_type, team_revise, one_monster_prob, two_monsters_prob, three_monsters_prob);
		UNMARSHAL_TEMPLVALUE(four_monsters_prob, monster_list, combat_start_hint_id, combat_common_hint_id, next_monster_group);
		UNMARSHAL_TEMPLVALUE(sneak_attack_probability, sneak_attacked_probability);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (group_type != GT_RANDOM && group_type != GT_FIXED)
			return false;

		if (team_revise != 0 && team_revise != 1)
			return false;

		if (!check_prob_valid(one_monster_prob) ||
			!check_prob_valid(two_monsters_prob) ||
			!check_prob_valid(three_monsters_prob) ||
			!check_prob_valid(four_monsters_prob) )
			return false;

		// 归一化检查
		if (group_type == GT_RANDOM)
		{
			int probability = 0;
			probability = one_monster_prob + two_monsters_prob + three_monsters_prob + four_monsters_prob;
			if (probability != kNormalizedProbValue)
				return false;
		}

		int probability = 0;
		for (size_t i = 0; i < monster_list.size(); ++i)
		{
			if (!monster_list[i].CheckValidity())
				return false;
			probability += monster_list[i].monster_gen_prob;
		}
		// 归一化检查
		if (group_type == GT_RANDOM && probability != kNormalizedProbValue)
			return false;

		if (combat_start_hint_id < 0)
			return false;

		if (combat_common_hint_id < 0)
			return false;

		if (next_monster_group < 0)
			return false;

		if (sneak_attack_probability < 0 || sneak_attack_probability > kNormalizedProbValue)
			return false;

		if (sneak_attacked_probability < 0 || sneak_attacked_probability > kNormalizedProbValue)
			return false;

		return true;
	}
};

} // namespace extraTempl

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_MONSTER_GROUP_H_
