#ifndef GAMED_GS_OBJ_MATTER_INL_H_
#define GAMED_GS_OBJ_MATTER_INL_H_

///
/// inline func
///
inline mapDataSvr::TemplID Matter::templ_id() const
{
	return templ_id_;
}
	
inline mapDataSvr::ElemID Matter::elem_id() const
{
	return elem_id_;
}

inline bool Matter::is_idle()
{
	return CheckIdleState();
}

inline void Matter::set_state(matterdef::MatterStates state)
{
	state_ = state;
}

inline matterdef::MatterStates Matter::state() const
{
	return state_;
}

#endif // GAMED_GS_OBJ_MATTER_INL_H_
