#include "achieve_msg.h"

#define MARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

namespace achieve
{

using namespace shared;
using namespace shared::net;

AchieveNotifyBase::AchieveNotifyBase(uint16_t type)
	: BasePacket(type)
{
}

AchieveNotifyBase::~AchieveNotifyBase()
{
}

void AchieveNotifyBase::Marshal()
{
	MARSHAL_TEMPLVALUE(id);
	OnMarshal();
}

void AchieveNotifyBase::Unmarshal()
{
	UNMARSHAL_TEMPLVALUE(id);
	OnUnmarshal();
}

// 子类需要实现OnMarshal和OnUnmarshal
void AchieveNotifyFinish::OnMarshal()
{
}

void AchieveNotifyFinish::OnUnmarshal()
{
}

void AchieveNotifyComplete::OnMarshal()
{
}

void AchieveNotifyComplete::OnUnmarshal()
{
}

void AchieveNotifyModify::OnMarshal()
{
    MARSHAL_TEMPLVALUE(data_type, data_subtype, value);
}

void AchieveNotifyModify::OnUnmarshal()
{
    UNMARSHAL_TEMPLVALUE(data_type, data_subtype, value);
}

void AchieveNotifyCheckFinish::OnMarshal()
{
}

void AchieveNotifyCheckFinish::OnUnmarshal()
{
}

void AchieveNotifyAward::OnMarshal()
{
}

void AchieveNotifyAward::OnUnmarshal()
{
}

void AchieveNotifyRevive::OnMarshal()
{
}

void AchieveNotifyRevive::OnUnmarshal()
{
}

} // namespace achieve
