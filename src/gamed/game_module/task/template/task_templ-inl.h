#ifndef TASK_TASK_TEMPL_INL_H_
#define TASK_TASK_TEMPL_INL_H_

// 内联函数
inline bool TaskTempl::can_giveup() const
{
	return (flag & FLAG_GIVEUP) != 0;
}

inline bool TaskTempl::can_seek() const
{
	return (flag & FLAG_SEEK) != 0;
}

inline bool TaskTempl::show_deliver_tips() const
{
	return (flag & FLAG_DELIVER_TIPS) != 0;
}

inline bool TaskTempl::show_complete_tips() const
{
	return (flag & FLAG_COMPLETE_TIPS) != 0;
}

inline bool TaskTempl::hide_task() const
{
	return (flag & FLAG_HIDE) != 0;
}

inline bool TaskTempl::auto_deliver() const
{
	return (flag & FLAG_AUTO) != 0;
}

inline bool TaskTempl::can_redo_after_succ() const
{
	return (flag & FLAG_REDO_SUCC) != 0;
}

inline bool TaskTempl::can_redo_after_fail() const
{
	return (flag & FLAG_REDO_FAIL) != 0;
}

inline bool TaskTempl::record_result() const
{
	return (flag & FLAG_RECORD) != 0;
}

inline bool TaskTempl::limit_count() const
{
    return limit.limit != LIMIT_NONE;
}

inline bool TaskTempl::share_task() const
{
	return (flag & FLAG_SHARE) != 0;
}

inline bool TaskTempl::fail_trigger_parent_fail() const
{
	return (flag & FLAG_FAIL_PARENT_FAIL) != 0;
}

inline bool TaskTempl::succ_trigger_parent_succ() const
{
	return (flag & FLAG_SUCC_PARENT_SUCC) != 0;
}

inline bool TaskTempl::ui_task() const
{
	return (flag & FLAG_UI) != 0;
}

inline bool TaskTempl::cat_vision() const
{
	return (flag & FLAG_CAT_VISION) != 0;
}

inline bool TaskTempl::cross_trans() const
{
	return (flag & FLAG_CROSS_TRANS) != 0;
}

inline bool TaskTempl::close_npc_area() const
{
	return (flag & FLAG_CLOSE_NPC_AREA) != 0;
}

inline bool TaskTempl::prior_track() const
{
	return (flag & FLAG_PRIOR_TRACK) != 0;
}

inline bool TaskTempl::mag_task() const
{
	return (flag & FLAG_MAG_TASK) != 0;
}

#endif // TASK_TASK_TEMPL_INL_H_
