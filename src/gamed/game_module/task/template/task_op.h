#ifndef TASK_TASK_OP_H_
#define TASK_TASK_OP_H_

#include "basic_op.h"

namespace task
{

enum ExclusiveOp
{
	EXOP_NONE,
	EXOP_SCRIPT,
	EXOP_GUIDE,
	EXOP_TRANS,
	EXOP_INS,
    EXOP_BG,
};

class TaskOp
{
public:
	TaskOp()
		: exclusive_op(EXOP_NONE), ins_id(0), bg_id(0), scale(SCALE_NORMAL)
	{
	}

	inline bool CheckDataValidity() const;

	// 独占操作
	int8_t exclusive_op;
	Trans trans;
	int32_t ins_id;
    int32_t bg_id;
	// 非独占，以下操作可以同时进行
	int8_t scale;
	MapElementCtrl ctrl_element;
	DeliverItem deliver_item;
	RecycleItem recycle_item;
	SummonNPC npc;
	SummonMonster monster;
	SummonMine mine;
	ModifyCounter counter;
	SendSysChat chat;
    ModifyBuff buff;
    ShowDlgTaskInfo info;
    ModifyFriend modify_friend;
    CameraMask camera_mask;

    void Pack(shared::net::ByteBuffer& buf)
    {
        buf << exclusive_op     << trans        << ins_id           << bg_id;
        buf << scale            << ctrl_element << deliver_item     << recycle_item; 
        buf << npc              << monster      << mine             << counter;    
        buf << chat             << buff         << info             << modify_friend;
        buf << camera_mask;
    }

    void UnPack(shared::net::ByteBuffer& buf)
    {
        buf >> exclusive_op     >> trans        >> ins_id           >> bg_id;
        buf >> scale            >> ctrl_element >> deliver_item     >> recycle_item;  
        buf >> npc              >> monster      >> mine             >> counter;    
        buf >> chat             >> buff         >> info             >> modify_friend;
        buf >> camera_mask;
    }
};

// 内联函数
inline bool TaskOp::CheckDataValidity() const
{
	CHECK_INRANGE(exclusive_op, EXOP_NONE, EXOP_BG)
	CHECK_INRANGE((scale & 0x0F), SCALE_NORMAL, SCALE_BIG)
	CHECK_VALIDITY(ctrl_element)
	CHECK_VALIDITY(deliver_item)
	CHECK_VALIDITY(recycle_item)
	CHECK_VALIDITY(npc)
	CHECK_VALIDITY(monster)
	CHECK_VALIDITY(mine)
	CHECK_VALIDITY(counter)
	CHECK_VALIDITY(trans)
	CHECK_VALIDITY(chat)
	CHECK_VALIDITY(buff)
	CHECK_VALIDITY(info)
	CHECK_VALIDITY(modify_friend)
    CHECK_VALIDITY(camera_mask)
	return true;
}

} // namespace task

#endif // TASK_TASK_OP_H_
