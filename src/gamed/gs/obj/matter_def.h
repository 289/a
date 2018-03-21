#ifndef GAMED_GS_OBJ_MATTER_DEF_H_
#define GAMED_GS_OBJ_MATTER_DEF_H_


namespace gamed {
namespace matterdef {

	struct MatterInitData
	{
		MatterInitData() : elem_id(0), templ_id(0), dir(0) { }

		XID::IDType  id;
		mapDataSvr::ElemID  elem_id;
		mapDataSvr::TemplID templ_id;
		A2DVECTOR    pos;
		uint8_t      dir;
	};

	enum MatterStates
	{
		STATE_IDLE = 0,  // 视野内没有玩家，matter处于休息状态，心跳变慢
		STATE_NORMAL,    // 视野内有玩家，心跳每秒一次
		STATE_ZOMBIE,    // 僵尸状态，如果是monster指进入战斗等
		STATE_REBORN,    // 重生状态
        STATE_RECYCLE,   // 回收状态
	};

	enum MineType
	{
		MINE_INVALID = 0,
		MINE_COMMON,        // 读条式的矿
		MINE_ERASING,       // 擦图式的矿
		MINE_BLANK_FILLING, // 填空式的矿
	};

    struct ShiftNode
    {
        int32_t cur;
        int32_t permit;
    };

} // matterdef
} // gamed

#endif // GAMED_GS_OBJ_MATTER_DEF_H_
