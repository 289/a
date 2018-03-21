#ifndef GAMED_GS_OBJ_NPC_INL_H_
#define GAMED_GS_OBJ_NPC_INL_H_

///
/// inline func
///
inline mapDataSvr::TemplID Npc::templ_id() const
{
	return templ_id_;
}

inline mapDataSvr::ElemID Npc::elem_id() const
{
	return elem_id_;
}

inline NpcSender* Npc::sender()
{
	return sender_;
}

inline NpcAI* Npc::aicore()
{
	return ai_core_;
}

inline void Npc::set_state(npcdef::NpcStates state)
{
	state_ = state;
}

inline bool Npc::is_idle()
{
	return CheckIdleState();
}

inline npcdef::NpcStates Npc::state() const
{
	return state_;
}

#endif // GAMED_GS_OBJ_NPC_INL_H_
