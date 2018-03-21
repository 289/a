#include "func_table.h"

#include "shared/lua/lua_value.h"

#include "gs/global/obj_query.h"
#include "gs/global/message.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world.h"
#include "gs/obj/area.h"
#include "gs/eventsys/src/eventsys_def.h"


namespace gamed {
namespace areaEv {

using namespace eventsys;

#define RETURN_1_VALUE(engine, value) \
	engine.ClearStack(); \
	engine.PushValue(luabind::LuaValue(value)); \
	return 1;


///
/// function table
///
luabind::CFunction area_event_func_table[] =
{
	///
	/// Condition Function
	///
	{"CheckPlayerLevel", gamed::areaEv::CheckPlayerLevel},
	{"CheckPlayerClass", gamed::areaEv::CheckPlayerClass},

	///
	/// Action Function
	///
	{"DeliverItem", gamed::areaEv::DeliverItem},
};

int func_table_size = sizeof(areaEv::area_event_func_table)/sizeof(luabind::CFunction);


///
/// Condition Functions 
///    （1）这些函数的返回值是指回传给lua的return参数个数
///    （2）尽量使用RETURN_XX_VALUE宏来返回值
///
int CheckPlayerLevel(lua_State* state)
{
	intptr_t obj_ptr      = NULL;
	RoleID roleid         = 0;
	int compare_op        = -1;
	int require_level     = -1;

	luabind::LuaEngine engine;
	engine.Init(state);
	engine.PopValue(require_level);
	engine.PopValue(compare_op);
	engine.PopValue(roleid);
	engine.PopValue(obj_ptr);

	if (roleid == 0 || require_level == 0 || obj_ptr == 0)
	{
		RETURN_1_VALUE(engine, ES_FAILURE);
	}
	
	AreaObj* parea = reinterpret_cast<AreaObj*>(obj_ptr);
	ASSERT(parea->IsLocked());

	int32_t player_level = -1;
	world::player_base_info baseinfo;
	if (parea->world_plane()->QueryPlayer(roleid, baseinfo))
	{
		player_level = baseinfo.level;
	}
	else
	{
		return ES_FAILURE;
	}

	int retcode = ES_FAILURE;
	switch (compare_op)
	{
		case RO_GREATER_THAN:
			if (player_level > require_level) 
				retcode = ES_SUCCESS;
			break;

		case RO_LESS_THAN:
			if (player_level < require_level)
				retcode = ES_SUCCESS;
			break;

		case RO_EQUAL:
			if (player_level == require_level)
				retcode = ES_SUCCESS;
			break;

		case RO_GREATER_THAN_OR_EQUAL:
			if (player_level >= require_level)
				retcode = ES_SUCCESS;
			break;

		case RO_LESS_THAN_OR_EQUAL:
			if (player_level <= require_level)
				retcode = ES_SUCCESS;
			break;

		case RO_NOT_EQUAL:
			if (player_level != require_level)
				retcode = ES_SUCCESS;
			break;

		default:
			retcode = ES_FAILURE;
			break;
	}
	
	RETURN_1_VALUE(engine, retcode);
}

int CheckPlayerClass(lua_State* state)
{
	intptr_t obj_ptr      = NULL;
	RoleID roleid         = 0;
	int require_class     = -1;

	luabind::LuaEngine engine;
	engine.Init(state);
	engine.PopValue(require_class);
	engine.PopValue(roleid);
	engine.PopValue(obj_ptr);

	if (require_class == 0 || roleid == 0 || obj_ptr == 0)
	{
		RETURN_1_VALUE(engine, ES_FAILURE);
	}
	
	AreaObj* parea = reinterpret_cast<AreaObj*>(obj_ptr);
	ASSERT(parea->IsLocked());

	int32_t player_class = -1;
	world::player_base_info baseinfo;
	if (parea->world_plane()->QueryPlayer(roleid, baseinfo))
	{
		player_class = baseinfo.cls;
	}
	else
	{
		return ES_FAILURE;
	}

	int retcode = ES_FAILURE;
	if (player_class == require_class)
	{
		retcode = ES_SUCCESS;
	}

	RETURN_1_VALUE(engine, retcode);
}


///
/// Action Functions
///    （1）这些函数的返回值是指回传给lua的return参数个数
///    （2）尽量使用RETURN_XX_VALUE宏来返回值
///
int DeliverItem(lua_State* state)
{
	luabind::LuaEngine engine;
	engine.Init(state);

	RETURN_1_VALUE(engine, ES_SUCCESS);
}

} // namespace areaEv
} // namespace gamed

