#include <stdio.h>
#include <time.h>
#include <map>
#include "combat.h"
#include "combat_player.h"
#include "skill_info.h"
#include "player_util.h"
#include "util.h"
#include "skill_expr.h"

using namespace gamed;
using namespace skill;

static std::vector<int32_t> skill_vec;
static std::string model = "";

static void ShowPlayerDmg(const PlayerDamageVec& dmg_vec)
{
	PlayerDamageVec::const_iterator pit = dmg_vec.begin();
	for (; pit != dmg_vec.end(); ++pit)
	{
		const PlayerDamage& player_dmg = *pit;
		__PRINTF("	defender=%ld", player_dmg.defender);
		const EffectDamageVec& effect_dmg_vec = player_dmg.dmgs;
		EffectDamageVec::const_iterator eit = effect_dmg_vec.begin();
		for (; eit != effect_dmg_vec.end(); ++eit)
		{
			const EffectDamage& effect_dmg = *eit;
			__PRINTF("		effectid=%d status=%d", effect_dmg.effectid, effect_dmg.status);
			const DamageVec& dmg_vec = effect_dmg.dmgs;
			DamageVec::const_iterator dit = dmg_vec.begin();
			for (; dit != dmg_vec.end(); ++dit)
			{
				const Damage& damage = *dit;
				__PRINTF("			prop=%d value=%d", damage.prop, damage.value);
			}
		}
	}
}

static void ShowSkillDmg(SkillDamage& dmg)
{
	__PRINTF("skillid=%d attacker=%ld", dmg.skillid, dmg.attacker);
	FrameDamageVec& frame_dmg_vec = dmg.frames;
	FrameDamageVec::const_iterator fit = frame_dmg_vec.begin();
	for (; fit != frame_dmg_vec.end(); ++fit)
	{
		const FrameDamage& frame_dmg = *fit;
		ShowPlayerDmg(frame_dmg.players);
		ShowPlayerDmg(frame_dmg.redir_players);
	}
}
// 测试战士
static void Init()
{
    //skill_vec.push_back(543);	
    //skill_vec.push_back(544);	
    //skill_vec.push_back(546);	
    skill_vec.push_back(549);	
    model = "/assets/config/model/wanjia/miao/zhanshi2gong/wanjia_miao_zhanshi2gong.xml";
}
// 测试法师
/*static void Init()
{
    skill_vec.push_back(163);
    model = "/assets/config/model/wanjia/miao/fashi1/wanjia_miao_fashi1.xml";
}
// 测试射手
static void Init()
{
    //skill_vec.push_back(308);
    skill_vec.push_back(310);
    model = "/assets/config/model/wanjia/miao/sheshou1/wanjia_miao_sheshou1.xml";
}*/

int main()
{
    SkillExpr expr("(P1,1000+P5,200)*(P3,1-P4.1)+100", NULL);
    expr.Calculate();
    return 0;

	InitSkillData("skill.gbd");
	InitActionData("action.gbd");

	Init();
	Combat::Clear();
	for (int8_t i = 1; i <=2; ++i)
	{
		for (int8_t j = 0; j < 4; ++j)
		{
			CombatPlayer* player = new CombatPlayer(i * 1000 + j , j, i);
			Combat::AddPlayer(i, player);
			ObjInterface* obj = Combat::GetPlayer(i, j);
			SkillWrapper* skill = obj->GetSkill();

			for (size_t index = 0; index < skill_vec.size(); ++index)
			{
				skill->AddSkill(skill_vec[index]);
			}
			skill->set_default_skillid(skill_vec[0]);
		}
	}

	for (int32_t i = 1; i <=2; ++i)
	{
		for (int32_t j = 0; j < 4; ++j)
		{
			ObjInterface* attacker = Combat::GetPlayer(i, j);
			BuffWrapper* buff = attacker->GetBuff();
			buff->CombatStart();
		}
	}

	int32_t round = 0;
	while (true)
	{
		++round;
		for (int8_t team = 1; team <= 2; ++team)
		{
			if (Combat::IsTeamAllDead(team))
			{
				__PRINTF("******team=%d fail, round=%d******", team, round);
				return 0;
			}
		}

		for (int32_t i = 1; i <=2; ++i)
		{
			for (int32_t j = 0; j < 4; ++j)
			{
				ObjInterface* attacker = Combat::GetPlayer(i, j);
				if (!PlayerUtil::CanAction(attacker))
				{
					continue;
				}
				for (int8_t team = 1; team <= 2; ++team)
				{
					if (Combat::IsTeamAllDead(team))
					{
						__PRINTF("******team=%d fail, round=%d******", team, round);
						return 0;
					}
				}

				Combat::Show();
				BuffDamageVec buff_dmg;
				BuffWrapper* buff = attacker->GetBuff();
				SkillWrapper* skill = attacker->GetSkill();
				buff->RoundStart(buff_dmg);
				buff_dmg.clear();

				bool recast = false;
				int32_t skillid = skill_vec[0];
				//if (round == 1)
				//{
					//skillid = skill_vec[0];
				//}
				//else
				//{
					//skillid = round % 2 == 0 ? skill_vec[1] : skill_vec[0];
				//}
                SkillDamageVec skill_dmg_vec;
				while (skillid != 0)
				{
					PlayerVec target;
					SkillDamage skill_dmg;
					int32_t pos = POS_INVALID;
					if ((pos = skill->GetTarget(skillid, target)) == POS_INVALID)
					{
						__PRINTF("attacker=%d cast skillid=%d", attacker->GetId(), skillid);
						break;
					}
					skill->CastSkill(skillid, target, skill_dmg, recast);
					skillid = buff->CastSkill(skill_dmg, buff_dmg);
					buff_dmg.clear();
					recast = skillid != 0;
					__PRINTF("************************************************");
					ShowSkillDmg(skill_dmg);
					__PRINTF("************************************************");
                    skill_dmg_vec.push_back(skill_dmg);
				}

                PauseTime pause;
                GetPauseTime(model, skill_dmg_vec, pause);

				buff->RoundEnd(buff_dmg);
				buff_dmg.clear();
                skill_dmg_vec.clear();
			}
		}
	}

	for (int32_t i = 1; i <=2; ++i)
	{
		for (int32_t j = 0; j < 4; ++j)
		{
			ObjInterface* attacker = Combat::GetPlayer(i, j);
			BuffWrapper* buff = attacker->GetBuff();
			buff->CombatEnd();
		}
	}
	return 0;
}
