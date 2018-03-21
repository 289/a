#include "logic_trigger_effect.h"
#include "skill_def.h"
#include "effect_templ.h"
#include "effect_logic.h"
#include "buff.h"
#include "util.h"
#include "obj_interface.h"
#include "attack_frame.h"
//#include "cast_proc.h"
#include "skill_templ_manager.h"

namespace skill
{

bool LogicTriggerEffect::RoundStart(InnerMsg& msg) const
{
	return Trigger(msg, UPDATE_ROUNDSTART);
}

bool LogicTriggerEffect::RoundEnd(InnerMsg& msg) const
{
	return Trigger(msg, UPDATE_ROUNDEND);
}

bool LogicTriggerEffect::Trigger(InnerMsg& msg, int32_t point) const
{
	const BuffInfo& info = effect_->buff;
	if (info.type == BUFF_NORMAL || info.type != point)
	{
		return false;
	}

	// 获取本次触发的效果集合
	/*AttackFrame attack_frame;
    FrameVec& frame_vec = attack_frame.redir;
    for (size_t i = 2; i < logic_->params.size(); ++i)
    {
        FrameInfo info;
        info.effect_id = atoi(logic_->params[i].c_str());
        frame_vec.push_back(info);
    }
	int32_t trigger_type = atoi(logic_->params[0].c_str());
	if (trigger_type != TRIGGER_ALL)
    {
		// 随机选取效果
		size_t trigger_num = atoi(logic_->params[1].c_str());
		while (frame_vec.size() > trigger_num)
		{
			int32_t index = Util::Rand(0, frame_vec.size());
			frame_vec.erase(frame_vec.begin() + index);
		}
    }
	FrameVec::const_iterator fit = frame_vec.begin();
	for (; fit != frame_vec.end(); ++fit)
	{
		const EffectTempl* templ = GetEffectTempl(fit->effect_id);
		assert(templ != NULL);
		attack_frame.redir_templ.push_back(templ);
	}

	// 释放触发效果
	FrameDamage frame_dmg;
	CastParam cast_param;
	InnerMsg cast_msg(NULL, &cast_param);
	CastProc proc(caster_, trigger_, &attack_frame, &frame_dmg, &cast_msg);
	proc.CastRedirEffect();

	// 转换成BuffDamage
	BuffDamageVec* buffdmg_vec = msg.buffdmg_vec;
	Buff* buff = static_cast<Buff*>(msg.param);
	PlayerDamageVec& playerdmg_vec = frame_dmg.redir_players;
	PlayerDamageVec::const_iterator pit = playerdmg_vec.begin();
	for (; pit != playerdmg_vec.end(); ++pit)
	{
		const EffectDamageVec& effectdmg_vec = pit->dmgs;
		EffectDamageVec::const_iterator eit = effectdmg_vec.begin();
		for (; eit != effectdmg_vec.end(); ++eit)
		{
			buffdmg_vec->push_back(BuffDamage());
			BuffDamage& buff_dmg = (*buffdmg_vec)[buffdmg_vec->size() - 1];
			buff_dmg.buff_sn = buff->sn_;
			buff_dmg.effectid = eit->effectid;
			buff_dmg.attacker = caster_->GetId();
			buff_dmg.defender = pit->defender;
			buff_dmg.dmgs = eit->dmgs;
		}
	}*/
	return true;
}

} // namespace skill
