#ifndef GAMED_GS_GLOBAL_OBJ_QUERY_H_
#define GAMED_GS_GLOBAL_OBJ_QUERY_H_

#include <map>

#include "shared/base/types.h"
#include "shared/base/rwlock.h"


namespace gamed {

class Player;
class Npc;
class AreaObj;
class Matter;

class ObjQuery
{
public:
	static Player*  FindPlayer(RoleID role_id);
	static Npc*     FindNpc(int64_t obj_id);
	static AreaObj* FindAreaObj(int64_t obj_id);
	static Matter*  FindMatter(int64_t obj_id);
	static bool     MapPlayer(RoleID role_id, size_t index);
	static int      UnmapPlayer(RoleID role_id);

private:
	typedef std::map<RoleID, size_t> RoleIdToIndexMap;
	static RoleIdToIndexMap  s_roleid_mapto_index_;
	static shared::RWLock    s_pmap_rwlock_;
};



///
///
///
/// TODO:  以下代码暂时不要使用！
///
///
///

/*
class Player;
class Npc;
*/

/**
 * @brief 最好不要在一个player里直接PlayerReadOnlyGuard来query别的player，
 *        因为可能导致死锁，应该使用World::QueryPlayer()来查询信息。
 *        或者使用MSG来询问对方的信息
 */
/*
class PlayerReadOnlyGuard
{
public:
	PlayerReadOnlyGuard()
		: pplayer_(NULL)
	{ }

	const Player* AttachReadOnly(RoleID roleid)
	{
		pplayer_ = Gmatrix::FindPlayerFromMan(roleid);
		if (NULL != pplayer_)
		{
			pplayer_->Lock();
			if (!pplayer_->IsActived())
			{
				pplayer_->Unlock();
				pplayer_ = NULL;
			}
		}
		return pplayer_;	
	}

	~PlayerReadOnlyGuard()
	{
		if (NULL != pplayer_) 
		{
			pplayer_->Unlock();
		}
	}

private:
	Player* pplayer_;
};

class NpcReadOnlyGuard
{
public:
	NpcReadOnlyGuard()
		: pnpc_(NULL)
	{ }

	const Npc* AttachReadOnly(XID xid)
	{
		if (!xid.IsNpc()) return NULL;

		pnpc_ = Gmatrix::FindNpcFromMan(xid.id);
		if (NULL != pnpc_)
		{
			pnpc_->Lock();
			if (!pnpc_->IsActived())
			{
				pnpc_->Unlock();
				pnpc_ = NULL;
			}
		}
		return pnpc_;
	}

	~NpcReadOnlyGuard()
	{
		if (NULL != pnpc_)
		{
			pnpc_->Unlock();
		}
	}

private:
	Npc* pnpc_;
};
*/

} // namespace gamed

#endif // GAMED_GS_GLOBAL_OBJ_QUERY_H_
