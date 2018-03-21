#ifndef GAMED_GS_GLOBAL_INTER_MSG_QUEUE_H_
#define GAMED_GS_GLOBAL_INTER_MSG_QUEUE_H_

#include <vector>

#include "shared/base/mutex.h"
#include "shared/base/base_define.h"

#include "runnable_pool.h"
#include "game_types.h"


namespace gamed {

struct MSG;
struct WORLDMSG;

///
/// MsgQueue is thread unsafe
///
template <typename T>
class MsgQueue : public RunnablePool::Task
{
	typedef std::vector<T> MSGQUEUE;
public:
	typedef void (*DispatchCB)(const T& msg);

	MsgQueue(DispatchCB cb = NULL);
	~MsgQueue();

	// **** thread unsafe ****
	void    AddMsg(const T& msg);
	inline void Swap(MsgQueue<T>& queue);
	inline bool IsEmpty() const;

	virtual void Run();


protected:
	void    Dispatch();
	void    Clear();


private:
	MSGQUEUE     msg_queue_;
	DispatchCB   dispatch_cb_;
};

///
/// template function
///
template <typename T>
void MsgQueue<T>::AddMsg(const T& msg)
{
	msg_queue_.push_back(msg);
}

template <typename T>
MsgQueue<T>::MsgQueue(DispatchCB cb)
	: dispatch_cb_(cb)
{
}

template <typename T>
MsgQueue<T>::~MsgQueue()
{
	Clear();
}

template <typename T>
void MsgQueue<T>::Clear()
{
	typename MSGQUEUE::iterator it  = msg_queue_.begin();
	typename MSGQUEUE::iterator end = msg_queue_.end();
	for (; it != end; ++it)
	{
		FreeMessage(&(*it));
	}

	msg_queue_.clear();
}

template <typename T>
void MsgQueue<T>::Dispatch()
{
	typename MSGQUEUE::iterator it  = msg_queue_.begin();
	typename MSGQUEUE::iterator end = msg_queue_.end();
	for (; it != end; ++it)
	{
		dispatch_cb_(*it);
	}

	Clear();
}

template <typename T>
void MsgQueue<T>::Run()
{
	Dispatch();
}

template <typename T>
inline void MsgQueue<T>::Swap(MsgQueue<T>& queue)
{
	msg_queue_.swap(queue.msg_queue_);
	dispatch_cb_ = queue.dispatch_cb_;
}

template <typename T>
inline bool MsgQueue<T>::IsEmpty() const
{
	return msg_queue_.empty();
}


///
/// ObjMsgQueueList
///
class ObjMsgQueueList
{
public:
	ObjMsgQueueList(MsgQueue<MSG>::DispatchCB cb);
	~ObjMsgQueueList();

	// ---- thread safe ----
	void    AddMsg(const MSG& msg);
	void    Dispatch();
	void    DispatchSpec();

private:
	MsgQueue<MSG>        cur_queue_;
	shared::MutexLock    mutex_cur_;
};


///
/// WorldMsgQueueList
///
class WorldMsgQueueList
{
public:
	WorldMsgQueueList(MsgQueue<WORLDMSG>::DispatchCB cb);
	~WorldMsgQueueList();

	// ---- thread safe ----
	void    AddMsg(MapID world_id, const MSG& msg, MapTag world_tag=0);
	void    Dispatch();
	void    DispatchSpec();

private:
	MsgQueue<WORLDMSG>   cur_queue_;
	shared::MutexLock    mutex_cur_;
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_INTER_MSG_QUEUE_H_
