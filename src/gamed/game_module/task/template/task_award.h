#ifndef TASK_TASK_AWARD_H_
#define TASK_TASK_AWARD_H_

#include "basic_op.h"

namespace task
{

class TaskAward
{
public:
	TaskAward()
		: skill_point(0), transfer_cls(CLS_NONE), deliver_task(0), succ_task(0), fail_task(0), 
        succ_ins(0), fail_ins(0), open_star(0), gevent_gid(0), mount_equip_index(0), battle_ground_id(0)
	{
	}

	inline bool CheckDataValidity() const;

	Gold gold;
	Exp exp;
	Score score;
	int32_t skill_point;
	int8_t transfer_cls;
	TaskID deliver_task;
	TaskID succ_task;
	TaskID fail_task;
	DeliverItem deliver_item;
	RecycleItem recycle_item;
	ModifyNPCBWList modify_bw;
	ModifyCounter counter;
	SendSysMail mail;
	int32_t succ_ins;
	int32_t fail_ins;
	SendSysChat chat;
	MapElementCtrl ctrl_element;
	Trans script_trans;
    TaskInfoVec dec_finish_time;
    ModifyReputation modify_reputation;
    ModifyUI modify_ui;
    ModifyBuff modify_buff;
    ModifyTalent modify_talent;
    ModifyTitle modify_title;
    ModifyFriend modify_friend;
    int32_t open_star;
    int32_t gevent_gid;
    int8_t mount_equip_index;
    int32_t battle_ground_id;

	void Pack(shared::net::ByteBuffer& buf)
	{
		buf << gold << exp << score;
		buf << skill_point << transfer_cls;
		buf << deliver_task << succ_task << fail_task << dec_finish_time;
		buf << deliver_item << recycle_item;
		buf << modify_bw << counter << mail;
		buf << succ_ins << fail_ins;
		buf << chat << ctrl_element;
		buf << script_trans;
        buf << modify_reputation;
        buf << modify_ui;
        buf << modify_buff;
        buf << modify_talent;
        buf << modify_title;
        buf << modify_friend;
        buf << open_star;
        buf << gevent_gid;
        buf << mount_equip_index;
        buf << battle_ground_id;
	}

	void UnPack(shared::net::ByteBuffer& buf)
	{
		buf >> gold >> exp >> score;
		buf >> skill_point >> transfer_cls;
		buf >> deliver_task >> succ_task >> fail_task >> dec_finish_time;
		buf >> deliver_item >> recycle_item;
		buf >> modify_bw >> counter >> mail;
		buf >> succ_ins >> fail_ins;
		buf >> chat >> ctrl_element;
		buf >> script_trans;
        buf >> modify_reputation;
        buf >> modify_ui;
        buf >> modify_buff;
        buf >> modify_talent;
        buf >> modify_title;
        buf >> modify_friend;
        buf >> open_star;
        buf >> gevent_gid;
        buf >> mount_equip_index;
        buf >> battle_ground_id;
	}
};

// 内联函数
inline bool TaskAward::CheckDataValidity() const
{
	if (skill_point < 0)
	{
		return false;
	}
	if (deliver_task < 0 || succ_task < 0 || fail_task < 0)
	{
		return false;
	}
    if (succ_ins < 0 || fail_ins < 0 || open_star < 0 || gevent_gid < 0 || battle_ground_id < 0)
    {
        return false;
    }
    if (mount_equip_index < -1 || mount_equip_index > 4)
    {
        return false;
    }
	CHECK_VALIDITY(gold)
	CHECK_VALIDITY(exp)
	CHECK_VALIDITY(score)
	CHECK_INRANGE(transfer_cls, CLS_NONE, CLS_S_RAKE_ARCHER)
	CHECK_VALIDITY(deliver_item)
	CHECK_VALIDITY(recycle_item)
	CHECK_VALIDITY(modify_bw)
	CHECK_VALIDITY(counter)
	CHECK_VALIDITY(mail)
	CHECK_VALIDITY(chat)
	CHECK_VALIDITY(ctrl_element)
	CHECK_VEC_VALIDITY(dec_finish_time)
    CHECK_VALIDITY(modify_reputation)
    CHECK_VALIDITY(modify_ui)
    CHECK_VALIDITY(modify_buff)
    CHECK_VALIDITY(modify_talent)
    CHECK_VALIDITY(modify_title)
    CHECK_VALIDITY(modify_friend)
	return true;
}

} // namespace task

#endif // TASK_TASK_AWARD_H_
