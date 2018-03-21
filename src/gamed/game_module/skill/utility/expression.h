#ifndef SKILL_UTILITY_EXPRESSION_H_
#define SKILL_UTILITY_EXPRESSION_H_

#include <string>
#include <queue>
#include <stack>

namespace skill
{

// 表达式基类
class Expression
{
public:
	Expression();
	Expression(const std::string& exp, void* param);
	Expression(const Expression& rhs);

	Expression& operator=(const Expression& rhs);
	void operator=(const std::string& exp);

	double Calculate();
protected:
	virtual bool IsParamTag(char ch) const;
	virtual double GetParamValue(const std::string& param) const;
	std::queue<std::string> DivideExpression();
	std::stack<std::string> ChangeToSuffix();
protected:
    bool minus_;
	std::string exp_; 
	void* param_;
};

} // namespace skill

#endif // SKILL_UTILITY_EXPRESSION_H_
