#ifndef GAMED_GS_OBJ_AREA_H_
#define GAMED_GS_OBJ_AREA_H_

#include <limits.h>
#include <map>

#include "gs/template/map_data/map_types.h"

#include "object.h"


namespace mapDataSvr
{
	class AreaWithRules;
	class TransferArea;
	class AreaMonster;
} // namespace mapDataSvr


namespace dataTempl
{
	class InstanceTempl;
} // namespace dataTempl


namespace gamed {

struct AreaInitData
{
	XID::IDType  id;
	A2DVECTOR    pos;
	mapDataSvr::ElemID  elem_id;
};

class AreaEventIf;
class AreaObjImp;
class AreaWithRulesImp;
class TransferAreaImp;
class LandmineAreaImp;

/**
 * @brief 区域以对象的形式真实存在于gs里，有自己的视野（进入区域多边形范围内的player等）
 */
class AreaObj : public WorldObject
{
	///
	/// TODO: AreaObj的friend class只能调用AreaObj的public及protected成员函数
	/// 人为保证这个限制
	///
	friend class AreaWithRulesImp;
	friend class TransferAreaImp;
	friend class LandmineAreaImp;

public:
	AreaObj();
	virtual ~AreaObj();

	bool Init(const AreaInitData& init_data);
	void HasInsertToWorld();

	virtual int  OnDispatchMessage(const MSG& msg);
	virtual void OnHeartbeat();
	virtual void Release();

	inline void  GetVertexes(std::vector<A2DVECTOR>& vertex_vec) const;
	inline mapDataSvr::ElemID elem_id();


protected:
	void   PushBackVertex(const A2DVECTOR& point);
	int    MessageHandler(const MSG& msg);
	void   HandleWorldClose();
	void   LifeExhaust();
	void   MapElementDeactive();
	void   PlayerEnterView(RoleID playerid);
	void   PlayerLeaveView(RoleID playerid);


private:
	struct player_in_view
	{
		RoleID   role_id;
	};


private:
	mapDataSvr::ElemID     elem_id_; // 地图元素id，此id全局唯一
	std::vector<A2DVECTOR> vertexes_;

	typedef std::map<RoleID, player_in_view> PlayerInViewMap;
	PlayerInViewMap    players_in_view_;

	AreaObjImp*  pimp_;
};

///
/// inline func
///
inline void AreaObj::GetVertexes(std::vector<A2DVECTOR>& vertex_vec) const
{
	vertex_vec.clear();
	std::copy(vertexes_.begin(), vertexes_.end(), back_inserter(vertex_vec));
}

inline mapDataSvr::ElemID AreaObj::elem_id()
{
	return elem_id_;
}


/**
 * @brief AreaObj implement
 */
class AreaObjImp
{
public:
	AreaObjImp(AreaObj& area);
	virtual ~AreaObjImp();

	virtual bool OnInit()                         { return false; }
	virtual void OnInsertToWorld()                { }
	virtual void OnHeartbeat()                    { }
	virtual int  OnMessageHandler(const MSG& msg) { return 0; }
	virtual void ObjectEnterArea(const XID& xid)  { }
	virtual void ObjectLeaveArea(const XID& xid)  { }


protected:
	AreaObj&  area_obj_;
};


/**
 * @brief 带规则的区域
 */
class AreaWithRulesImp : public AreaObjImp
{
public:
	AreaWithRulesImp(AreaObj& area);
	virtual ~AreaWithRulesImp();

	virtual bool OnInit();
	virtual void OnInsertToWorld();
	virtual void OnHeartbeat();
	virtual void ObjectEnterArea(const XID& xid);
	virtual void ObjectLeaveArea(const XID& xid);


private:
	const mapDataSvr::AreaWithRules* pelemdata_;
	AreaEventIf*  event_if_;
	bool          is_inserted_;
};


/**
 * @brief 传送区域
 */
class TransferAreaImp : public AreaObjImp
{
	static const int kTimeCounter = 3; // 单位：s
public:
	TransferAreaImp(AreaObj& area);
	virtual ~TransferAreaImp();

	virtual bool OnInit();
	virtual void OnHeartbeat();
	virtual void ObjectEnterArea(const XID& xid);
	virtual void ObjectLeaveArea(const XID& xid);


private:
	struct entry_t
	{
		int32_t   counter;
		XID       player_xid;
		int32_t   ins_tid;
		int32_t   mapid;
		A2DVECTOR pos;
	};
	void SendTransportMsg(const entry_t& ent);
	bool CheckPlayerCond(RoleID roleid) const;
	bool CheckPlayerLevel(RoleID roleid) const;
	bool CheckMapCondition(const entry_t& ent);


private:
	const mapDataSvr::TransferArea* pelemdata_;
	const dataTempl::InstanceTempl* pins_templ_;

	typedef std::map<RoleID, entry_t> InViewPlayersMap;
	InViewPlayersMap  players_map_;
};


/**
 * @brief LandmineAreaImp 暗雷区域，玩家进入可能触发战斗
 */
class LandmineAreaImp : public AreaObjImp
{
	static const float kMinCalcDistanceSquare;
public:
	LandmineAreaImp(AreaObj& area);
	virtual ~LandmineAreaImp();

	virtual bool OnInit();
	virtual void OnHeartbeat();
	virtual int  OnMessageHandler(const MSG& msg);
	virtual void ObjectEnterArea(const XID& xid);
	virtual void ObjectLeaveArea(const XID& xid);

private:
	struct entry_t
	{
		entry_t()
			: encounter_timer(0),
              next_interval(5)
		{ }

		XID       player_xid;
		int       encounter_timer;
        int       next_interval;
		A2DVECTOR prev_pos;
	};

    enum EventType
    {
        PLAYER_ENTER_AREA,
        PLAYER_LEAVE_AREA,
        ENCOUNTER_CHANGE,
    };


private:
	void TriggerCombat(entry_t& ent);
	void HandleCombatStart(RoleID playerid, const msg_obj_trigger_combat_re& msg_param);
	void HandleCombatEnd(const msg_combat_end& msg_param);
    void NotifyLandmineInfo(const entry_t& ent, EventType ev_type);


private:
	const mapDataSvr::AreaMonster* pelemdata_;

	typedef std::map<RoleID, entry_t> InViewPlayersMap;
	InViewPlayersMap  in_view_players_map_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_AREA_H_
