#ifndef GAMED_GS_GLOBAL_GMATRIX_H_
#define GAMED_GS_GLOBAL_GMATRIX_H_

#include <string>
#include <vector>

#include "shared/base/base_define.h"
#include "shared/base/singleton.h"
#include "shared/base/conf.h"

#include "game_def.h"
#include "game_types.h"
#include "obj_insertor.h"
#include "obj_manager.h"
#include "inter_msg_queue.h"
#include "gmatrix_def.h"


namespace combat {
struct MobKilled;
struct CombatPVEResult;
struct CombatPVPResult;
struct WorldBossCombatStatus;
};

namespace shared {
namespace net {
	class ProtoPacket;
}; // namespace net
}; // namespace shared


namespace gamed {

class Npc;
class Player;
class WorldObject;
class AreaObj;
class Matter;
class WorldManager;

/**
 * @brief Gmatrix
 *    1.Gmatrix函数中需要使用的struct，enum 定义在gmatrix_def.h里
 */
class Gmatrix : public shared::Singleton<Gmatrix>
{
	friend class shared::Singleton<Gmatrix>;
	friend class ObjQuery;
public:
	static inline Gmatrix* GetInstance() {
		return &(get_mutable_instance());
	}

	int Init(shared::Conf* gmconf, 
			 shared::Conf* gsconf,
			 shared::Conf* aliasconf,
             int gs_id);
	void StopProcess();
	void ReleaseAll();

	void HeartbeatTick(TickIndex tick_index);
	void LinkDisconnect(int32_t linkid);
	void MasterDisconnect(int32_t masterid);
	void SetDebugPrintMode(bool is_debug_print);

	///
	//*  STATIC PUBLIC INTERFACE FUNCTIONS */
	///
	
	// 以下对object的操作函数，所有通过FindXXXFromMan函数获取的指针，
	// 在调用obj->Lock()后，仍需调用obj->IsActived()，才能使用该指针（多线程的考虑）
	// ---- thread safe ----
	static Player*   FindPlayerFromMan(RoleID roleid);
	static Player*   AllocPlayer(RoleID roleid);
	static void      FreePlayer(Player* player_ptr);
	static void      InsertPlayerToMan(Player* player_ptr);
	static void      RemovePlayerFromMan(Player* player_ptr);
	// ---- thread safe ----
	static Npc*      FindNpcFromMan(XID::IDType id);
	static Npc*      AllocNpc();
	static void      FreeNpc(Npc* npc_ptr);
	static void      InsertNpcToMan(Npc* npc_ptr);
	static void      RemoveNpcFromMan(Npc* npc_ptr);
	// ---- thread safe ----
	static AreaObj*  FindAreaObjFromMan(XID::IDType id);
	static AreaObj*  AllocAreaObj();
	static void      FreeAreaObj(AreaObj* area_ptr);
	static void      InsertAreaObjToMan(AreaObj* area_ptr);
	static void      RemoveAreaObjFromMan(AreaObj* area_ptr);
	// ---- thread safe ----
	static Matter*   FindMatterFromMan(XID::IDType id);
	static Matter*   AllocMatter();
	static void      FreeMatter(Matter* matter_ptr);
	static void      InsertMatterToMan(Matter* matter_ptr);
	static void      RemoveMatterFromMan(Matter* matter_ptr);
	// ---- thread safe ----
	static size_t    GetMaxNpcCount();
	static size_t    GetMaxPlayerCount();
	static size_t    GetMaxAreaObjCount();
	static size_t    GetMaxMatterCount();
	// ---- thread safe ----
	static const Player*  GetPlayerByIndex(size_t index);
	static const Npc*     GetNpcByIndex(size_t index);
	static const AreaObj* GetAreaObjByIndex(size_t index);
	static const Matter*  GetMatterByIndex(size_t index);

	/**
	 * @brief InsertWorldManager 
	 *     1.InsertWorldManager需要在单线程中执行
	 *     2.Insert在启动的时候执行，因此FindWorldManager是安全的 - 
	 *       s_worldMan_map_启动后不会变（暂时没有考虑副本）
	 *     3.FindWorldManager()取到的指针是不能上锁的，因此取到的WorldManager指针 -
	 *       调用的所有函数都应该是线程安全的！
	 *     4.Player类里不能直接调用FindWorldManager()获取WorldManager的指针
	 */
	// ---- thread safe ----
	static int           InsertWorldManager(MapID id, WorldManager* manager, MapTag tag);
	static WorldManager* FindWorldManager(MapID world_id, MapTag world_tag);
	static void          GetAllWorldInCharge(std::vector<gmatrixdef::WorldIDInfo>& id_vec);

	// ---- thread safe ----
	static bool          GetWorldXID(MapID world_id, XID& xid);
	static bool          IsWorldInCharge(MapID world_id);
	static int           GetGameServerID();

	// ---- thread safe ----
	static XID::IDType   AssignNpcID(Npc* pnpc); // 分配object id
	static XID::IDType   AssignAreaObjID(AreaObj* parea);
	static XID::IDType   AssignMatterID(Matter* pmatter);

	// 载入地图数据
	static bool LoadMapDataGbd(const char* file);
	static bool LoadMoveMapGbd(int mapid, const char* path);
	
	/**
	 * @brief SendObjectMsg 
	 *     1.发送内部message
	 *     2.is_sequence指明MSG是否需要有序执行
	 */
	static void SendObjectMsg(const MSG& msg, bool is_sequence = false);
	static void SendWorldMsg(const MSG& msg);

	// 获取player职业默认模板配置
	static gmatrixdef::PlayerClassDefaultTempl GetPlayerClsTemplCfg(int cls);
	// 获取player不同职业的最大等级
	static int16_t GetPlayerMaxLevel(int16_t player_cls);
	// 获取Player不同职业的普攻ID
	static int32_t GetPlayerNormalAttackID(int16_t player_cls);
	// 获取Player不同职业的资源模型路径
	static std::string GetPlayerModelPath(int16_t player_cls);
	// 获取player升级经验
	static int32_t GetPlayerLevelUpExp(int16_t cur_lvl);
	// 获取瞄类视觉的最大等级
	static int32_t GetCatVisionMaxLevel();
	// 获取瞄类视觉升级经验
	static int32_t GetCatVisionLevelUpExp(int16_t cur_lvl);
	// 获取瞄类视觉精力消耗速度
	static int32_t GetCatVisionEPSpeedUse();
	// 获取瞄类视觉精力恢复速度
	static int32_t GetCatVisionEPGenSpeed();
    // 获取瞄类视觉精力的恢复时间间隔
    static int32_t GetCatVisionEPGenInterval();
	// 获取宠物能量升级时上限的提升量
	static const gmatrixdef::PetPowerLvlUpConfig& GetPetPowerLvlUpConfig();
	
	// 战斗发送消息给客户端
	static void CombatSendCMD(int64_t roleid, int32_t combat_id, const shared::net::ProtoPacket& packet);
	static void CombatPVEResult(int64_t roleid, int32_t combat_id, int32_t unit_id, const combat::CombatPVEResult& result, int32_t master_server_id);
	static void CombatPVPResult(int64_t roleid, int32_t combat_id, int32_t unit_id, const combat::CombatPVPResult& result, int32_t master_server_id);
	static void CombatStart(int64_t roleid, int32_t combat_id);
	static void CombatPVEEnd(int64_t object_id, int64_t world_id, int32_t combat_id, bool win, const std::vector<int64_t/*roleid*/> rolelist, const std::vector<combat::MobKilled>& mobs_killed);
	static void CombatPVPEnd(int64_t object_id, int64_t world_id, int32_t combat_id, bool win);
    static void WorldBossStatus(int32_t boss_object_id, const combat::WorldBossCombatStatus& result);

	///
	/// 获取各种配置
	///
	static const std::string& GetPlayerClassCfgDir();
	static const std::string& GetGameDataDir();
	static const std::string& GetMapDataDir();
	static const std::string& GetInsScriptDir();
    static const std::string& GetBGScriptDir();
    static const std::string& GetGameResVersion();
	static const gmatrixdef::ServerParamInfo GetServerParam();

	// 获取真实的(运行时)world id
	static MapID GetRealWorldID(MapID world_id);
    static MapID ConvertToCRWorldID(MapID world_id); // 转成跨服的地图id，发给master时会用这个函数
	static bool  IsCrossRealmServer();

    // 判断玩家是否是同一个服务器
    static bool  IsInSameServer(RoleID rhs, RoleID lhs);
    static void  SvrIdMapToMasterId(int32_t svr_id, int32_t master_id);
    static void  GetAllMasterId(std::set<int32_t>& masters);


protected:
	Gmatrix();
	~Gmatrix();


private:
	static WorldObject* LocateObjectFromMsg(const MSG& msg);
	static WorldObject* LocateObjectFromXID(const XID& target);
	// needed lock outside
	//static int  CallMessageHandler(WorldObject* obj, const MSG& msg);
	static void DispatchObjectMessage(const MSG& msg);
	static void DispatchWorldMessage(const WORLDMSG& msg);
	static void KickoutPlayerByLinkId(Player* player, void* pdata);
	static void KickoutPlayerByMasterId(Player* player, void* pdata);
	static void KickoutPlayerByGSvrStop(Player* player, void* pdata);

	// init and load files for launch gs
	int     InitNetIO(shared::Conf* gmconf);
	void    StopNetIO();
	bool    InitNetmsgProcThread();
	void    StopNetmsgProcThread();
	bool    LoadGameBinaryData(shared::Conf* gsconf);
    bool    CheckGameBinaryData() const;
	bool    InitVariousConfig(shared::Conf* gsconf, shared::Conf* aliasconf);
	bool    InitGlobalDirConfig(const std::string& config_root);
	bool    InitGameRelatedModule();
	bool    InitEventSystem(const std::string& root, shared::Conf* gsconf);
    bool    InitAuctionConfig(const std::string& path, shared::Conf* gsconf);
    bool    InitAchieveConfig(const std::string& path, shared::Conf* gsconf);
    bool    InitSysMailConfig(const std::string& path, shared::Conf* gsconf);
	bool    InitRuntimeData();

	void    CollectHeartbeatWorldMan(std::vector<WorldManager*>& list);
	void    OnlinePlayersChange(Player* player_ptr, bool is_added_player);
    // ForEachPlayer() 回调时传入的Player指针是已上锁的
	// 回调函数中不要执行过多的逻辑，因为遍历players是加锁的操作
	typedef void (*EachPlayerCallback)(Player*, void*);
	void    ForEachPlayer(EachPlayerCallback cb, void* pdata);
    void    SetSvrIdMapToMasterId(int32_t svr_id, int32_t master_id);
    void    ClrSvrIdMapToMasterId(int32_t master_id);
    int     GetMasterIdBySvrId(int32_t svr_id) const;
    void    CollectAllMasterId(std::set<int32_t>& masters) const;

	// Game Related Module Init
	bool    InitCombat();

	// Various Config Init
	bool    InitServerParamInfo(shared::Conf* gsconf, shared::Conf* aliasconf);
    bool    InitGameVersionInfo(shared::Conf* gsconf);
	bool    InitPlayerClassDefaultConfig();
	bool    InitPlayerLevelUpExpConfig();
	bool    InitCatVisionLevelUpExpConfig();
	bool    InitPetPowerLevelUpConfig();

	// Load gbd files
	bool    LoadDataTemplGbd(const char* file);
	bool    LoadExtraDataGbd(const char* file);
	bool    LoadTransferTableGbd(const char* file);
	bool    LoadSkillDataGbd(const char* skill_file, const char* action_file);
	bool    LoadTaskDataGbd(const char* file);
	bool    LoadAchieveDataGbd(const char* file);


  // static member
private:
	static ObjectManager<Player, TICK_PER_SEC, ObjInsertor, kMaxPlayerCount>   s_player_manager_;
	static ObjectManager<Npc, TICK_PER_SEC, ObjInsertor, kMaxNpcCount>         s_npc_manager_;
	static ObjectManager<AreaObj, TICK_PER_SEC, ObjInsertor, kMaxAreaObjCount> s_areaobj_manager_;
	static ObjectManager<Matter, TICK_PER_SEC, ObjInsertor, kMaxMatterCount>   s_matter_manager_;

	static int                         s_gameserver_id_;
	static shared::AtomicInt64         s_cur_highest_obj_id_;
	static gmatrixdef::ServerParamInfo s_svr_param_info_;
    static gmatrixdef::GameVersionInfo s_game_ver_info_;


  // member
private:
    //MutexLock                    heartbeat_mutex_;

	ObjMsgQueueList              obj_msg_queue_;
	ObjMsgQueueList              obj_msg_seq_queue_;  // 顺序队列，队列里的MSG都是顺序执行的
	WorldMsgQueueList            wm_msg_seq_queue_;   // WorldManager只有顺序消息队列，在tick线程执行

	typedef std::map<RoleID, int32_t> OnlinePlayersMap;
	MutexLock                    online_players_mutex_;
	OnlinePlayersMap             online_players_;        // 需要与s_player_manager_一致

	typedef std::set<MapID> WorldInChangeSet;
	WorldInChangeSet             world_inchange_set_; // gs运行时不能动态变化，必须在gs.conf里预先配好

    // 考虑到合服的情况，原有的服务器会由新master接管（roleid高16位是创建服的masterid）
    typedef std::map<int32_t, int32_t> SvrIdToMasterIdMap;
    SvrIdToMasterIdMap           svrid_to_masterid_map_;
    mutable MutexLock            svr_mid_map_mutex_;
};


#define s_pGmatrix gamed::Gmatrix::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_GMATRIX_H_
