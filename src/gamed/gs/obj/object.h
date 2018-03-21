#ifndef GAMED_GS_OBJ_OBJECT_H_
#define GAMED_GS_OBJ_OBJECT_H_

#include <vector>

#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"

#include "gs/global/game_types.h"
#include "gs/global/message.h"
#include "gs/global/math_types.h"


namespace gamed {

class Object : public shared::noncopyable
{
public:
	Object();
	virtual ~Object();

	virtual void    Lock() { }
	virtual void    Unlock() { }

private:
};

class World;
class WorldObject : public Object
{
public:
	WorldObject();
	virtual ~WorldObject();

	int     DispatchMessage(const MSG& msg);

	// base virtual func
	virtual void    Lock();
	virtual void    Unlock();
	virtual bool    IsLocked();
	virtual void    Release();

	// SetActive: insert into ObjManager, ClrActive: remove from ObjManager
	virtual bool    IsActived() const;
	virtual void    SetActive();
	virtual void    ClrActive();

	// need subclass overwrite
	virtual int     OnDispatchMessage(const MSG& msg) { return 0; }
	virtual void    OnHeartbeat() { }

	// inline func	
	inline World*  world_plane() const;
	inline void    set_world_plane(World* plane);
	inline void    set_world_id(MapID worldid, MapTag worldtag);
	inline MapID   world_id() const;
	inline MapTag  world_tag() const;
	const  XID&    world_xid() const;

	inline void    set_xid(XID::IDType id, XID::Type type);
	inline int64_t object_id() const;
	inline const XID& object_xid() const;

	inline const A2DVECTOR& pos() const;
	inline void      set_pos(const A2DVECTOR& new_pos);
	inline uint8_t   dir() const;
	inline void      set_dir(uint8_t new_dir);


public:
	///
	/// 以下的Send函数原则上只能由自身、自己的imp类或者自己的成员类调用（人肉保证）
	///
	/**
	 * @brief SendMsg
	 *    object间发送消息
	 */
	void SendMsg(int message, const XID& target, int64_t param = 0);
	void SendMsg(int message, const XID& target, const void* buf, size_t len);
	void SendMsg(int message, const XID& target, int64_t param, const void* buf, size_t len);

	/**
	 * @brief SendPlaneMsg 
	 *    没有指定target则会发给object所属的地图
	 */
	void SendPlaneMsg(int message, int64_t param = 0);
	void SendPlaneMsg(int message, const void* buf, size_t len);
	void SendPlaneMsg(int message, int64_t param, const void* buf, size_t len);

	/**
	 * @brief SendPlaneMsg 
	 *    object发给别的地图消息，用以下接口，需要指定target
	 */
	void SendPlaneMsg(int message, const XID& target, int64_t param = 0);
	void SendPlaneMsg(int message, const XID& target, const void* buf, size_t len);
	void SendPlaneMsg(int message, const XID& target, int64_t param, const void* buf, size_t len);


protected:
	int  MessageHandler(const MSG& msg);
	void DoHeartbeat();


protected:
	shared::MutexLock    mutex_;


private:
  // WorldObject runtime data
	MapID       world_id_;
	MapTag      world_tag_;  // for instance map
	A2DVECTOR   pos_;
	uint8_t     dir_;        // direction 朝向、方向

	XID         xid_;
	World*      plane_;
	bool        is_actived_; // object是actived的才能使用
};


///
/// lock guard
///
class WorldObjectLockGuard
{
public:
	explicit WorldObjectLockGuard(WorldObject* pobj)
		: pobj_(pobj)
	{
		pobj_->Lock();
	}

	~WorldObjectLockGuard()
	{
		pobj_->Unlock();
	}

private:
	WorldObject* pobj_;
};
#define WorldObjectLockGuard(x) SHARED_STATIC_ASSERT(false); // missing guard var name


///
/// inline func
///
inline World* WorldObject::world_plane() const
{
	return plane_;
}

inline void WorldObject::set_world_plane(World* plane)
{
	plane_ = plane;
}

inline void WorldObject::set_world_id(MapID worldid, MapTag worldtag)
{
	world_id_  = worldid;
	world_tag_ = worldtag;
}

inline MapID WorldObject::world_id() const
{
	return world_id_;
}
	
inline MapTag WorldObject::world_tag() const
{
	return world_tag_;
}

inline void WorldObject::set_xid(XID::IDType id, XID::Type type)
{
	xid_.id   = id;
	xid_.type = type;
}

inline int64_t WorldObject::object_id() const
{
	return xid_.id;
}

inline const XID& WorldObject::object_xid() const
{
	return xid_;
}

inline const A2DVECTOR& WorldObject::pos() const
{
	return pos_;
}
	
inline void WorldObject::set_pos(const A2DVECTOR& new_pos)
{
	pos_ = new_pos;
}

inline uint8_t WorldObject::dir() const
{
	return dir_;
}
	
inline void WorldObject::set_dir(uint8_t new_dir)
{
	dir_ = new_dir;
}

} // namespace gamed

#endif // GAMED_GS_OBJ_OBJECT_H_
