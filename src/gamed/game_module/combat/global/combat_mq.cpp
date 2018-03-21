#include "combat_mq.h"
#include "object.h"
#include "combat_def.h"
#include "combat_man.h"

namespace combat
{

/*******************************MsgQueue******************************/
/*******************************MsgQueue******************************/
/*******************************MsgQueue******************************/
/*******************************MsgQueue******************************/

/**************************MsgQueue::MultiCast************************/
MsgQueue::MultiCast::MultiCast(const ObjectVec& l, const MSG& message)
	: list(l)
{
	DupeMessage(msg, message);
}

MsgQueue::MultiCast::~MultiCast()
{
	FreeMessage(msg);
}

void MsgQueue::MultiCast::MultiCast::Send()
{
	for (size_t i = 0; i < list.size(); ++ i)
	{
		Object* obj = list[i];
		if (!obj->IsActived())
			continue;

		s_pCombatMan->DispatchMSG(obj, msg);
	}
}

/**************************MsgQueue::IDMultiCast**********************/
MsgQueue::IDMultiCast::IDMultiCast(const XIDVec& l, const MSG& message)
	: list(l)
{
	DupeMessage(msg, message);
}

MsgQueue::IDMultiCast::IDMultiCast(const XID* first, const XID* last, const MSG& message)
{
	assert(last - first > 0);
	list.reserve(last - first);
	while (first != last)
	{
		list.push_back(*first);
		++ first;
	}
	DupeMessage(msg, message);
}

MsgQueue::IDMultiCast::~IDMultiCast()
{
	FreeMessage(msg);
}
 
void MsgQueue::IDMultiCast::IDMultiCast::Send()
{
	XIDVec::iterator it = list.begin();
	for (; it != list.end(); ++ it)
	{
		msg.target = *it;
		s_pCombatMan->DispatchMSG(msg);
	}
}

/*******************************MsgQueue******************************/
bool MsgQueue::IsEmpty() const
{
	return queue_.empty() && multi_queue_.empty() && id_multi_queue_.empty();
}

void MsgQueue::AddMsg(const MSG& message)
{
	MSG msg;
	DupeMessage(msg, message);
	queue_.push_back(msg);
}

void MsgQueue::AddMultiMsg(const ObjectVec&list, const MSG& msg)
{
	multi_queue_.push_back(MultiCast(list, msg));
}

void MsgQueue::AddMultiMsg(const XID* first, const XID* last, const MSG& msg)
{
	id_multi_queue_.push_back(IDMultiCast(first, last, msg));
}

void MsgQueue::Send()
{
	for (MSGQUEUE::iterator it = queue_.begin(); it != queue_.end(); ++ it)
	{
		s_pCombatMan->DispatchMSG(*it);
	}

	for (IDMULTICASTQUEUE::iterator it = id_multi_queue_.begin(); it != id_multi_queue_.end(); ++ it)
	{
		it->Send();
	}

	Clear();
}

void MsgQueue::Swap(MsgQueue& queue)
{
	queue_.swap(queue.queue_);
	multi_queue_.swap(queue.multi_queue_);
	id_multi_queue_.swap(queue.id_multi_queue_);
}

void MsgQueue::Clear()
{
	MSGQUEUE::iterator it = queue_.begin();
	for (; it != queue_.end(); ++ it)
	{
		FreeMessage(*it);
	}

	queue_.clear();
	multi_queue_.clear();
	id_multi_queue_.clear();
}

/*******************************MsgQueueList******************************/
/*******************************MsgQueueList******************************/
/*******************************MsgQueueList******************************/
/*******************************MsgQueueList******************************/
MsgQueueList::MsgQueueList():
	offset_(0),
	cur_queue_size_(0),
	tick_counter_(0)
{
	for(size_t i = 0; i < SIZE; ++ i)
	{
		list_[i] = NULL;
	}
}

MsgQueueList::~MsgQueueList()
{
	for(size_t i = 0; i < SIZE; ++ i)
	{
		if (list_[i])
		{
			delete list_[i];
			list_[i] = NULL;
		}
	}
}

MsgQueue* MsgQueueList::GetQueue(int target)
{
	if (list_[target] == NULL)
	{
		list_[target] = new MsgQueue();
	}
	return list_[target];
}

void MsgQueueList::AddMsg(const MSG& msg, size_t tick_latency)
{
	if (tick_latency == 0)
	{
        MsgQueue __queue;
        {
            shared::MutexLockGuard keeper(cur_lock_);
            cur_queue_.AddMsg(msg);

		    if (++ cur_queue_size_ > MAX_QUEUE_SIZE)
            {
                cur_queue_size_ = 0;
                __queue.Swap(cur_queue_);
            }
        }

		SEND_CUR_QUEUE(__queue);
	}
	else
	{
		assert(tick_latency < MAX_MSG_LATENCY);

		shared::MutexLockGuard keeper(list_lock_);
		int target = offset_ + tick_latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMsg(msg);
	}
}

void MsgQueueList::AddMultiMsg(const ObjectVec& list, const MSG& msg, size_t tick_latency)
{
	if (tick_latency == 0)
	{
        MsgQueue __queue;
        {
            shared::MutexLockGuard keeper(cur_lock_);
		    cur_queue_.AddMultiMsg(list, msg);

		    if (++ cur_queue_size_ > MAX_QUEUE_SIZE)
            {
                cur_queue_size_ = 0;
                __queue.Swap(cur_queue_);
            }
        }

		SEND_CUR_QUEUE(__queue);
	}
	else
	{
		assert(tick_latency < MAX_MSG_LATENCY);

		shared::MutexLockGuard keeper(list_lock_);
		int target = offset_ + tick_latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMultiMsg(list, msg);
	}
}

void MsgQueueList::AddMultiMsg(const XID* first, const XID* last, const MSG& msg, size_t tick_latency)
{
	if (tick_latency == 0)
	{
        MsgQueue __queue;
        {
            shared::MutexLockGuard keeper(cur_lock_);
		    cur_queue_.AddMultiMsg(first, last, msg);

		    if (++ cur_queue_size_ > MAX_QUEUE_SIZE)
            {
                cur_queue_size_ = 0;
                __queue.Swap(cur_queue_);
            }
        }

		SEND_CUR_QUEUE(__queue);
	}
	else
	{
		assert(tick_latency < MAX_MSG_LATENCY);

		shared::MutexLockGuard keeper(list_lock_);
		int target = offset_ + tick_latency;
		if(target >= SIZE) target %= SIZE;
		GetQueue(target)->AddMultiMsg(first, last, msg);
	}
}

void MsgQueueList::HeartBeat()
{
	//每100ms执行一次
	if (cur_queue_size_ > 0)
	{
	    MsgQueue __queue;
        {
		    shared::MutexLockGuard keeper(cur_lock_);
            __queue.Swap(cur_queue_);
            cur_queue_size_ = 0;
        }

        SEND_CUR_QUEUE(__queue);
	}

	{
		shared::MutexLockGuard keeper(list_lock_);
		MsgQueue* __queue = list_[offset_];
		if (__queue)
		{
			__queue->Send();
		}

		++ offset_;
		assert(offset_ <= MAX_MSG_LATENCY);
		if (offset_ >= MAX_MSG_LATENCY)
		{
			offset_ = 0;
		}
	}
}

};
