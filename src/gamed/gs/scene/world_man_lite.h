#ifndef GAMED_GS_SCENE_WORLD_MANAGER_LITE_H_
#define GAMED_GS_SCENE_WORLD_MANAGER_LITE_H_

#include "world_man.h"


namespace gamed {

class WorldManagerLite : public WorldManager
{
public:
	WorldManagerLite();
	virtual ~WorldManagerLite();

	virtual bool   OnInit() { return true; }
	virtual void   OnHeartbeat() { }

private:
};

} // namespace gamed

#endif // GAMED_GS_SCENE_WORLD_MANAGER_LITE_H_
