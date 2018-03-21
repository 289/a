#include "inter_msg_queue.h"

#include "message.h"


namespace gamed {

using namespace shared;

namespace {

	template <typename T>
	void SendCurQueue(MsgQueue<T>& queue)
	{
		if (!queue.IsEmpty())
		{
			MsgQueue<T>* pQueue = new MsgQueue<T>();
			pQueue->Swap(queue);
			RunnablePool::AddTask(pQueue);
		}
	}

	template <typename T>
	void SendCurQueueSpec(MsgQueue<T>& queue, int32_t thread_index)
	{
		if (!queue.IsEmpty())
		{
			MsgQueue<T>* pQueue = new MsgQueue<T>();
			pQueue->Swap(queue);
			RunnablePool::AddSpecTask(pQueue, thread_index);
		}
	}

} // Anonymous


///
/// ObjMsgQueueList
/// 
ObjMsgQueueList::ObjMsgQueueList(MsgQueue<MSG>::DispatchCB cb)
	: cur_queue_(cb)
{
}

ObjMsgQueueList::~ObjMsgQueueList()
{
}

void ObjMsgQueueList::AddMsg(const MSG& msg)
{
	MutexLockTimedGuard lock(mutex_cur_);
	MSG dupe_msg;
	CopyMessage(dupe_msg, msg);
	cur_queue_.AddMsg(dupe_msg);
}

void ObjMsgQueueList::Dispatch()
{
	MutexLockTimedGuard lock(mutex_cur_);
	SendCurQueue(cur_queue_);
}

void ObjMsgQueueList::DispatchSpec()
{
	MutexLockTimedGuard lock(mutex_cur_);
	SendCurQueueSpec(cur_queue_, 1);
}


///
/// WorldMsgQueueList
///
WorldMsgQueueList::WorldMsgQueueList(MsgQueue<WORLDMSG>::DispatchCB cb)
	: cur_queue_(cb)
{
}

WorldMsgQueueList::~WorldMsgQueueList()
{
}

void WorldMsgQueueList::AddMsg(MapID a_world_id, const MSG& msg, MapTag a_world_tag)
{
	MutexLockTimedGuard lock(mutex_cur_);
	WORLDMSG dupe_msg;
	dupe_msg.world_id  = a_world_id;
	dupe_msg.world_tag = a_world_tag;
	CopyMessage(dupe_msg.msg, msg);
	cur_queue_.AddMsg(dupe_msg);
}

void WorldMsgQueueList::Dispatch()
{
	MutexLockTimedGuard lock(mutex_cur_);
	SendCurQueue(cur_queue_);
}

void WorldMsgQueueList::DispatchSpec()
{
	MutexLockTimedGuard lock(mutex_cur_);
	SendCurQueueSpec(cur_queue_, 0);
}

} // namespace gamed
