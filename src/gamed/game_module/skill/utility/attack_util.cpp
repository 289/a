#include "attack_util.h"
#include "player_util.h"
#include "obj_interface.h"
#include "buff_wrapper.h"
#include "skill_def.h"
#include "skill_templ_manager.h"
#include "effect_templ.h"

namespace skill
{

void AttackUtil::Attack(Player* player, BuffDamageVec& buffdmg_vec)
{
	BuffDamageVec buff_dmg_vec = buffdmg_vec;
	buffdmg_vec.clear();
	while (!buff_dmg_vec.empty())
	{
		BuffDamageVec new_buff_dmg;
		InnerMsg msg(&new_buff_dmg, NULL);

		BuffDamageVec::iterator vit = buff_dmg_vec.begin();
		for (; vit != buff_dmg_vec.end(); ++vit)
		{
			BuffDamage& buff_dmg = *vit;
			Player* attacker = PlayerUtil::FindPlayer(player, buff_dmg.attacker);
			Player* defender = PlayerUtil::FindPlayer(player, buff_dmg.defender);
			const EffectTempl* effect = GetEffectTempl(buff_dmg.effectid);
			AttackedParam param(attacker, effect, &(buff_dmg.dmgs));
			msg.param = &param;
			BuffWrapper* buff = defender->GetOwner()->GetBuff();
			buff->BeAttacked(msg);
		}

		buffdmg_vec.insert(buffdmg_vec.end(), buff_dmg_vec.begin(), buff_dmg_vec.end());
		buff_dmg_vec = new_buff_dmg;
	}
}

static int32_t AttackPlayer(Player* player, Player* attacker, PlayerDamageVec& playerdmg_vec, BuffDamageVec& buffdmg_vec)
{
	AttackedParam param(attacker);
	InnerMsg msg(&buffdmg_vec, &param);

	int32_t status = RECAST_NONE;
	PlayerDamageVec::iterator pit = playerdmg_vec.begin();
	for (; pit != playerdmg_vec.end(); ++pit)
	{
		PlayerDamage& player_dmg = *pit;
		Player* defender = PlayerUtil::FindPlayer(player, player_dmg.defender);
		BuffWrapper* buff = defender->GetOwner()->GetBuff();		
		EffectDamageVec& effect_dmg_vec = player_dmg.dmgs;
		EffectDamageVec::iterator eit = effect_dmg_vec.begin();
		for (; eit != effect_dmg_vec.end(); ++eit)
		{
			status |= eit->status & EFFECT_CRIT ? RECAST_CRIT : RECAST_NONE;
			param.effect = GetEffectTempl(eit->effectid);
			param.dmg_vec = &(eit->dmgs);
			status |= buff->BeAttacked(msg);
		}
	}
	return status;
}

int32_t AttackUtil::Attack(Player* player, SkillDamage& skill_dmg, BuffDamageVec& buffdmg_vec)
{
	int32_t status = RECAST_NONE;
	Player* attacker = PlayerUtil::FindPlayer(player, skill_dmg.attacker);
	FrameDamageVec& frame_vec = skill_dmg.frames;
	FrameDamageVec::iterator vit = frame_vec.begin();
	for (; vit != frame_vec.end(); ++vit)
	{
		FrameDamage& frame_dmg = *vit;
		status |= AttackPlayer(player, attacker, frame_dmg.players, buffdmg_vec);
		status |= AttackPlayer(player, attacker, frame_dmg.redir_players, buffdmg_vec);
	}
	return status;
}

} // namespace skill
