#include "base_skill_templ.h"

namespace skill
{

using namespace shared;
using namespace shared::net;
using namespace std;

BaseTempl::BaseTempl(TemplType type)
	: BasePacket(type)
{
}

BaseTempl::~BaseTempl()
{
}

void BaseTempl::Marshal()
{
	MARSHAL_SKILLSYS_TEMPL_VALUE(templ_id);
	OnMarshal();
}

void BaseTempl::Unmarshal()
{
	UNMARSHAL_SKILLSYS_TEMPL_VALUE(templ_id);
	OnUnmarshal();
}

bool BaseTempl::CheckDataValidity() const
{
	return OnCheckDataValidity(); 
}

BaseTemplCreater::BaseTemplCreater()
{
}

BaseTemplCreater::~BaseTemplCreater()
{
}

BaseTempl* BaseTemplCreater::CreatePacket(TemplType type)
{
	return BaseTemplCreater::GetInstance()->OnCreatePacket(type);
}

bool BaseTemplCreater::InsertPacket(uint16_t type, BaseTempl* templ)
{
	return BaseTemplCreater::GetInstance()->OnInsertPacket(type, templ);
}

// 检查模板类型是否合法
bool BaseTemplCreater::IsValidType(int32_t type)
{
	return type == TEMPL_TYPE_SKILL || type == TEMPL_TYPE_EFFECT;
}

BaseTempl* BaseTemplCreater::OnCreatePacket(TemplType type)
{
	BaseTemplMap::iterator it = templ_map_.find(type);
	return it == templ_map_.end() ? NULL : dynamic_cast<BaseTempl*>(it->second->Clone());
}

bool BaseTemplCreater::OnInsertPacket(uint16_t type, BaseTempl* templ)
{
	pair<BaseTemplMap::iterator, bool> ret = templ_map_.insert(make_pair(type, templ));
	assert(ret.second == true);
	return ret.second;
}

} // namespace skill
