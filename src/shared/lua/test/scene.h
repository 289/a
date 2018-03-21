#ifndef SCENE_H
#define SCENE_H

#include <map>
#include "shared/base/singleton.h"

namespace luabind
{
class LuaEngine;
}

namespace gamed
{

using namespace std;
using namespace shared;

class Player;

class Scene : public Singleton<Scene>
{
	friend class Singleton<Scene>;
public:
	typedef map<int64_t/*roleid*/, Player*> PlayerMap;

	static inline Scene* GetInstance()
	{
		return &(get_mutable_instance());
	}
	
	void AddPlayer(Player* player);
	Player* GetPlayer(int64_t roleid) const;
	void AddBuff(int scriptid);
	void Show();
protected:
	Scene();
	~Scene();
private:
	luabind::LuaEngine* engine_;
	PlayerMap players_;
};

#define s_pScene Scene::GetInstance()

} // namespace gamed

#endif
