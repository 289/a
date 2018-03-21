#ifndef __GAME_MODULE_COMBAT_OBJECT_H__
#define __GAME_MODULE_COMBAT_OBJECT_H__

#include <stdint.h>
#include "combat_msg.h"
#include "combat_types.h"
#include "shared/base/mutex.h"

namespace combat
{

/**
 * @class Object
 * @brief 战斗系统对象池对象基类,
 * @brief 所有使用战斗池的对象都需要从这里继承.
 */
class Object
{
protected:
	XID xid_;
	bool is_active_;
	shared::MutexLock mutex_;

public:
	Object* pPrev;
	Object* pNext;

	Object(char type):
		xid_(type, 0),
		is_active_(false),
		pPrev(NULL),
		pNext(NULL)
	{
	}
	virtual ~Object()
	{
	}

public:
	void Lock()
	{
		mutex_.lock();
	}
	void Unlock()
	{
		mutex_.unlock();
	}
	shared::MutexLock& GetMutex()
	{
		return mutex_;
	}
	char GetType() const
	{
		return xid_.type;
	}
	void SetID(int32_t index)
	{
		xid_.id = MAKE_ID(xid_.type, index);
	}
	UnitID GetID() const
	{
		return xid_.id;
	}
	const XID& GetXID() const
	{
		return xid_;
	}
	void SetActive()
	{
		is_active_ = true;
	}
	bool IsActived() const
	{
		return is_active_;
	}
	void SendMSG(const MSG& msg, size_t tick_latency=0);
    void SendMSG(int message, const XID& target, int64_t param, const void* buf, size_t len);

	virtual void MessageHandler(const MSG& msg) = 0;
	virtual void HeartBeat() = 0;
	virtual void Trace() const = 0;
	virtual void Clear()
	{
		is_active_ = false;
	}
};

};

#endif // __GAME_MODULE_COMBAT_OBJECT_H__
