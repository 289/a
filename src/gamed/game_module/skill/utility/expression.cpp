#include <stdint.h>
#include <stdlib.h>
#include "expression.h"

namespace skill
{

using namespace std;

Expression::Expression()
	: minus_(false), exp_(""), param_(NULL)
{
}

Expression::Expression(const string& exp, void* param)
	: param_(param)
{
    minus_ = exp[0] == '-';
    exp_ = minus_ ? string(exp, 1) : exp;
}

Expression::Expression(const Expression& rhs)
	: minus_(rhs.minus_), exp_(rhs.exp_), param_(rhs.param_)
{
}

Expression& Expression::operator=(const Expression& rhs)
{
    minus_ = rhs.minus_;
	exp_ = rhs.exp_;
	param_ = rhs.param_;
	return *this;
}

void Expression::operator=(const string& exp)
{
    minus_ = exp[0] == '-';
    exp_ = minus_ ? string(exp, 1) : exp;
}

static bool IsBracketMatch(const string& exp)
{
	stack<char> stack;
	for (size_t i = 0; i < exp.size(); ++i)
	{
		char ch = exp[i];
		if (ch != '(' && ch != ')')
		{
			continue;
		}
		if (ch == '(')
		{
			stack.push(ch);
		}
		else if (ch == ')' && stack.empty())
		{
			return false;
		}
		else
		{
			stack.pop();
		}
	}
	return stack.empty();
}

static bool IsNumber(char ch)
{
	int32_t num = ch - '0';
	return num >= 0 && num <= 9;
}

static bool IsOperator(char ch)
{
	return ch == '(' || ch == ')' || ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

queue<string> Expression::DivideExpression()
{
	queue<string> que;
	if (!IsBracketMatch(exp_))
	{
		return que;
	}

	string str = "";
	bool is_num = false;
	bool is_param = false;
	size_t size = exp_.size();
	for (size_t i = 0; i < size; ++i)
	{
		char ch = exp_[i];		
		if (ch == ' ')
		{
			continue;
		}
		if (IsNumber(ch) && !is_param)
		{			
			is_num = true;
		}
		else if (IsOperator(ch))
		{
			is_num = false;
			is_param = false;
		}
		else if (!is_param) 
		{
			is_param = true;
		}	

		if (is_num || is_param)
		{
			str += ch;
			if (i == size - 1)
			{
				que.push(str);
			}
		}
		else
		{
			if (str.size() != 0)
			{
				que.push(str);
			}
			str = ch;
			que.push(str);
			str = "";
		}
	}
	return que;
}

stack<string> Expression::ChangeToSuffix()
{
	queue<string> que;
	stack<string> stack_A;
	stack<string> stack_B;
	que = DivideExpression();   //取得中缀表达式数列
	if (que.empty())
	{
		return stack_B;
	}

	string str;
	while (!que.empty())
	{
		str = que.front();
		que.pop();
		if (str == "(")
		{
			stack_B.push(str);
		}
		else if (str == ")")
		{
			while(!stack_B.empty() && stack_B.top()!="(")
			{
				stack_A.push(stack_B.top());
				stack_B.pop();
			}
			if(!stack_B.empty())
			{
				stack_B.pop();
			}
		}
		else if(str=="+"||str=="-")
		{
			if(stack_B.empty()||stack_B.top()=="(")
			{
				stack_B.push(str);
			}
			else
			{
				while(!stack_B.empty()&&stack_B.top()!="(")
				{
					stack_A.push(stack_B.top());
					stack_B.pop();
				}
				stack_B.push(str);
			}
		}
		else if(str=="*"||str=="/")
		{
			if(stack_B.empty()||stack_B.top()=="+"||stack_B.top()=="-"||stack_B.top()=="(")
			{
				stack_B.push(str);
			}
			else
			{
				stack_A.push(stack_B.top());
				stack_B.top();
				stack_B.push(str);
			}
		}
		else 
		{
			stack_A.push(str);
		}
	}
	while(!stack_B.empty())
	{
		if(stack_B.top()!="(")
		{
			stack_A.push(stack_B.top());
		}
		stack_B.pop();
	}
	while(!stack_A.empty())
	{ 
		stack_B.push(stack_A.top());
		stack_A.pop();
	}

	return stack_B;
}

double Expression::Calculate()
{
	stack<string>stack_A = ChangeToSuffix();
	if(stack_A.empty())
	{
		return 0;
	}

	stack<double>stack;
	string str;
	char ch;
	double dbl;
	while(!stack_A.empty())
	{ 
		str=stack_A.top();
		stack_A.pop();
		ch=str.at(0);
		switch(ch)
		{
		case '+':
			dbl=stack.top();stack.pop();
			dbl+=stack.top();
			stack.pop();
			stack.push(dbl);
		break;
		case '-':
			dbl=stack.top();
			stack.pop();
			dbl=stack.top()-dbl;
			stack.pop();
			stack.push(dbl);
		break;
		case '*':
			dbl=stack.top();
			stack.pop();
			dbl*=stack.top();
			stack.pop();
			stack.push(dbl);
		break;
		case '/':
			dbl=stack.top();
			stack.pop();
			if(dbl != 0.000)  //除数不为0
			{
				dbl = stack.top() / dbl;
				stack.pop();
				stack.push(dbl);
			}
		break;
		default:
			//将字符串所代表的操作数转换成双精度浮点数，并进栈
			if (ch >= '0' && ch <= '9')
			{
				stack.push(atof(str.c_str()));
			}
			else if (IsParamTag(ch))
			{
				stack.push(GetParamValue(str));
			}
			break;
		}
	}

    double res = stack.top();
    return minus_ ? -1 * res : res;
}

bool Expression::IsParamTag(char ch) const
{
	return false;
}

double Expression::GetParamValue(const string& param) const
{
	return 0;
}

} // namespace skill
