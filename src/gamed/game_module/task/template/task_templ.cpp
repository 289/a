#include "task_templ.h"

namespace task
{

INIT_STATIC_SYS_TEMPL(TaskTempl, TEMPL_TYPE_TASK);

void TaskTempl::OnMarshal()
{
	MARSHAL_SYS_TEMPL_VALUE(name, type, flag, finish_mode, track_err_msg);
	MARSHAL_SYS_TEMPL_VALUE(tag1, tag2, quality, subtask_deliver_mode);
	MARSHAL_SYS_TEMPL_VALUE(deliver_prob);
	MARSHAL_SYS_TEMPL_VALUE(deliver_npc, finish_npc);
	MARSHAL_SYS_TEMPL_VALUE(deliver_talk, unqualified_talk, unfinish_talk, finish_talk);
	MARSHAL_SYS_TEMPL_VALUE(desc, can_deliver_desc, hint_desc, hint_award_desc, succ_cond_desc, succ_diary, fail_diary);
    MARSHAL_SYS_TEMPL_VALUE(track);
    MARSHAL_SYS_TEMPL_VALUE(item_hint);
    MARSHAL_SYS_TEMPL_VALUE(limit);
	MARSHAL_SYS_TEMPL_VALUE(path_finder);
	MARSHAL_SYS_TEMPL_VALUE(premise);
	MARSHAL_SYS_TEMPL_VALUE(op);
	MARSHAL_SYS_TEMPL_VALUE(succ_cond, fail_cond);
	MARSHAL_SYS_TEMPL_VALUE(succ_award, fail_award);
	MARSHAL_SYS_TEMPL_VALUE(parent, next_sibling, first_child);
}

void TaskTempl::OnUnmarshal()
{
	UNMARSHAL_SYS_TEMPL_VALUE(name, type, flag, finish_mode, track_err_msg);
	UNMARSHAL_SYS_TEMPL_VALUE(tag1, tag2, quality, subtask_deliver_mode);
	UNMARSHAL_SYS_TEMPL_VALUE(deliver_prob);
	UNMARSHAL_SYS_TEMPL_VALUE(deliver_npc, finish_npc);
	UNMARSHAL_SYS_TEMPL_VALUE(deliver_talk, unqualified_talk, unfinish_talk, finish_talk);
	UNMARSHAL_SYS_TEMPL_VALUE(desc, can_deliver_desc, hint_desc, hint_award_desc, succ_cond_desc, succ_diary, fail_diary);
    UNMARSHAL_SYS_TEMPL_VALUE(track);
    UNMARSHAL_SYS_TEMPL_VALUE(item_hint);
    UNMARSHAL_SYS_TEMPL_VALUE(limit);
	UNMARSHAL_SYS_TEMPL_VALUE(path_finder);
	UNMARSHAL_SYS_TEMPL_VALUE(premise);
	UNMARSHAL_SYS_TEMPL_VALUE(op);
	UNMARSHAL_SYS_TEMPL_VALUE(succ_cond, fail_cond);
	UNMARSHAL_SYS_TEMPL_VALUE(succ_award, fail_award);
	UNMARSHAL_SYS_TEMPL_VALUE(parent, next_sibling, first_child);
}

// 检测数据有效性
bool TaskTempl::OnCheckDataValidity() const
{
	CHECK_INRANGE(finish_mode, FINISH_DIRECT, FINISH_NPC)
	CHECK_INRANGE(tag1, TAG_NONE1, TAG_MONEY)
	CHECK_INRANGE(tag2, TAG_NONE2, TAG_GOAL)
	CHECK_INRANGE(quality, QUALITY_WHITE, QUALITY_MAIN)
	CHECK_INRANGE(subtask_deliver_mode, DELIVER_MANUAL, DELIVER_ALL)
	CHECK_VEC_VALIDITY(deliver_talk)
	CHECK_VEC_VALIDITY(unfinish_talk)
	CHECK_VEC_VALIDITY(unfinish_talk)
	CHECK_VEC_VALIDITY(finish_talk)
	CHECK_VALIDITY(track)
	CHECK_VALIDITY(item_hint)
	CHECK_VALIDITY(limit)
	CHECK_VALIDITY(path_finder)
	CHECK_VALIDITY(premise)
	CHECK_VALIDITY(op)
	CHECK_VALIDITY(succ_cond)
	CHECK_VALIDITY(fail_cond)
	CHECK_VALIDITY(succ_award)
	CHECK_VALIDITY(fail_award)
	if (deliver_prob < 0 || parent < 0 || next_sibling < 0 || first_child < 0)
	{
		return false;
	}
	return true;
}

} // namespace task
