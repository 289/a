#ifndef GAMED_GS_OBJ_FACTION_H_
#define GAMED_GS_OBJ_FACTION_H_

namespace gamed {

enum FactionMask
{
	FACTION_NONE    = 0x0000,   // 无阵营
	FACTION_MONSTER = 0x0001,   // 普通怪物
	FACTION_PLAYER  = 0x0002,   // 普通玩家
	FACTION_RED     = 0x0004,   // 红方
	FACTION_BLUE    = 0x0008,   // 蓝方
};

enum BGFactionIndex
{
    BG_FACTION_INDEX_A = 0, // 战场第一阵营
    BG_FACTION_INDEX_B,     // 战场第二阵营
    BG_FACTION_INDEX_C,     // 战场第三阵营
    BG_FACTION_INDEX_D,     // 战场第四阵营
    BGFI_MAX
};

inline int GetNormalPlayerEnemyMask()
{
	return FACTION_MONSTER;
}

inline int GetNormalMonsterEnemyMask()
{
	return FACTION_PLAYER;
}

} // namespace gamed

#endif // GAMED_GS_OBJ_FACTION_H_
