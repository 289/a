#ifndef __GAME_MODULE_COMBAT_MQ_H__
#define __GAME_MODULE_COMBAT_MQ_H__

#include <vector>
#include "combat_msg.h"
#include "combat_types.h"
#include "shared/base/mutex.h"

namespace combat
{

#define MAX_MSG_LATENCY 600 // 消息队列支持的消息最大延迟为60s
#define MAX_QUEUE_SIZE  256 // 消息队列大小大于等于该值时，则提前执行发送消息逻辑

class MSG;
class Object;

/**
 * @class MsqQueue
 * @brief 消息队列，支持单播和多播
 * @brief 同一战场无需通过消息通信，战场中间通过消息传递信息
 */
class MsgQueue
{
	struct MultiCast
	{
		ObjectVec list;
		MSG msg;

		MultiCast(const ObjectVec& list, const MSG& msg);
		~MultiCast();

		void Send();
	};

	struct IDMultiCast
	{
		XIDVec list;
		MSG msg;

		IDMultiCast(const XIDVec& l, const MSG& msg);
		IDMultiCast(const XID* first, const XID* last, const MSG& msg);
		~IDMultiCast();

		void Send();
	};

	typedef std::vector<MSG> MSGQUEUE;
	typedef std::vector<MultiCast> MULTICASTQUEUE;
	typedef std::vector<IDMultiCast> IDMULTICASTQUEUE;

	MSGQUEUE queue_;
	MULTICASTQUEUE multi_queue_;
	IDMULTICASTQUEUE id_multi_queue_;

public:
	MsgQueue() {}
	~MsgQueue()
	{
		Clear();
	}

	bool IsEmpty() const;
	void AddMsg(const MSG& msg);
	void AddMultiMsg(const ObjectVec& list, const MSG& msg);
	void AddMultiMsg(const XID* first, const XID* last, const MSG& msg);
	void Send();
	void Swap(MsgQueue& queue);
	void Clear();
};


/**
 * @class MsgQueueList
 * @brief 扩展消息队列，支持单播/多播/延迟消息
 */
class MsgQueueList
{
	enum
	{	
		SIZE = MAX_MSG_LATENCY
	};

	MsgQueue  cur_queue_;
	MsgQueue* list_[SIZE];
	size_t offset_;
	size_t cur_queue_size_;
	size_t tick_counter_;

	shared::MutexLock cur_lock_;
	shared::MutexLock list_lock_;


public:	
	MsgQueueList();
	~MsgQueueList();

	MsgQueue* GetQueue(int target);
	void AddMsg(const MSG& msg, size_t tick_latency);
	void AddMultiMsg(const ObjectVec& list, const MSG& msg, size_t tick_latency);
	void AddMultiMsg(const XID* first, const XID* last, const MSG& msg, size_t tick_latency);
	void HeartBeat();

private:
	void SEND_CUR_QUEUE(MsgQueue& queue)
	{
        if (!queue.IsEmpty())
        {
            queue.Send();
        }
	}
};

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_MQ_H__
