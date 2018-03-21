#ifndef TASK_MONSTER_INFO_H_
#define TASK_MONSTER_INFO_H_

#include "basic_info.h"

namespace task
{

class MonsterKilled
{
public:
	MonsterKilled()
		: package_id(0), monster_id(0), monster_count(0), 
		drop_item_id(0), drop_item_count(0), drop_item_prob(0)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t package_id;
	int32_t monster_id;
	int32_t monster_count;
	int32_t drop_item_id;
	int32_t drop_item_count;
	int32_t drop_item_prob;

	NESTED_DEFINE(package_id, monster_id, monster_count, drop_item_id, drop_item_count, drop_item_prob);
};
typedef std::vector<MonsterKilled> MonsterKilledVec;

class GameObjInfo
{
public:
	GameObjInfo()
		: id(0), dir(DIR_RIGHT), layout(0), absolute(true), appear(APPEAR_NORMAL)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t id;
	int32_t dir;
	int32_t layout;
	Position pos;
	bool absolute;
	int8_t appear;
    std::string name;

	NESTED_DEFINE(id, dir, layout, pos, absolute, appear, name);
};

class NPCInfo : public GameObjInfo
{
public:
	inline bool CheckDataValidity() const;

    std::string action;

	void Pack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::Pack(buf);
        buf << action;
	}

	void UnPack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::UnPack(buf);
        buf >> action;
	}
};
typedef std::vector<NPCInfo> NPCVec;

class MonsterInfo : public GameObjInfo
{
public:
	MonsterInfo()
		: group(0), scene(0), active(false)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t group;
	int32_t scene;
	bool active;
    std::string action;

	void Pack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::Pack(buf);
		buf << group << scene << active << action;
	}

	void UnPack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::UnPack(buf);
		buf >> group >> scene >> active >> action;
	}
};
typedef std::vector<MonsterInfo> MonsterVec;

class MineInfo : public GameObjInfo
{
public:
	inline bool CheckDataValidity() const;

	void Pack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::Pack(buf);
	}

	void UnPack(shared::net::ByteBuffer& buf)
	{
		GameObjInfo::UnPack(buf);
	}
};
typedef std::vector<MineInfo> MineVec;

// NPC黑白名单，决定NPC的可见性
// 这里的NPC为广义NPC，目前包括NPC和矿，但不包含怪
class NPCBWInfo
{
public:
	NPCBWInfo()
		: templ_id(0), black(true), add(true)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t templ_id;
	bool black;
	bool add;

	NESTED_DEFINE(templ_id, black, add);
};
typedef std::vector<NPCBWInfo> NPCBWList;

class MapElementOp
{
public:
	MapElementOp()
		: element_id(0), open(true)
	{
	}

	inline bool CheckDataValidity() const;

	int32_t element_id;
	bool open;

	NESTED_DEFINE(element_id, open);
};
typedef std::vector<MapElementOp> MapElementVec;

inline bool MonsterKilled::CheckDataValidity() const
{
	return monster_id >= 0 && monster_count >= 0 && drop_item_id >= 0 
		&& drop_item_count >= 0 && drop_item_prob >= 0;
}

inline bool GameObjInfo::CheckDataValidity() const
{
	CHECK_INRANGE(dir, DIR_RIGHT, DIR_RIGHT_DOWN)
	return id >= 0 && pos.CheckDataValidity();
}

inline bool NPCInfo::CheckDataValidity() const
{
	return GameObjInfo::CheckDataValidity();
}

inline bool MonsterInfo::CheckDataValidity() const
{
	if (!GameObjInfo::CheckDataValidity())
	{
		return false;
	}
	return group >= 0 && scene >= 0 && appear >= APPEAR_NORMAL && appear <= APPEAR_FADEIN;
}

inline bool MineInfo::CheckDataValidity() const
{
	return GameObjInfo::CheckDataValidity();
}

inline bool NPCBWInfo::CheckDataValidity() const
{
	return templ_id > 0;
}

inline bool MapElementOp::CheckDataValidity() const
{
	return element_id > 0;
}

} // namespace task

#endif // TASK_MONSTER_INFO_H_
