#ifndef GAMED_GS_OBJ_MATTER_H_
#define GAMED_GS_OBJ_MATTER_H_

#include "shared/base/noncopyable.h"

#include "gs/template/map_data/map_types.h"
#include "gs/global/game_types.h"
#include "gs/global/game_def.h"

#include "unit.h"
#include "matter_def.h"


namespace gamed {

class MatterImp;
class MatterSender;

/**
 * @brief 动态物品
 */
class Matter : public WorldObject
{
	typedef mapDataSvr::TemplID TemplID;
	typedef mapDataSvr::ElemID  ElemID;

	///
	/// TODO: Matter的friend class只能调用Matter的public及protected成员函数
	/// 人为保证这个限制
	friend class MineImp;

public:
	Matter();
	virtual ~Matter();

	bool Init(const matterdef::MatterInitData& init_data);
	void HasInsertToWorld();

	// virtual func
	virtual void Release();
	virtual void OnHeartbeat();
	virtual int  OnDispatchMessage(const MSG& msg);

	bool  StepMove(const A2DVECTOR& offset);

	inline TemplID templ_id() const;
	inline ElemID  elem_id() const;
	inline bool    is_idle();
	inline matterdef::MatterStates state() const;


protected:
	void  SetIdleState();
	void  SetZombieState(int time_counter);
	void  SetRebornState(int time_counter);
    void  SetRecycleState();
    bool  CheckStateSwitch(matterdef::MatterStates new_state) const;
    void  RebornAtOnce();
    void  SetBirthPlace(const A2DVECTOR& pos);

	inline void set_state(matterdef::MatterStates state);


private:
	bool  InitRuntimeData();
	bool  HasPlayerInView() const;
	int   MessageHandler(const MSG& msg);
	bool  CheckIdleState();
	void  DriveMachine();
	void  LifeExhaust();
	void  Reborn();
	void  RecycleSelf();
	void  HandleWorldClose();
	void  MapElementDeactive();
	void  PlayerEnterView(RoleID playerid, int32_t linkid, int32_t sid);
	void  PlayerLeaveView(RoleID playerid);
    void  HandleObjectTeleport(const MSG& msg);
    bool  CanTeleport();
	

private:
	///
	/// TODO: matter的imp类不要直接访问matter的成员变量（虽然是friend class）
	///       可以直接访问public及protected的成员函数。
	///
	ElemID         elem_id_;
	TemplID        templ_id_; // 模板id每一个matter都会有，一些特殊的matter比如地图光效，
	                          // 地图npc区域锚点等，模板id定义在global/game_def.h里
	MatterImp*     pimp_;
	MatterSender*  sender_;

	int            idle_timer_;
	int            zombie_timer_;
	int            reborn_timer_;

	typedef std::set<RoleID> InViewPlayersSet;
	InViewPlayersSet in_view_players_set_;

	matterdef::MatterStates state_;
	A2DVECTOR      birth_place_;
};


/**
 * @brief Matter implement
 */
class MatterImp : shared::noncopyable
{
public:
	MatterImp(Matter& matter)
		: matter_(matter)
	{ }
	virtual ~MatterImp() { }

	virtual bool OnInit()                           { return true; }
	virtual void OnHeartbeat()                      { }
	virtual int  OnMessageHandler(const MSG& msg)   { return -1; }
	virtual void OnPlayerEnterView(RoleID playerid) { }
	virtual void OnPlayerLeaveView(RoleID playerid) { }
    virtual bool OnMapTeleport()                    { return false; }

	virtual bool CanAutoReborn() const              { return true; }
	virtual int  GetRebornInterval()                { return kMatterDefaultRebornInterval; }
    virtual void SetBirthPlace(const A2DVECTOR& pos) { }


protected:
	Matter&    matter_;
};

#include "matter-inl.h"

} // namespace gamed

#endif // GAMED_GS_OBJ_MATTER_H_
