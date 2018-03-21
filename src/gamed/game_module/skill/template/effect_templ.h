#ifndef SKILL_DATATEMPL_EFFECT_TEMPL_H_
#define SKILL_DATATEMPL_EFFECT_TEMPL_H_

#include "base_skill_templ.h"
#include "redirect.h"
#include "mutex_info.h"
#include "buff_info.h"
#include "bullet.h"
#include "effect_logic.h"

namespace skill
{

class EffectTempl : public BaseTempl
{
	DECLARE_SKILLSYS_TEMPL(EffectTempl, TEMPL_TYPE_EFFECT);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;

	bool life_chain() const;
	bool is_dizzy() const;
	int32_t charged_skillid() const;
    int32_t summon_golemid() const;
public:
	// 基本信息
	std::string name;
	int8_t type;
	int8_t scope;
	int32_t prob;
	bool attack;

	Redirect redir;		// 效果重定向
	MutexInfo mutex;	// 效果互斥
	BuffInfo buff;		// 持续类效果信息
	LogicVec logic;		// 效果逻辑列表

	// 效果分组
	EffectGroupSet effect_group;

	// 美术资源
    Bullet bullet;
	std::string hit_gfx_path;
	std::string hit_gfx_fullscreen_path;
	std::string buff_gfx_path;
	std::string buff_action;
	std::string icon_path;
    std::string hit_sound;
    std::string miss_sound;
};

} // namespace skill

#endif // SKILL_DATATEMPL_EFFECT_TEMPL_H_
