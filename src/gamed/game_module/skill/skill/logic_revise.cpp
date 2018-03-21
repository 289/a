#include "logic_revise.h"
#include "effect_logic.h"
#include "skill_expr.h"
#include "skill_def.h"

namespace skill
{

using namespace std;

static void CalcRevise(Player* caster, const EffectLogic* logic, int32_t overlap, ReviseParam* revise)
{
	int32_t change_type = atoi(logic->params[1].c_str());
	int32_t& value = change_type == CHANGE_POINT ? revise->point : revise->scale;
	const string& expr = logic->params[2];
	value += (int32_t)(SkillExpr(expr, caster).Calculate()) * overlap;
}

bool LogicRevise::ConsumeRevise(InnerMsg& msg) const
{
	int32_t revise_type = atoi(logic_->params[0].c_str());
	if (revise_type != REVISE_CONSUME)
	{
		return false;
	}

	ReviseParam* revise = static_cast<ReviseParam*>(msg.param);
	CalcRevise(caster_, logic_, overlap_, revise);
	return true;
}

bool LogicRevise::CastRevise(InnerMsg& msg) const
{
	int32_t revise_type = atoi(logic_->params[0].c_str());
	if (revise_type == REVISE_CONSUME || revise_type == REVISE_DAMAGE)
	{
		return false;
	}

	CastParam* param = static_cast<CastParam*>(msg.param);
	ReviseParam& revise = param->revise[revise_type];
	CalcRevise(caster_, logic_, overlap_, &revise);
	return true;
}

bool LogicRevise::AttackedRevise(InnerMsg& msg) const
{
	int32_t revise_type = atoi(logic_->params[0].c_str());
	if (revise_type != REVISE_DAMAGE)
	{
		return false;
	}

	ReviseParam* revise = static_cast<ReviseParam*>(msg.param);
	CalcRevise(caster_, logic_, overlap_, revise);
	return true;
}

} // namespace skill
