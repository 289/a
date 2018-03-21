#include <stdlib.h>
#include <assert.h>
#include "achieve_expr.h"
#include "achieve_interface.h"
#include "achieve_data.h"

namespace achieve
{

using namespace gamed;
using namespace std;

AchieveExpr::AchieveExpr()
{
}

AchieveExpr::AchieveExpr(const std::string& exp, void* param)
	: Expression(exp, param)
{
}

bool AchieveExpr::IsParamTag(char ch) const
{
	return ch == 'V';
}

double AchieveExpr::GetParamValue(const string& param) const
{
	AchieveInterface* player = static_cast<AchieveInterface*>(param_);

    // 获取对应成就系统的数值
    size_t start = param.find("[");
    size_t end = param.find(",");
    int32_t type = atoi(param.substr(start + 1, end - start - 1).c_str());
    start = end;
    end = param.find("]");
    int32_t sub_type = atoi(param.substr(start + 1, end - start - 1).c_str());
    return player->GetAchieveData(type, sub_type);
}

} // namespace skill
