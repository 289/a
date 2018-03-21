#include "ins_func_table.h"

#include "shared/logsys/logging.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world_cluster.h"
#include "gs/scene/world_man.h"


namespace gamed {
namespace insScript {

#define RETURN_0_VALUE(engine) \
	engine.ClearStack(); \
	return 0;

#define RETURN_1_VALUE(engine, value) \
	engine.ClearStack(); \
	engine.PushValue(luabind::LuaValue(value)); \
	return 1;

#define CHECK_WORLD_ID(id) \
    XID tmpxid = MakeWorldXID(id); \
	if (!IS_INS_MAP(tmpxid)) \
	{ \
		LOG_WARN << "副本脚本调用错误！world64_id输入不正确:" << id; \
		return RC_FAILURE; \
	}

#define CHECK_VALUE(value) \
    if (value <= 0) \
    { \
        return RC_FAILURE; \
    }

///
/// function table
///
luabind::CFunction ins_script_func_table[] = {

	// 地图元素
	{ "ActivateMapElem",      gamed::insScript::ActivateMapElem     },
	{ "DeactivateMapElem",    gamed::insScript::DeactivateMapElem   },

	// 任务
	{ "DeliverTask",          gamed::insScript::DeliverTask         },
	{ "DeliverTaskToAll",     gamed::insScript::DeliverTaskToAll    },

	// 计数器
	{ "IncreaseCounter",      gamed::insScript::IncreaseCounter     },	
	{ "DecreaseCounter",      gamed::insScript::DecreaseCounter     },
	{ "AssignmentCounter",    gamed::insScript::AssignmentCounter   },
	{ "GetCounter",           gamed::insScript::GetCounter          },
	{ "LockCounter",          gamed::insScript::LockCounter         },
	{ "UnlockCounter",        gamed::insScript::UnlockCounter       },
	{ "IsCounterLocked",      gamed::insScript::IsCounterLocked     },

	// 计时器
	{ "ActivateClock",        gamed::insScript::ActivateClock       },
	{ "DeactivateClock",      gamed::insScript::DeactivateClock     },

	// 关闭副本
	{ "CloseInstance",        gamed::insScript::CloseInstance       },

    // 地图提示信息
    { "ShowPromptMessage",    gamed::insScript::ShowPromptMessage   },
    { "ShowCountDown",        gamed::insScript::ShowCountDown       },

    // 定点地图元素瞬移
    { "SpotMapElemTeleport",  gamed::insScript::SpotMapElemTeleport },

    // 定点怪移动到某点
    { "SpotMonsterMoveTo",    gamed::insScript::SpotMonsterMoveTo   },
    { "SetSpotMonsterSpeed",  gamed::insScript::SetSpotMonsterSpeed },

};

int func_table_size = sizeof(insScript::ins_script_func_table)/sizeof(luabind::CFunction);


///
/// Anonymous
///
namespace {

	void send_plane_msg(int message, const XID& target, int64_t param)
	{
		ASSERT(target.IsWorld());
		MSG msg;
		BuildMessage(msg, message, target, target, param, NULL, 0);
		Gmatrix::SendWorldMsg(msg);
	}

	void send_plane_msg(int message, const XID& target, const void* buf, size_t len)
	{
		ASSERT(target.IsWorld());
		MSG msg;
		BuildMessage(msg, message, target, target, 0, buf, len);
		Gmatrix::SendWorldMsg(msg);
	}

	void send_object_msg(int message, const XID& target, const XID& source, int64_t param)
	{
		ASSERT(target.IsObject());
		MSG msg;
		BuildMessage(msg, message, target, source, param, NULL, 0);
		Gmatrix::SendObjectMsg(msg, true);
	}

	void send_object_msg(int message, const XID& target, const XID& source, const void* buf, size_t len)
	{
		ASSERT(target.IsObject());
		MSG msg;
		BuildMessage(msg, message, target, source, 0, buf, len);
		Gmatrix::SendObjectMsg(msg, true);
	}

} // Anonymous


/**
 * lua里调用C++函数时，填写的参数顺序和PopValue顺序是反的
 *
 */

///
/// 地图元素
///
int ActivateMapElem(lua_State* state)
{
	// param
	int32_t elem_id    = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(elem_id));
	CHECK_VALUE(engine.PopValue(world64_id));

    CHECK_WORLD_ID(world64_id);

	if (elem_id > 0)
	{
		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_ENABLE_MAP_ELEM, target, elem_id);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: ActivateMapElem()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int DeactivateMapElem(lua_State* state)
{
	// param
	int32_t elem_id    = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(elem_id));
	CHECK_VALUE(engine.PopValue(world64_id));
	
	CHECK_WORLD_ID(world64_id);

	if (elem_id > 0)
	{
		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_DISABLE_MAP_ELEM, target, elem_id);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: DeactivateMapElem()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}


///
/// 任务相关
///
int DeliverTask(lua_State* state)
{
	// param
	int32_t task_id    = 0;
	int64_t role_id    = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(task_id));
	CHECK_VALUE(engine.PopValue(role_id));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (task_id > 0 && role_id > 0)
	{
		XID target;
		MAKE_XID(role_id, target);
		if (!target.IsPlayer())
		{
			LOG_WARN << "副本脚本调用failure: DeliverTask() -- role_id invalid";
			return RC_FAILURE;
		}
		XID source = MakeWorldXID(world64_id);
		send_object_msg(GS_MSG_WORLD_DELIVER_TASK, target, source, task_id);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: DeliverTask()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int DeliverTaskToAll(lua_State* state)
{
    // param
    int32_t task_id    = 0;
    int64_t world64_id = 0;

    // process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(task_id));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

    if (task_id > 0)
	{
        XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_MAP_DELIVER_TASK, target, task_id);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: DeliverTaskToAll()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

///
/// 计数器
///
int IncreaseCounter(lua_State* state)
{
	// param
	int32_t value      = 0;
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(value));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0 && value > 0)
	{
		plane_msg_modify_map_counter param;
		param.op_type = static_cast<int32_t>(MMC_INCREASE);
		param.index   = index;
		param.value   = value;

		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_MODIFY_MAP_COUNTER, target, &param, sizeof(param));
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: IncreaseCounter()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int DecreaseCounter(lua_State* state)
{
	// param
	int32_t value      = 0;
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(value));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0 && value > 0)
	{
		plane_msg_modify_map_counter param;
		param.op_type = static_cast<int32_t>(MMC_DECREASE);
		param.index   = index;
		param.value   = value;

		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_MODIFY_MAP_COUNTER, target, &param, sizeof(param));
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: DecreaseCounter()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int AssignmentCounter(lua_State* state)
{
	// param
	int32_t value      = 0;
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(value));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0)
	{
		plane_msg_modify_map_counter param;
		param.op_type = static_cast<int32_t>(MMC_ASSIGNMENT);
		param.index   = index;
		param.value   = value;

		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_MODIFY_MAP_COUNTER, target, &param, sizeof(param));
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: AssignmentCounter()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int GetCounter(lua_State* state)
{
    // param
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

    int32_t value = -1;
	if (index > 0 && index <= 100)
	{
		XID target = MakeWorldXID(world64_id);
        int32_t worldid  = MAP_ID(target);
        int32_t worldtag = MAP_TAG(target);
        WorldManager* wman = wcluster::FindWorldManager(worldid, worldtag);
        if (wman != NULL && wman->IsActived())
        {
            value = wman->GetMapCounter(index);
        }
    }

    RETURN_1_VALUE(engine, value);
}

int LockCounter(lua_State* state)
{
    // param
	int32_t value      = 0;
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(value));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0)
	{
		plane_msg_lock_map_counter param;
		param.index   = index;
		param.value   = value;

		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_LOCK_MAP_COUNTER, target, &param, sizeof(param));
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: LockCounter()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int UnlockCounter(lua_State* state)
{
    // param
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0)
	{
		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_UNLOCK_MAP_COUNTER, target, index);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: LockCounter()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int IsCounterLocked(lua_State* state)
{
    // param
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

    bool locked = false;
	if (index > 0 && index <= 100)
	{
		XID target = MakeWorldXID(world64_id);
        int32_t worldid  = MAP_ID(target);
        int32_t worldtag = MAP_TAG(target);
        WorldManager* wman = wcluster::FindWorldManager(worldid, worldtag);
        if (wman != NULL && wman->IsActived())
        {
            locked = wman->IsMapCounterLocked(index);
        }
    }

    RETURN_1_VALUE(engine, (locked ? 1 : 0));
}


///
/// 计时器
///
int ActivateClock(lua_State* state)
{
	// param
	int32_t secs       = 0;
	int32_t times      = 0;
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(secs));
	CHECK_VALUE(engine.PopValue(times));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0 && times > 0 && secs > 0)
	{
		plane_msg_active_map_clock param;
		param.index   = index;
		param.times   = times;
		param.seconds = secs;

		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_ACTIVE_MAP_CLOCK, target, &param, sizeof(param));
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: ActivateClock()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int DeactivateClock(lua_State* state)
{
	// param
	int32_t index      = 0;
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	if (index > 0)
	{
		XID target = MakeWorldXID(world64_id);
		send_plane_msg(GS_PLANE_MSG_DEACTIVE_MAP_CLOCK, target, index);
	}
	else
	{
		LOG_WARN << "副本脚本调用failure: DeactivateClock()";
		return RC_FAILURE;
	}

	RETURN_1_VALUE(engine, RC_SUCCESS);
}


///
/// 关闭副本
///
int CloseInstance(lua_State* state)
{
	// param
	int32_t ins_result = 0; // 0表示玩家失败，1表示玩家胜利
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(ins_result));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);

	XID target = MakeWorldXID(world64_id);
	plane_msg_sys_close_ins param;
	param.sys_type = plane_msg_sys_close_ins::ST_SCRIPT;
	if (ins_result == 1)
	{
		param.ins_result = plane_msg_sys_close_ins::PLAYER_VICTORY;
	}
	else
	{
		param.ins_result = plane_msg_sys_close_ins::PLAYER_FAILURE;
	}
	send_plane_msg(GS_PLANE_MSG_SYS_CLOSE_INS, target, &param, sizeof(param));
	
	RETURN_1_VALUE(engine, RC_SUCCESS);
}


///
/// 地图提示信息
///
int ShowPromptMessage(lua_State* state)
{
	// param
    int32_t duration   = 1;  // 提示持续多长时间，最小1秒
    int32_t delay      = 0;  // 延迟几秒后播提示，0表示立即播放
    int32_t index      = 0;  // 应第几句提示语
	int64_t world64_id = 0;

	// process
	luabind::LuaEngine engine;
	engine.Init(state);
	CHECK_VALUE(engine.PopValue(duration));
	CHECK_VALUE(engine.PopValue(delay));
	CHECK_VALUE(engine.PopValue(index));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);
    if (index < 0 || index > 1000)
    {
        LOG_WARN << "ShowPromptMessage的index填写错误!";
        RETURN_1_VALUE(engine, RC_FAILURE);
    }
    duration = (duration > 0) ? duration : 1;

	XID target = MakeWorldXID(world64_id);
    plane_msg_map_prompt_message param;
    param.index    = index;
    param.delay    = delay;
    param.duration = duration;
	send_plane_msg(GS_PLANE_MSG_MAP_PROMPT_MESSAGE, target, &param, sizeof(param));
	
	RETURN_1_VALUE(engine, RC_SUCCESS);
}

int ShowCountDown(lua_State* state)
{
    // param
    int32_t screen_y   = 0; // 屏幕上显示的y坐标
    int32_t screen_x   = 0; // 屏幕上显示的x坐标
    int32_t countdown  = 5; // 倒计时数
    int64_t world64_id = 0;

    // process
	luabind::LuaEngine engine;
	engine.Init(state);
    CHECK_VALUE(engine.PopValue(screen_y));
    CHECK_VALUE(engine.PopValue(screen_x));
	CHECK_VALUE(engine.PopValue(countdown));
	CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);
    if (countdown <= 0)
    {
        LOG_WARN << "ShowCountDown的countdown倒计时数填写错误！";
        RETURN_1_VALUE(engine, RC_FAILURE);
    }
	
    XID target = MakeWorldXID(world64_id);
    plane_msg_map_countdown param;
    param.countdown = countdown;
    param.screen_x  = screen_x;
    param.screen_y  = screen_y;
	send_plane_msg(GS_PLANE_MSG_MAP_COUNTDOWN, target, &param, sizeof(param));
	
    RETURN_1_VALUE(engine, RC_SUCCESS);
}


///
/// 定点地图元素瞬移（npc、怪、矿）
///
int SpotMapElemTeleport(lua_State* state)
{
    // param
    int32_t target_dir = 0;    // 瞬移到目标点后的方向
    float target_pos_y = 0.f;  // 瞬移目标点的y坐标
    float target_pos_x = 0.f;  // 瞬移目标点的x坐标
    int32_t elem_id    = 0;    // 地图元素id，只能是定点npc或怪
    int64_t world64_id = 0;

    // process
    luabind::LuaEngine engine;
    engine.Init(state);
    CHECK_VALUE(engine.PopValue(target_dir));
    CHECK_VALUE(engine.PopValue(target_pos_y));
    CHECK_VALUE(engine.PopValue(target_pos_x));
    CHECK_VALUE(engine.PopValue(elem_id));
    CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);
    if (elem_id <= 0 || 
        (target_pos_x < -128 || target_pos_x > 128) || 
        (target_pos_y < -128 || target_pos_y > 128) ||
        (target_dir < 0 || target_dir > 7))
    {
        LOG_WARN << "SpotMapElemTeleport 传入参数有误！";
        RETURN_1_VALUE(engine, RC_FAILURE);
    }

	XID target = MakeWorldXID(world64_id);
    plane_msg_spot_mapelem_teleport param;
    param.elem_id = elem_id;
    param.pos_x   = target_pos_x;
    param.pos_y   = target_pos_y;
    param.dir     = target_dir;
    send_plane_msg(GS_PLANE_MSG_SPOT_MAPELEM_TELEPORT, target, &param, sizeof(param));

    RETURN_1_VALUE(engine, RC_SUCCESS);
}


///
/// 定点怪移动到某个点
///
int SpotMonsterMoveTo(lua_State* state)
{
    // param
    float speed        = 0.f;  // 移动速度，米每秒
    float target_pos_y = 0.f;  // 移动到目标点的y坐标
    float target_pos_x = 0.f;  // 移动到目标点的x坐标
    int32_t elem_id    = 0;    // 地图元素id，只能是定点npc或怪
    int64_t world64_id = 0;

    // process
    luabind::LuaEngine engine;
    engine.Init(state);
    CHECK_VALUE(engine.PopValue(speed));
    CHECK_VALUE(engine.PopValue(target_pos_y));
    CHECK_VALUE(engine.PopValue(target_pos_x));
    CHECK_VALUE(engine.PopValue(elem_id));
    CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);
    if (elem_id <= 0 || 
        (target_pos_x < -128 || target_pos_x > 128) || 
        (target_pos_y < -128 || target_pos_y > 128) ||
        speed < 0.1)
    {
        LOG_WARN << "SpotMonsterMoveTo 传入参数有误！";
        RETURN_1_VALUE(engine, RC_FAILURE);
    }

    XID target = MakeWorldXID(world64_id);
    plane_msg_spot_monster_move param;
    param.elem_id = elem_id;
    param.pos_x   = target_pos_x;
    param.pos_y   = target_pos_y;
    param.speed   = speed;
    send_plane_msg(GS_PLANE_MSG_SPOT_MONSTER_MOVE, target, &param, sizeof(param));

    RETURN_1_VALUE(engine, RC_SUCCESS);
}

int SetSpotMonsterSpeed(lua_State* state)
{
    // param
    float speed        = 0.f;  // 移动速度，米每秒
    int32_t elem_id    = 0;    // 地图元素id，只能是定点npc或怪
    int64_t world64_id = 0;

    // process
    luabind::LuaEngine engine;
    engine.Init(state);
    CHECK_VALUE(engine.PopValue(speed));
    CHECK_VALUE(engine.PopValue(elem_id));
    CHECK_VALUE(engine.PopValue(world64_id));

	CHECK_WORLD_ID(world64_id);
    if (elem_id <= 0 || speed < 0.1)
    {
        LOG_WARN << "SpotMonsterMoveTo 传入参数有误！";
        RETURN_1_VALUE(engine, RC_FAILURE);
    }

    XID target = MakeWorldXID(world64_id);
    plane_msg_spot_monster_speed param;
    param.elem_id = elem_id;
    param.speed   = speed;
    send_plane_msg(GS_PLANE_MSG_SPOT_MONSTER_SPEED, target, &param, sizeof(param));

    RETURN_1_VALUE(engine, RC_SUCCESS);
}

} // namespace insScript
} // namespace gamed
