#include "scene.h"
#include "player.h"
#include "lua_engine.h"
#include "lua_value.h"
#include "func_table.h"

namespace gamed
{

using namespace luabind;

Scene::Scene()
{
	engine_ = new LuaEngine();
	engine_->Init();
	engine_->LoadFile("param.lua");
	engine_->RegisterCFunc(func_table, sizeof(func_table)/sizeof(luabind::CFunction));
}

Scene::~Scene()
{
	if(engine_ != NULL)
	{
		engine_->Destroy();
		delete engine_;
		engine_ = NULL;
	}
}

void Scene::AddPlayer(Player* player)
{
	players_[player->GetRoleId()] = player;
}

Player* Scene::GetPlayer(int64_t roleid) const
{
	return players_.find(roleid) == players_.end() ? NULL : players_.at(roleid);
}

void Scene::AddBuff(int scriptid)
{
	PlayerMap::iterator it = players_.begin();
	for(; it != players_.end(); ++it)
	{
		LuaValueArray args;
		args.push_back(LuaValue(it->first));
		args.push_back(LuaValue(scriptid));
		if(!engine_->Call("addbuf.lua", "addbuf", &args))
		{
			printf("LuaEngine::Call Error\n");
			return;
		}
		int res = 0;
		engine_->PopValue(res);
		printf("res=%d\n", res);
	}
}

void Scene::Show()
{
	PlayerMap::const_iterator it = players_.begin();
	for(; it != players_.end(); ++it)
	{
		it->second->Show();
	}
}

} // namespace gamed
