#include "task_msg.h"

#define MARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).marshal(buf_);

#define UNMARSHAL_TEMPLVALUE(...) \
	shared::detail::make_define(__VA_ARGS__).unmarshal(buf_);

namespace task
{

using namespace shared;
using namespace shared::net;

TaskNotifyBase::TaskNotifyBase(uint16_t type)
	: BasePacket(type)
{
}

TaskNotifyBase::~TaskNotifyBase()
{
}

void TaskNotifyBase::Marshal()
{
	MARSHAL_TEMPLVALUE(id);
	OnMarshal();
}

void TaskNotifyBase::Unmarshal()
{
	UNMARSHAL_TEMPLVALUE(id);
	OnUnmarshal();
}

void TaskNotifyBase::OnMarshal()
{
}

void TaskNotifyBase::OnUnmarshal()
{
}

// 子类需要实现OnMarshal和OnUnmarshal
void TaskNotifyErr::OnMarshal()
{
	MARSHAL_TEMPLVALUE(err);
}

void TaskNotifyErr::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(err);
}

void TaskNotifyFinish::OnMarshal()
{
	MARSHAL_TEMPLVALUE(succ);
}

void TaskNotifyFinish::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(succ);
}

void TaskNotifyMonsterKilled::OnMarshal()
{
	MARSHAL_TEMPLVALUE(monster_index, monster_count);
}

void TaskNotifyMonsterKilled::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(monster_index, monster_count);
}

void TaskNotifyModify::OnMarshal()
{
	MARSHAL_TEMPLVALUE();
}

void TaskNotifyModify::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE();
}

void TaskNotifyStorageRes::OnMarshal()
{
	MARSHAL_TEMPLVALUE(override, task_list);
}

void TaskNotifyStorageRes::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(override, task_list);
}

void TaskNotifyLimitRes::OnMarshal()
{
	MARSHAL_TEMPLVALUE(num);
}

void TaskNotifyLimitRes::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(num);
}

void TaskNotifyScriptEnd::OnMarshal()
{
	MARSHAL_TEMPLVALUE(skip);
}

void TaskNotifyScriptEnd::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(skip);
}

void TaskNotifyMiniGameEnd::OnMarshal()
{
	MARSHAL_TEMPLVALUE(succ);
}

void TaskNotifyMiniGameEnd::OnUnmarshal()
{
	UNMARSHAL_TEMPLVALUE(succ);
}

} // namespace task
