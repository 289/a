#ifndef GAMED_GS_OBJ_NPC_DEF_H_
#define GAMED_GS_OBJ_NPC_DEF_H_

#include "gs/template/map_data/map_types.h"


namespace gamed {
namespace npcdef {

	struct NpcInitData
	{
		NpcInitData() : elem_id(0), templ_id(0), dir(0) { }

		XID::IDType  id;
		mapDataSvr::ElemID  elem_id;
		mapDataSvr::TemplID templ_id;
		uint8_t      dir;
		A2DVECTOR    pos;
	};

    // 需要配合npc_state.cpp里的状态检查
	enum NpcStates
	{
		STATE_IDLE = 0,  // 视野内没有玩家，怪物出去休息状态，心跳变慢
		STATE_NORMAL,    // 视野内有玩家，心跳每秒一次
		STATE_ZOMBIE,    // 僵尸状态，如果是monster指进入战斗等
		STATE_REBORN,    // 重生状态
        STATE_RECYCLE,   // 回收状态
	};

	enum MonsterStates
	{
		MON_STATE_PEACE,          // 普通状态，和平状态
		MON_STATE_WAITING_COMBAT, // 等待战斗，等待战斗确认
		MON_STATE_COMBAT,         // 正在战斗
	};

    struct ShiftNode
    {
        int32_t cur;
        int32_t permit;
    };
	
} // namespace npcdef
} // namespace gamed

#endif // GAMED_GS_OBJ_NPC_DEF_H_
