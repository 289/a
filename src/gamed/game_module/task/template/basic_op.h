#ifndef TASK_BASIC_OP_H_
#define TASK_BASIC_OP_H_

#include "basic_info.h"
#include "item_info.h"
#include "monster_info.h"
#include "zone_info.h"

namespace task
{

class MapElementCtrl
{
public:
	inline bool CheckDataValidity() const;

	MapElementVec element_list;

	NESTED_DEFINE(element_list);
};

enum TransType
{
	TRANS_NORMAL,
	TRANS_RANDOM,
};

class Trans
{
public:
	Trans()
		: trans_type(TRANS_NORMAL)
	{
	}

	inline bool CheckDataValidity() const;

	int8_t trans_type;
	PositionVec pos_list;

	NESTED_DEFINE(trans_type, pos_list);
};

class DeliverItem
{
public:
	DeliverItem()
		: rand_deliver(false)
	{
	}

	inline bool CheckDataValidity() const;

	bool rand_deliver;
	ItemDeliveredVec item_list;

	NESTED_DEFINE(rand_deliver, item_list);
};

class RecycleItem
{
public:
	inline bool CheckDataValidity() const;

	ItemInfoVec item_list;

	NESTED_DEFINE(item_list);
};

// 召唤操作不允许在奖励中出现，只能放在开启操作中
// 原因：
// 在开启操作中，没完成可以重新生成，
// 放在奖励中，如果奖励发放后下线则由于任务已经完成则无法重新发放
class SummonNPC
{
public:
	inline bool CheckDataValidity() const;

	NPCVec npc_list;

	NESTED_DEFINE(npc_list);
};

class SummonMonster
{
public:
	inline bool CheckDataValidity() const;

	MonsterVec monster_list;

	NESTED_DEFINE(monster_list);
};

class SummonMine
{
public:
	inline bool CheckDataValidity() const;

	MineVec mine_list;

	NESTED_DEFINE(mine_list);
};

// 修改NPC黑白名单
class ModifyNPCBWList
{
public:
	inline bool CheckDataValidity() const;

	NPCBWList bw_list;

	NESTED_DEFINE(bw_list);
};

class ModifyCounter
{
public:
	inline bool CheckDataValidity() const;

	CounterVec counter_list;

	NESTED_DEFINE(counter_list);
};

class SendSysMail
{
public:
	SendSysMail()
		: attach_score(0)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t attach_score;
	std::string sender;
	std::string title;
	std::string content;
	ItemDeliveredVec item_list;

	NESTED_DEFINE(attach_score, sender, title, content, item_list);
};

class SendSysChat
{
public:
	inline bool CheckDataValidity() const;

	ChatVec chat_list;

	NESTED_DEFINE(chat_list);
};

class ModifyReputation
{
public:
    inline bool CheckDataValidity() const;

    ReputationVec reputation_list;

    NESTED_DEFINE(reputation_list);
};

class ModifyUI
{
public:
    inline bool CheckDataValidity() const;

    UIOpVec ui_list;

    NESTED_DEFINE(ui_list);
};

class ModifyBuff
{
public:
    inline bool CheckDataValidity() const;

    BuffOpVec buff_list;

    NESTED_DEFINE(buff_list);
};

class ModifyTalent
{
public:
    inline bool CheckDataValidity() const;

    std::vector<int32_t> talent_list;

    NESTED_DEFINE(talent_list);
};

class ModifyTitle
{
public:
    inline bool CheckDataValidity() const;

    std::vector<int32_t> title_list;

    NESTED_DEFINE(title_list);
};

class ShowDlgTaskInfo
{
public:
    inline bool CheckDataValidity() const;

    DlgTaskInfoVec info_list;

    NESTED_DEFINE(info_list);
};

class ItemUseHint
{
public:
    inline bool CheckDataValidity() const;

    ItemUseZoneVec zone_list;

    NESTED_DEFINE(zone_list);
};

class ModifyFriend
{
public:
    inline bool CheckDataValidity() const;

    FriendOpVec friend_list;
    NESTED_DEFINE(friend_list);
};

class CameraMask
{
public:
    CameraMask() : op(0)
    {
    }

    inline bool CheckDataValidity() const;

    int8_t op;
    std::string gfx_path;
    NESTED_DEFINE(op, gfx_path);
};

#include "basic_op-inl.h"

} // namespace task

#endif // TASK_BASIC_OP_H_
