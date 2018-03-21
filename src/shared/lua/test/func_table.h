#ifndef GAMED_FUNC_TABLE_H
#define GAME_FUNC_TABLE_H

#include <stdio.h>
#include <vector>
#include "lua_engine.h"
#include "player.h"
#include "scene.h"

namespace gamed
{

using namespace std;

int AddHp(lua_State* state);


luabind::CFunction func_table[] =
{
	{"AddHp", gamed::AddHp},
};

// 该函数在给lua返回值的时候，具体的值通过PushValue压入栈中
// return返回的值是压入栈中值的数目
int AddHp(lua_State* state)
{
	int64_t roleid = 0;
	int inc_hp = 0;

	luabind::LuaEngine engine;
	engine.Init(state);
	engine.PopValue(inc_hp);
	engine.PopValue(roleid);

	Player* player = s_pScene->GetPlayer(roleid);
	if(player != NULL)
	{
		player->AddHp(inc_hp);
	}
	engine.PushValue(2);
	engine.PushValue(100);
	return 2;
}

} // namespace gamed

#endif
