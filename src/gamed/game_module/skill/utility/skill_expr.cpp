#include <stdlib.h>
#include <assert.h>
#include "skill_expr.h"
#include "obj_interface.h"

namespace skill
{

using namespace gamed;
using namespace std;

SkillExpr::SkillExpr()
{
}

SkillExpr::SkillExpr(const std::string& exp, void* param)
	: Expression(exp, param)
{
}

// l表示level，p表示prop
bool SkillExpr::IsParamTag(char ch) const
{
	return ch == 'l' || ch == 'p';
}

double SkillExpr::GetParamValue(const string& param) const
{
	ObjInterface* obj = static_cast<ObjInterface*>(param_);
	if (param[0] == 'l')
	{
		return obj->GetLevel();
	}

	int32_t prop_num = atoi(param.substr(4).c_str());
	// 1-19为属性的当前值，1001-1019为该属性未经Buff修正的初始值
	assert((prop_num >=1 && prop_num <= 19) || (prop_num >= 1001 && prop_num <= 1019));
	return obj->GetProperty(prop_num - 1);
}

} // namespace skill
