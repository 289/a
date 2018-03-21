#include "skill_templ.h"
#include "effect_templ.h"

namespace skill
{

INIT_STATIC_SKILLSYS_TEMPL(SkillTempl, TEMPL_TYPE_SKILL);

void SkillTempl::OnMarshal()
{
    MARSHAL_SKILLSYS_TEMPL_VALUE(type, magician, team, scope, action, flag);
	MARSHAL_SKILLSYS_TEMPL_VALUE(hit, cooldown, recast);
	MARSHAL_SKILLSYS_TEMPL_VALUE(target, range);
	MARSHAL_SKILLSYS_TEMPL_VALUE(camera);
	MARSHAL_SKILLSYS_TEMPL_VALUE(premise, consume);
	MARSHAL_SKILLSYS_TEMPL_VALUE(trigger_mode, attack_frame);
	MARSHAL_SKILLSYS_TEMPL_VALUE(skill_group);
	MARSHAL_SKILLSYS_TEMPL_VALUE(cast_type, cast_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(move_frame, attack_path, attack_gfx_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(atb_stop_time);
}

void SkillTempl::OnUnmarshal()
{
    UNMARSHAL_SKILLSYS_TEMPL_VALUE(type, magician, team, scope, action, flag);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(hit, cooldown, recast);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(target, range);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(camera);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(premise, consume);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(trigger_mode, attack_frame);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(skill_group);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(cast_type, cast_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(move_frame, attack_path, attack_gfx_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(atb_stop_time);
}

// 检测数据有效性
bool SkillTempl::OnCheckDataValidity() const
{
	CHECK_INRANGE(type, SKILL_ACTIVE, SKILL_PASSIVE)
	CHECK_INRANGE(magician, MAGICIAN_PLAYER, MAGICIAN_SCENE)
	CHECK_INRANGE(team, TEAM_MATE, TEAM_ENEMY)
	CHECK_INRANGE(scope, SCOPE_SCENE, SCOPE_GLOBAL)
	CHECK_INRANGE(action, ACTION_MELEE, ACTION_RANGE)
	CHECK_INRANGE(trigger_mode, TRIGGER_NONE, TRIGGER_GFX)
	CHECK_INRANGE(cast_type, CAST_ACTION, CAST_GFX)
	CHECK_VALIDITY(hit)
	CHECK_VALIDITY(cooldown)
	CHECK_VALIDITY(recast)
	CHECK_VALIDITY(target)
	CHECK_VALIDITY(range)
	CHECK_VEC_VALIDITY(camera)
	CHECK_VEC_VALIDITY(premise)
	CHECK_VEC_VALIDITY(consume)
	CHECK_VEC_VALIDITY(attack_frame)
    if (move_frame < 0 || flag < 0)
    {
        return false;
    }
	return true;
}

int32_t SkillTempl::summon_golemid() const
{
    int32_t golemid = 0;
    if (target.type != TARGET_SELF)
    {
        return golemid;
    }

    const EffectTemplVec* templ_vec = NULL;
    AttackFrameVec::const_iterator ait = attack_frame.begin();
    for (; ait != attack_frame.end(); ++ait)
    {
        for (int32_t i = 0; i < 2; ++i)
        {
            templ_vec = i == 0 ? &(ait->effect_templ) : &(ait->redir_templ);
            EffectTemplVec::const_iterator tit = templ_vec->begin();
            for (; tit != templ_vec->end(); ++tit)
            {
                if ((golemid = (*tit)->summon_golemid()) != 0)
                {
                    return golemid;
                }
            }
        }
    }
    return golemid;
}

} // namespace skill
