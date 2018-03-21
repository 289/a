#ifndef TASK_BASIC_OP_INL_H_
#define TASK_BASIC_OP_INL_H_

inline bool MapElementCtrl::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(element_list)
	return true;
}

inline bool Trans::CheckDataValidity() const
{
	CHECK_INRANGE(trans_type, TRANS_NORMAL, TRANS_RANDOM)

	// 普通传送时，只允许填一个传送点
	// 如果存在多个，每个传送直接间隔多久执行，这个没有定义
	// 如果的确需要在多个点间传送，可以使用多个子任务完成
	if (trans_type == TRANS_NORMAL && pos_list.size() > 1)
	{
		return false;
	}

	CHECK_VEC_VALIDITY(pos_list)
	return true;
}

inline bool DeliverItem::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(item_list)
	return true;
}

inline bool RecycleItem::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(item_list)
	return true;
}

inline bool SummonNPC::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(npc_list)
	return true;
}

inline bool SummonMonster::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(monster_list)
	return true;
}

inline bool SummonMine::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(mine_list)
	return true;
}

inline bool ModifyNPCBWList::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(bw_list)
	return true;
}

inline bool ModifyCounter::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(counter_list)
	return true;
}

inline bool SendSysMail::CheckDataValidity() const
{
	if (attach_score < 0)
	{
		return false;
	}
	CHECK_VEC_VALIDITY(item_list)
	return true;
}

inline bool SendSysChat::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(chat_list)
	return true;
}

inline bool ModifyReputation::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(reputation_list);
    return true;
}

inline bool ModifyUI::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(ui_list);
    return true;
}

inline bool ModifyBuff::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(buff_list);
    return true;
}

inline bool ModifyTalent::CheckDataValidity() const
{
    for (size_t i = 0; i < talent_list.size(); ++i)
    {
        if (talent_list[i] <= 0)
        {
            return false;
        }
    }
    return true;
}

inline bool ModifyTitle::CheckDataValidity() const
{
    for (size_t i = 0; i < title_list.size(); ++i)
    {
        if (title_list[i] <= 0)
        {
            return false;
        }
    }
    return true;
}

inline bool ShowDlgTaskInfo::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(info_list)
    return true;
}

inline bool ItemUseHint::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(zone_list)
    return true;
}

inline bool ModifyFriend::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(friend_list)
    return true;
}

inline bool CameraMask::CheckDataValidity() const
{
    return op >= CAMERA_MASK_NONE && op <= CAMERA_MASK_CLOSE;
}

#endif // TASK_BASIC_OP_INL_H_
