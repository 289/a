#include "effect_templ.h"

namespace skill
{

INIT_STATIC_SKILLSYS_TEMPL(EffectTempl, TEMPL_TYPE_EFFECT);

void EffectTempl::OnMarshal()
{
	MARSHAL_SKILLSYS_TEMPL_VALUE(name, type, scope, prob, attack);
	MARSHAL_SKILLSYS_TEMPL_VALUE(redir, mutex, buff, logic);
	MARSHAL_SKILLSYS_TEMPL_VALUE(effect_group);
	MARSHAL_SKILLSYS_TEMPL_VALUE(bullet);
	MARSHAL_SKILLSYS_TEMPL_VALUE(hit_gfx_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(hit_gfx_fullscreen_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(buff_gfx_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(buff_action);
	MARSHAL_SKILLSYS_TEMPL_VALUE(icon_path);
	MARSHAL_SKILLSYS_TEMPL_VALUE(hit_sound);
	MARSHAL_SKILLSYS_TEMPL_VALUE(miss_sound);
}

void EffectTempl::OnUnmarshal()
{
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(name, type, scope, prob, attack);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(redir, mutex, buff, logic);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(effect_group);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(bullet);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(hit_gfx_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(hit_gfx_fullscreen_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(buff_gfx_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(buff_action);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(icon_path);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(hit_sound);
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(miss_sound);
}

// 检查数据有效性
bool EffectTempl::OnCheckDataValidity() const
{
	CHECK_INRANGE(type, EFFECT_INSTANT, EFFECT_BUFF)
	CHECK_INRANGE(scope, SCOPE_SCENE, SCOPE_GLOBAL)
	CHECK_INRANGE(prob, 0, 10000)
	CHECK_VALIDITY(redir)
	CHECK_VALIDITY(mutex)
	CHECK_VALIDITY(buff)
	CHECK_VALIDITY(bullet)
	CHECK_VEC_VALIDITY(logic)
	return true;
}

bool EffectTempl::life_chain() const
{
    if (type != EFFECT_BUFF)
    {
        return false;
    }

	LogicVec::const_iterator it = logic.begin();
	for (; it != logic.end(); ++it)
	{
		if (it->type == LOGIC_LIFE_CHAIN)
		{
			return true;
		}
	}
	return false;
}

bool EffectTempl::is_dizzy() const
{
	if (type != EFFECT_BUFF)
	{
		return false;
	}

	LogicVec::const_iterator it = logic.begin();
	for (; it != logic.end(); ++it)
	{
		if (it->type == LOGIC_CONTROL)
		{
			return atoi(it->params[0].c_str()) == CONTROL_DIZZY;
		}
	}
	return false;
}

int32_t EffectTempl::charged_skillid() const
{
    if (type != EFFECT_BUFF)
    {
        return 0;
    }

	LogicVec::const_iterator it = logic.begin();
	for (; it != logic.end(); ++it)
	{
		if (it->type == LOGIC_CHARGE)
		{
            return atoi(it->params[0].c_str());
		}
	}
	return 0;
}

int32_t EffectTempl::summon_golemid() const
{
    if (type != EFFECT_BUFF)
    {
        return 0;
    }

	LogicVec::const_iterator it = logic.begin();
	for (; it != logic.end(); ++it)
	{
		if (it->type == LOGIC_SUMMON_GOLEM)
		{
            return atoi(it->params[0].c_str());
		}
	}
	return 0;
}

} // namespace skill
