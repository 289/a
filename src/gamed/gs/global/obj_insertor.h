#ifndef GAMED_GS_GLOBAL_OBJ_INSERTOR_H_
#define GAMED_GS_GLOBAL_OBJ_INSERTOR_H_

#include <vector>

#include "gs/obj/npc.h"
#include "gs/obj/matter.h"


namespace gamed {

class ObjInsertor
{
public:
	template <typename T>
	static int push_back(std::vector<T*>& list, T* pObj)
	{
		list.push_back(pObj);
		return 0;
	}

	static int push_back(std::vector<Npc*>& list, Npc* pNpc)
	{
		if (!pNpc->is_idle())
		{
			list.push_back(pNpc);
		}
		return 0;
	}

	static int push_back(std::vector<Matter*>& list, Matter* pMatter)
	{
		if (!pMatter->is_idle())
		{
			list.push_back(pMatter);
		}
		return 0;
	}
};

} // gamed

#endif // GAMED_GS_GLOBAL_OBJ_INSERTOR_H_
