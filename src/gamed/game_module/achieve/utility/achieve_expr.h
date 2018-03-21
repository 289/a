#ifndef ACHIEVE_UTILITY_ACHIEVE_EXPR_H_
#define ACHIEVE_UTILITY_ACHIEVE_EXPR_H_

#include "expression.h"

namespace achieve
{

// 技能公式
class AchieveExpr : public Expression
{
public:
	AchieveExpr();
	AchieveExpr(const std::string& exp, void* param);
protected:
	virtual bool IsParamTag(char ch) const;
	virtual double GetParamValue(const std::string& param) const;
};

} // namespace achieve

#endif // ACHIEVE_UTILITY_ACHIEVE_EXPR_H_
