#ifndef __GAME_MODULE_COMBAT_TYPES_H__
#define __GAME_MODULE_COMBAT_TYPES_H__

#include <vector>
#include <stdint.h>

namespace combat
{

class Object;

typedef int32_t MapID;
typedef int32_t UnitID;
typedef int32_t SkillID;
typedef int32_t TemplID;
typedef int32_t CombatID;
typedef int32_t ObjectID;
typedef int32_t Damage;
typedef int32_t PropType;
typedef int64_t RoleID;
typedef int64_t WObjID;

enum
{
	OBJ_TYPE_INVALID,
	OBJ_TYPE_PLAYER,        // 玩家
	OBJ_TYPE_NPC,           // NPC类型(包含怪物、宠物和魔偶)
	OBJ_TYPE_COMBAT_PVE,    // PVE战场
	OBJ_TYPE_COMBAT_PVP,    // PVP战场
	OBJ_TYPE_WORLD_BOSS,    // 大世界BOSS(不是战斗BOSS，对应大世界的BOSS)
};

struct XID
{
	char type;
	UnitID id;
	XID(): type(0), id(0) {}
	XID(int type_, ObjectID id_): type(type_),id(id_) {}

	bool operator ==(const XID& rhs) const { return type == rhs.type && id == rhs.id; }
	bool operator !=(const XID& rhs) const { return type != rhs.type || id != rhs.id; }
	bool IsPlayer() const                  { return type == OBJ_TYPE_PLAYER; }
	bool IsNPC() const                     { return type == OBJ_TYPE_NPC; }
	bool IsCombatUnit() const              { return type == OBJ_TYPE_PLAYER || type == OBJ_TYPE_NPC; }
	bool IsPVECombat() const               { return type == OBJ_TYPE_COMBAT_PVE; }
	bool IsPVPCombat() const               { return type == OBJ_TYPE_COMBAT_PVP; }
	bool IsCombat() const                  { return type == OBJ_TYPE_COMBAT_PVE || type == OBJ_TYPE_COMBAT_PVP; }
	bool IsWorldBoss() const               { return type == OBJ_TYPE_WORLD_BOSS; }
};

inline int ID2Type(ObjectID id)
{
	int type = 0;
	int type_mask = (id & 0xFF000000) >> 24;
	while (type_mask)
	{
		++ type;
		type_mask = type_mask >> 1;
	}
	return type;
}

inline XID MAKE_XID(ObjectID id)
{
	int type = ID2Type(id);
	return XID(type, id);
}

#define ID2INDEX(id) (id & 0x00FFFFFF)
#define MAKE_ID(type, index) (((1 << (23+type)) & 0xFF000000) + index)

typedef std::vector<XID>     XIDVec;
typedef std::vector<Object*> ObjectVec;

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_TYPES_H__
