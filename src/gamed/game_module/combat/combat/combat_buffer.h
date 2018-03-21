#ifndef __GAME_MODULE_COMBAT_BUFF_H__
#define __GAME_MODULE_COMBAT_BUFF_H__

#include <vector>
#include <stdint.h>
#include <assert.h>

namespace combat
{

/**
 * @class CombatBuffer
 * @brief 缓存一个回合中BUFF的变化情况 :
 *        buff_list_add_表示本回合新增了哪些BUFF
 *        buff_list_del_表示本回合删除了哪些BUFF
 */

class CombatBuffer
{
	struct Buff
	{
		uint32_t buff_seq;
		int32_t buff_id;
		int32_t attacher;
		int32_t target;
	};

	std::vector<Buff> buff_list_add_; // 回合中新增的buff列表
	std::vector<Buff> buff_list_del_; // 回合中删除的buff列表

public:
	typedef std::vector<Buff> BuffVec;

public:
	CombatBuffer() {}
	~CombatBuffer()
	{
		buff_list_add_.clear();
		buff_list_del_.clear();
	}

	void AttachBuffer(uint32_t buff_seq, int32_t buff_id, int32_t attacher, int32_t target)
	{
		assert(buff_seq > 0 && buff_id > 0 && attacher > 0 && target > 0);

		Buff buff;
		buff.buff_seq = buff_seq;
		buff.buff_id  = buff_id;
		buff.attacher = attacher;
		buff.target   = target;
		buff_list_add_.push_back(buff);
	}

	void DetachBuffer(uint32_t buff_seq, int32_t buff_id, int32_t attacher, int32_t target)
	{
		assert(buff_seq > 0 && buff_id > 0 && attacher > 0 && target > 0);

		Buff buff;
		buff.buff_seq = buff_seq;
		buff.buff_id  = buff_id;
		buff.attacher = attacher;
		buff.target   = target;
		buff_list_del_.push_back(buff);
	}

	const BuffVec& GetBuffersAdd() const
	{
		return buff_list_add_;
	}

	const BuffVec& GetBuffersDel() const
	{
		return buff_list_del_;
	}

	bool IsEmpty() const
	{
		return buff_list_add_.empty() && buff_list_del_.empty();
	}

	void Clear()
	{
		buff_list_add_.clear();
		buff_list_del_.clear();
	}
};

};

#endif // __GAME_MODULE_COMBAT_COMBAT_BUFF_H__
