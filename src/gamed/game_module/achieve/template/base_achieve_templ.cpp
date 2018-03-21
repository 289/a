#include "base_achieve_templ.h"

namespace achieve
{

using namespace shared;
using namespace shared::net;
using namespace std;

BaseTempl::BaseTempl(TemplType type)
	: BasePacket(type), id(0)
{
}

BaseTempl::~BaseTempl()
{
}

void BaseTempl::Marshal()
{
	MARSHAL_SYS_TEMPL_VALUE(id);
	OnMarshal();
}

void BaseTempl::Unmarshal()
{
	UNMARSHAL_SYS_TEMPL_VALUE(id);
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

bool BaseTemplCreater::InsertPacket(TemplType type, BaseTempl* templ)
{
	return BaseTemplCreater::GetInstance()->OnInsertPacket(type, templ);
}

// 检查模板类型是否合法
bool BaseTemplCreater::IsValidType(int32_t type)
{
	return type >= TEMPL_TYPE_ACHIEVE && type <= TEMPL_TYPE_ACHIEVE;
}

BaseTempl* BaseTemplCreater::OnCreatePacket(TemplType type)
{
	BaseTemplMap::iterator it = templ_map_.find(type);
	return it == templ_map_.end() ? NULL : dynamic_cast<BaseTempl*>(it->second->Clone());
}

bool BaseTemplCreater::OnInsertPacket(TemplType type, BaseTempl* templ)
{
	pair<BaseTemplMap::iterator, bool> ret = templ_map_.insert(make_pair(type, templ));
	assert(ret.second == true);
	return ret.second;
}

} // namespace achieve
