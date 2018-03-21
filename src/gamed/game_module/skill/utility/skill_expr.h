#ifndef SKILL_UTILITY_SKILL_EXPR_H_
#define SKILL_UTILITY_SKILL_EXPR_H_

#include "expression.h"

namespace skill
{

// 技能公式
class SkillExpr : public Expression
{
public:
	SkillExpr();
	SkillExpr(const std::string& exp, void* param);
protected:
	virtual bool IsParamTag(char ch) const;
	virtual double GetParamValue(const std::string& param) const;
};

} // namespace skill

#endif // SKILL_UTILITY_SKILL_EXPR_H_
