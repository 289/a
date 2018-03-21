#ifndef GAMED_GS_SCENE_SLICE_H_
#define GAMED_GS_SCENE_SLICE_H_

#include <math.h>
#include <set>

#include "shared/base/assertx.h"
#include "shared/base/mutex.h"
#include "shared/base/noncopyable.h"
#include "shared/logsys/logging.h"

#include "gs/global/math_types.h"


namespace gamed {

using namespace shared;

class WorldObject;
class Npc;
class Player;
class Matter;
class AreaObj;

class Slice : shared::noncopyable
{
	friend class Grid;
public:
	enum 
	{
		EDGE        = 0x0001,
		BORDER      = 0x0002,
		OUTSIDE     = 0x0004,
		INSIDE      = 0x0008,
		SENSITIVE   = 0x0010,
	};

	Slice();
	~Slice();

	// ---- thread safe ----
	void InsertPlayer(const Player* pplyaer);
	void RemovePlayer(const Player* pplyaer);

	// ---- thread safe ----
	void InsertNPC(const Npc* pnpc);
	void RemoveNPC(const Npc* pnpc);

	// ---- thread safe ----
	void InsertAreaObj(const AreaObj* parea);
	void RemoveAreaObj(const AreaObj* parea);

	// ---- thread safe ----
	void InsertMatter(const Matter* pmatter);
	void RemoveMatter(const Matter* pmatter);

	// ---- thread safe ----
	int MovePlayer(const Player* pplayer, Slice* dest);
	int MoveNPC(const Npc* pnpc, Slice* dest);

	//inline void Lock();
	//inline void Unlock();

	inline bool IsEdge() const { return flag_ & EDGE; }
	inline bool IsBorder() const { return flag_ & BORDER; }
	inline bool IsInWorld() const { return flag_ & INSIDE; }
	inline bool IsOutside(float x, float y) const { return slice_range_.IsOut(x, y); }
	inline bool IsInside(float x, float y) const {return slice_range_.IsIn(x, y); }

	inline rect slice_range() const { return slice_range_; }
	inline void set_slice_range(rect& range);

	inline float Distance(Slice* pslice) const;

	inline int NumberOfPlayer() const;
	inline int NumberOfNPC() const;
	inline int NumberOfMatter() const;

	// **** thread unsafe ****
	//inline ObjectSet& GetPlayerList();


private:
	typedef std::set<const WorldObject*> ObjectSet;
	// ---- thread unsafe ----
	bool insertObject(const WorldObject* pobject, ObjectSet& object_set);
	int  removeObject(const WorldObject* pobject, ObjectSet& object_set);


private:
	MutexLock       player_mutex_;
	ObjectSet       player_set_;

	MutexLock       npc_mutex_;
	ObjectSet       npc_set_;

	MutexLock       area_mutex_;
	ObjectSet       area_set_;

	MutexLock       matter_mutex_;
	ObjectSet       matter_set_;

	int             flag_;
	struct rect     slice_range_;
};

///
/// inline func
///
inline float Slice::Distance(Slice* pslice) const
{
	float dis_x = fabs(pslice->slice_range_.left - slice_range_.left);
	float dis_y = fabs(pslice->slice_range_.top - slice_range_.top);
	return dis_x > dis_y ? dis_x : dis_y;
}

inline int Slice::NumberOfPlayer() const
{
	return player_set_.size();
}

inline int Slice::NumberOfNPC() const
{
	return npc_set_.size();
}

inline int Slice::NumberOfMatter() const
{
	return matter_set_.size();
}

inline void Slice::set_slice_range(rect& range)
{
	slice_range_ = range;
}

} // namespace gamed

#endif // GAMED_GS_SCENE_SLICE_H_
