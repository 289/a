#ifndef GAMED_GS_SCENE_WORLD_MANAGER_H_
#define GAMED_GS_SCENE_WORLD_MANAGER_H_

#include "shared/base/noncopyable.h"
#include "shared/base/conf.h"

#include "gs/global/game_types.h"

#include "world.h"


namespace gamed {

struct MSG;
class AreaObj;
class WorldManagerImp;
class NpcGenerator;
class AreaGenerator;
class MatterGenerator;
class InsWorldManImp;

/**
 * @brief WorldManager
 *    （1）要保证WorldManager不会去主动锁player
 */
class WorldManager : public shared::noncopyable
{
    friend class InsWorldManImp;
    friend class BGWorldManImp;
    friend class BaseWorldManImp;

public:
	enum WorldManType
	{
		INVALID_WORLD  = 0,
		NORMAL_WORLD,
		INSTANCE_WORLD,
        BATTLEGROUND_WORLD,
	};

public:
	WorldManager();
	virtual ~WorldManager();

	inline MapID   GetWorldID() const;
	inline MapTag  GetWorldTag() const;
	inline XID     GetWorldXID() const;

	bool    IsActived() const;
	void    SetActive();
	void    ClrActive();

	bool    Init(shared::Conf* pconf, const char* world_name, WorldManType wmtype, int32_t worldtag);
	void    Heartbeat();
	void    Release();
	int     DispatchMessage(const MSG& msg);
	bool    InitGenWorldObjects();
	bool    InitRuntimeModule(WorldManType wmtype);
	void    SetObjGenerator(NpcGenerator* npcgen, AreaGenerator* areagen, MatterGenerator* mattergen, int64_t ownerptr);
	void    SetWorldTag(int32_t world_tag);
	bool    CreateWorldPlane();

	// ---- thread safe ----
	int     InsertPlayer(Player* player_ptr);
	void    RemovePlayer(Player* player_ptr);
	int     InsertNpc(Npc* npc_ptr);
	void    RemoveNpc(Npc* npc_ptr);
	int     InsertAreaObj(AreaObj* area_ptr);
	void    RemoveAreaObj(AreaObj* area_ptr);
	int     InsertMatter(Matter* matter_ptr);
	void    RemoveMatter(Matter* matter_ptr);

	// ---- thread safe ----
	void    RemoveSelfFromCluster();

    // ---- thread safe ----
    // 本地图的功能函数
    int32_t GetMapCounter(int32_t index) const;
    bool    IsMapCounterLocked(int32_t index) const;
    bool    CheckPlayerCountLimit(world::BGEnterType enter_type) const;
    void    GetAllPlayerInWorld(std::vector<XID>& player_vec);
    void    GetAllPlayerLinkInfo(std::vector<world::player_link_info>& player_vec);
    int32_t GetPlayerCount(); // 获取地图内玩家数量
    void    SetMapTeamInfo(void* buf, int len);

	// **** thread unsafe ****
	inline bool PosInWorld(const A2DVECTOR& pos) const;
	inline bool IsWalkablePos(const A2DVECTOR& pos) const;
	inline bool SetInsInfo(const world::instance_info& info);
	inline bool GetInsInfo(world::instance_info& info) const;
    inline bool SetBGInfo(const world::battleground_info& info);
    inline bool GetBGInfo(world::battleground_info& info) const;
    inline void SetMapTeamFlag();
    inline void SetMapCounterFlag();
    inline void SetMapGatherFlag();
    inline bool IsClosed() const;


protected:
	// virtual func
	virtual bool OnInit();
	virtual void OnHeartbeat();
	virtual int  OnDispatchMessage(const MSG& msg);
	/**
	 * @brief OnMessageHandler 
	 * @return 返回1表示处理错误，返回0表示成功，返回-1表示没有对应的处理逻辑（这里应该assert）
	 */
	virtual int  OnMessageHandler(const MSG& msg);


public:
    ///
	/// 以下的Send函数原则上只能由自身、自己的imp类或者自己的成员类调用（人肉保证）
	///
	/**
	 * @brief SendObjectMSG
	 *     向object发送消息
	 */
	void SendObjectMSG(int message, const XID& target, int64_t param = 0) const;
	void SendObjectMSG(int message, const XID& target, const void* buf, size_t len) const;

	/**
	 * @brief SendWorldMSG 
	 *    向别的World发给地图消息
	 */
	void SendWorldMSG(int message, const XID& target, int64_t param = 0) const;
	void SendWorldMSG(int message, const XID& target, const void* buf, size_t len) const;


  // for friend class
protected:
    void    BroadcastToAllPlayer(int message, const void* buf, size_t len);
    bool    QueryMapElemInfo(int32_t elem_id, std::vector<XID>& obj_vec) const;
    void    CloseWorldMan();
	inline const World* plane() const;


  // func
private:
	int     InitBase(shared::Conf* pconf, const char* section, int32_t worldtag);
	bool    InitGenNpcs();
	bool    InitGenAreas();
	bool    InitGenMatters();
	int     MessageHandler(const MSG& msg);
	void    SetForbidObjectInsert();
	bool    CreateNewWorld();

	void    RecreateNpc(int32_t elem_id, int32_t templ_id);
	void    RecreateMatter(int32_t elem_id, int32_t templ_id);
	void    RecreateAreaObj(int32_t elem_id);
	bool    CreateNpcMapElem(int32_t elem_id);
	bool    CreateMatterMapElem(int32_t elem_id);
	bool    CreateAreaMapElem(int32_t elem_id);
	void    AcceptingNpc(Npc* pnpc);
	void    AcceptingMatter(Matter* pmatter);
	void    AcceptingAreaObj(AreaObj* pareaobj);
	
	void    MapElementActive(int32_t elem_id, const XID& obj);
	void    MapElementDeactive(int32_t elem_id, const XID& obj);
	void    EnableMapElement(int32_t elem_id);
	void    DisableMapElement(int32_t elem_id);
	void    QueryNpcZoneInfo(const XID& player, int32_t elem_id, int32_t link_id, int32_t sid_in_link);
	void    BroadcastWorldManClosing();
	
	inline bool IsObjGenOwner() const;


  // variable
private:
	bool    is_actived_;          // worldman是actived的才能使用

	float   min_visible_range_;   // 地图内对象的最近可见范围（小于这个距离一定可见）
	float   max_visible_range_;   // 地图内对象的最最远可视视野（大于这个距离才离开视野）

	World*  plane_;

	int64_t           obj_gen_owner_;
	NpcGenerator*     npc_gen_;
	AreaGenerator*    area_gen_;
	MatterGenerator*  matter_gen_;

	WorldManType      wm_type_;

	mutable MutexLock wm_imp_mutex_;
	WorldManagerImp*  pimp_;

	bool              is_forbid_obj_insert_;
	int64_t           sec_hb_counter_;

    typedef std::set<int64_t> MapElemObjIDSet;
	typedef std::map<int32_t, MapElemObjIDSet> ActiveMapElemMap;
	ActiveMapElemMap  map_elem_map_;
};

#include "world_man-inl.h"


/**
 * @brief WorldManagerImp
 *    （1）不要直接访问WorldManager的成员变量（虽然是friend class）
 *         可以通过调用WorldManager的public及protected函数来实现
 */
class WorldManagerImp
{
public:
	WorldManagerImp(WorldManager& worldMan)
		: world_man_(worldMan)
	{ }
	virtual ~WorldManagerImp() { }

	virtual bool OnInit()                         { return true; }
	virtual void OnHeartbeat()                    { } // 心跳单位是s
	virtual int  OnMessageHandler(const MSG& msg) { return -1; }

	virtual bool OnInsertPlayer(Player* pplayer)  { return true; }
	virtual void OnPlayerEnterMap(RoleID roleid)  { }
	virtual void OnPlayerLeaveMap(RoleID roleid)  { }
    virtual bool OnCheckPlayerCountLimit(world::BGEnterType enter_type) const { return true; }
    virtual void OnSetMapTeamInfo(void* buf, int len) { }


protected:
	WorldManager& world_man_;
};

} // namespace gamed

#endif // GAMED_GS_SCENE_WORLD_MANAGER_H_
