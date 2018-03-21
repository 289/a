#ifndef GAMED_GS_PLAYER_PLAYER_H_
#define GAMED_GS_PLAYER_PLAYER_H_

#include "common/obj_data/player_data.h"
#include "client_proto/player_visible_state.h"

#include "gs/global/math_types.h"
#include "gs/global/game_def.h"
#include "gs/global/cool_down.h"
#include "gs/obj/creature.h"

#include "player_state.h"
#include "player_def.h"
#include "player_prop.h"
#include "player_value_prop.h"


namespace shared {
namespace net {
	class RpcMessage;
} // namespace net
} // namespace shared


namespace task {
	class ActiveTask;
	class FinishTask;
	class FinishTimeTask;
	class TaskDiary;
	class TaskStorage;
} // namespace task


namespace achieve
{
    class ActiveAchieve;
    class FinishAchieve;
    class AchieveData;
} // namespace achieve


namespace common {
namespace protocol {
namespace global {
	class InstanceInfo;
    class BattleGroundInfo;
} // namespace global
} // namespace protocol
} // namespace common


namespace gamed {
namespace world {
	struct instance_info;
    struct battleground_info;
} // namespace world
} // namespace gamed 


namespace gamed {

using namespace playerdef;

class CmdPacketDispatcher;
class PlayerController;
class PlayerMoveControl;
class PlayerSender;
class PlayerSubSystem;
class PlayerInventory;
class PlayerCombat;
class PlayerSkill;
class PlayerTask;
class SubSysIf;
class MasterRpcProxy;
class Filter;
class PlayerImp;
class PlayerTeam;
class PlayerMapTeam;
struct itemdata;

/**
 * @brief Player
 *    1.player函数中需要使用的struct，enum 定义在player_def.h里
 *    2.TODO: Player所有的成员变量都必须在Release()里重置及释放，因为Player指针放回对象池并没有真正delete
 *    3.成员变量首次初始化还是在构造函数里
 */
class Player : public Creature
{
	///
	/// TODO: Player的friend class尽量只调用Player的public及protected成员函数
	/// 人为保证这个限制
	///
	friend class PropertyPolicy;
	friend class PlayerImp;

public:
	Player();
	virtual ~Player();
	
	virtual void    Release(); // TODO: Player所有的成员变量都必须在Release()里重置及释放

	virtual int     OnDispatchMessage(const MSG& msg);
	virtual void    OnHeartbeat();

    // static function
    static  void    InitPlayerScriptSys(const char* script_dir);

	// **** thread unsafe ****
	bool   InitRuntimeModule(RoleID roleid);
	int    InitPlayerData(RoleID roleid, const char* pbuf, int32_t len);
	bool   InitPlayerRuntimeData();
	int    DispatchCommand(int cmd_type_no, shared::net::ProtoPacket* packet);
	int    MessageHandler(const MSG& msg);
	
	inline PlayerController*   commander();
	inline PlayerMoveControl*  move_ctrl();
	inline PlayerSender*       sender();
	inline SubSysIf*           subsys_if();
	inline PlayerState&        state();
	inline PlayerVisibleState& visible_state();
    inline PlayerValueProp&    value_prop();

	bool   HandleFirstLogin(common::PlayerData* pdata); // 首次登录需要根据职业模板给playerdata填充数据，
                                                        // 必须在LoadFromDatabaseFormat()之前调用
	
	void   PlayerLogout(LogoutType type);
	void   EnterWorld();
	void   LeaveWorld();
	void   EnterMap(); // WorldManager调用InsertPlayer之后才能调用
	void   LeaveMap(); // WorldManager调用RemovePlayer之前调用
	void   SyncDispatchMsg(const MSG& msg); // 同步发送MSG，取到player指针同步处理MSG，不进消息队列
	void   ChangeGsComplete();
	void   OnGetAllData();

	void   ElsePlayerEnterView(RoleID id, int32_t linkid, int32_t sid);
	void   ElsePlayerLeaveView(RoleID id);
	void   ClearElsePlayerInView();

    // rpc related
	void   MasterRpcMessageProc(const shared::net::RpcMessage& msg);
	void   CheckRpcTimeout(time_t cur_time);

    // PlayerData data
	inline RoleID    role_id() const;
	inline int32_t   level() const;
	inline uint8_t   role_class() const;
	inline ClassMask class_mask() const;
	inline uint8_t   gender() const;
	inline std::string rolename() const;
	inline std::string first_name() const;
	inline std::string middle_name() const;
	inline std::string last_name() const;
	inline int32_t   create_time() const;
    inline int32_t   last_login_time() const;
	inline uint32_t  equip_crc();

    // -------- Player runtime data --------
    inline PlayerState::MaskType GetState() const;
	inline float   GetSpeedByMode(int mode) const;
	inline void    set_refresh_state(bool state);
	inline void    set_link_id(int32_t id);
	inline void    set_sid_in_link(int32_t sid);
	inline void    set_master_id(int32_t id);
    inline void    set_user_id(int64_t userid);
    inline int64_t user_id() const;
	inline bool    refresh_state() const;
	inline int32_t link_id() const;
	inline int32_t sid_in_link() const;
	inline int32_t master_id() const;  // 现在masterid就是指master的session id, masterid相同表示来自同一个服务器
	inline float   cur_speed() const;
	inline int32_t faction() const;
	inline bool    can_see_each_other() const; // 玩家能不能相互看到
	inline int32_t cat_vision_level() const;
	inline void    set_equip_crc(uint32_t crc);

	///
	/// 子系统相关接口
	///
	PlayerSubSystem* QuerySubSys(int type);
	const PlayerSubSystem* QuerySubSys(int type) const;
    template <typename T>
    T* GetSubSysPtr(int type);
    template <typename T>
    const T* GetSubSysPtr(int type) const;

	///
	/// 通用接口
	///
	bool GetPlayerDataContent(shared::net::Buffer* buf);
	void SetSpecSavePos(int32_t worldid, const A2DVECTOR& pos); // 特殊地图位置是不能直接存盘的，比如副本
	bool GetSpecSavePos(int32_t& worldid, A2DVECTOR& pos) const;
	void PullBackToValidPos(int seq_from_cli);
    void ErrorMessage(int err_no) const;
    void Say(const std::string& content); // 向客户端发附近聊天（只有自己能收到），debug命令使用

	///
	/// 冷却相关接口
	///
    /// 冷却id，可以是服务器自己定义的index也可以是策划配的模板id
    /// 检查物品冷却都是策划配的模板id（包括冷却组id，物品自己的模板id）
	void PlayerGetCoolDownData();
	void SetCoolDown(int id, int msec);
	void ClrCoolDown(int id);
	bool TestCoolDown(int index); // 检查index，index枚举见playerdef
	bool TestItemCoolDown(int cd_group_id, int item_id); // 检查物品冷却用这个函数

	///
	/// 技能相关
	///
	void LevelUpSkill();
	void LevelUpSkill(int32_t sk_tree_id, int8_t lvl, int8_t lvlup_mode);
    void LevelUpAllSkill(int8_t lvl, int8_t lvlup_mode);
	void QueryAvailableSkillTree(std::vector<playerdef::SkillTreeInfo>& skill_tree_info_list) const;
    void CastSkill(int32_t skill_id);
    int32_t IsSkillExist(int32_t skill_id) const;

	///
	/// 物品、包裹相关接口
	///
	int32_t GetWeaponID() const;
	int32_t CountItem(int item_type) const;
	int32_t CountEquipItem(int item_type) const;
	bool IsInvFull(int where) const;
    // 通过mode可以知道物品是哪个系统发的，客户端需要做些处理，比如副本结束的抽奖
	bool GainItem(int item_type, int item_count);
	bool GainItem(int item_type, int item_count, int valid_time, GainItemMode mode);
	bool GainItem(int where, itemdata& item, GainItemMode mode);
	bool CheckItem(int item_type, int count) const;
	bool CheckItem(int item_idx, int item_type, int count) const;
	bool HasSlot(int where, int empty_slot_count) const;
	bool TakeOutItem(int item_type, int count);
	bool TakeOutItem(int item_idx, int item_type, int count);
	bool TakeOutAllItem(int item_type, int count);
	bool QueryItem(int where, int item_idx, itemdata& item) const;
	void QueryItemlist(int where, int& inv_cap, std::vector<itemdata>& itemlist) const;
	bool UpdatePetItem(int item_idx);
	bool UpdateCardItem(int item_idx);
	bool CanTrade(int where, int item_idx) const;
    int32_t GetEquipNum(int32_t refine_level, int32_t rank) const;
    int32_t GetCardNum(int32_t level, int32_t rank) const;
    bool GetItemListDetail(int where, std::vector<itemdata>& list) const;

	///
	/// 宠物相关
	///
	bool GainPet(int32_t pet_item_id, int pet_item_idx);
	bool RegisterPet(const playerdef::PetEntry& pet);
	bool UnRegisterPet(int pet_item_idx);
	bool QueryPetInfo(int pet_item_idx, playerdef::PetEntry& pet) const;
	void QueryCombatPetInfo(std::vector<playerdef::PetInfo>& list) const;
	void QueryPetPowerInfo(int32_t& power, int32_t& power_cap, int32_t& power_gen_speed) const;
    int32_t GetCombatPetNum(int32_t level, int32_t blevel, int32_t rank) const;
    int32_t QueryPetAttackCDTime() const;
	void GainPetExp(int32_t exp);
	void GainPetPower(int32_t offset);
	void SetPetPower(int32_t power);
	void IncPetPowerGenSpeed(int32_t value);
	void DecPetPowerGenSpeed(int32_t value);
    void IncPetAttackCDTime(int32_t time);
    void DecPetAttackCDTime(int32_t time);

	///
	/// 金钱相关接口
	///
	int64_t GetMoney() const;
	bool    CheckMoney(int32_t money) const;
	void    GainMoney(int32_t money);
	void    SpendMoney(int32_t money);

	///
	/// 元宝相关
	///
	int32_t GetCash() const;
	void    AddCash(int32_t cash_add); // 只增加当前可用值，并不存盘
	void    SetCashTotal(int32_t cash_total);//充值时调用,保证即时到账
	void    UseCash(int32_t offset);
	bool    CheckCash(int32_t cash) const;
    void    AddCashByGame(int32_t cash_add); // 增加游戏里获得的元宝（区别于游戏充值），通知master修改数据库，返回成功后才调用AddCash

	///
    /// 学分相关
	///
	void	PlayerGetOwnScore();
	int64_t GetScore() const;
    int64_t GetScoreTotal() const;
    int64_t GetScoreUsed() const;
	bool    CheckScore(int32_t score) const;
	void    GainScore(int32_t score);
	void    SpendScore(int32_t score);

	///
	/// 天赋相关
	///
    bool    HasTalent(int32_t talent_group_id, int32_t talent_id) const;
    bool    HasTalentGroup(int32_t talent_group_id) const;
    void    OpenTalent(int32_t talent_group_id, int32_t talent_id);
	void    OpenTalentGroup(int32_t talent_group_id);
	void    CloseTalentGroup(int32_t talent_group_id);
	void    LevelUpTalent(int32_t talent_group_id, int32_t talent_id);
	void    IncTalentTempLevel(int32_t talent_group_id, int32_t talent_id);
	void    DecTalentTempLevel(int32_t talent_group_id, int32_t talent_id);
	void    QueryTalentSkill(std::set<int32_t>& skills) const;
    int32_t GetTalentLevel(int32_t talent_group_id, int32_t talent_id) const;

	///
	/// 上线发送数据给客户端的接口
	///
	void PlayerGetInventoryAll();
	void PlayerGetInventory(int where);
	void PlayerGetInventoryDetail(int where);
	void PlayerGetBaseInfo();
	void PlayerGetVolatileInfo();
	void PlayerGetExtendProp();
	void PlayerGetSkillData();
	void PlayerGetTaskData();
	void PlayerGetPetData();
	void PlayerGetOwnMoney();
	void PlayerGetCombatData();
	void PlayerGainMoney(int32_t amount);
	void PlayerSpendMoney(int32_t cost);
	void PlayerGetBuddyInfo();
	void PlayerGetInsTeamInfo();
	void PlayerGetStaticRoleInfo();
	void PlayerGetCash();
	void PlayerAnnounceNewMail();
	void PlayerGetTalentData();
    void PlayerGetFriendData();
    void PlayerGetTitleData();
    void PlayerGetReputationData();
    void PlayerGetUIConf();
    void PlayerGetAchieveData();
    void PlayerGetLoginData();
    void PlayerGetStarData();
    void PlayerGetEnhanceData();
    void PlayerGetPCounterList() const;
    void PlayerGetPunchCardData() const;
    void PlayerGetMoveSpeed();
    void PlayerGetGameVersion();
    void PlayerGetBossChallengeData() const;

	///
	/// 属性相关接口
	///
	inline int32_t GetHP() const;
	inline int32_t GetExp() const;
	inline int32_t GetMP() const;
	inline int32_t GetEP() const;
	inline int32_t GetMaxHP() const;
	inline int32_t GetMaxMP() const;
	inline int32_t GetMaxEP() const;
	inline int32_t GetLevel() const;
	inline int32_t GetCatVisionExp() const;
	inline int32_t GetCatVisionLevel() const;
	inline int32_t GetMaxProp(size_t index) const;
    inline int32_t GetBaseProp(size_t index) const;
	inline void    SetLevel(int32_t level);
	void    GenHP(int32_t hp);
	void    GenMP(int32_t mp);
	void    GenEP(int32_t ep);
	void    IncHP(int32_t hp);
	void    DecHP(int32_t hp);
	void    IncMP(int32_t mp);
	void    DecMP(int32_t mp);
	void    IncEP(int32_t ep);
	void    DecEP(int32_t ep);
	void    SetHP(int32_t hp);
	void    FullHP();
	void    SetMP(int32_t mp);
	void    SetEP(int32_t ep);
	void    IncPropPoint(size_t index, int32_t value);
	void    DecPropPoint(size_t index, int32_t value);
	void    IncPropScale(size_t index ,int32_t value);
	void    DecPropScale(size_t index ,int32_t value);
	void    IncEquipPoint(size_t index, int32_t value);
	void    DecEquipPoint(size_t index, int32_t value);
	void    IncEquipScale(size_t index, int32_t value);
	void    DecEquipScale(size_t index, int32_t value);
	void    IncExp(int32_t exp);
	void    LevelUp();
	void    TransferCls(int32_t dest_cls);
	void	TransferGender(uint8_t gender);
	int32_t GetClsTemplId() const;
	void    QueryExtProp(std::vector<int32_t>& props) const;
    void    IncSpeed(int32_t value);
    void    DecSpeed(int32_t value);

    ///
	/// 伤害相关
	///
	void    Die();
	void    Resurrect(int choice); // 复活
    void    ForceResurrect();      // 强制复活，地图踢人的时候需要强制复活
	void    RecoverHP(int32_t hp);
	int     DoDamage(int32_t damage);

	///
	/// 战斗相关
	///
	int32_t GetCombatSceneID() const;        // 获取战斗场景
    int32_t GetCombatValue() const;          // 获取战斗力，线程安全的
	int32_t CalcCombatValue() const;         // 计算战斗力
	bool    CheckJoinCombat(RoleID someone); // 加入战斗
	void    BroadCastEnterCombat(int32_t combat_id, int32_t world_boss_id); // 向周围的队友或同阵营玩家广播我进入战斗
	int     BroadCastQueryTeamCombat(int64_t monster_obj_id); // 向周围队友广播查询战斗id，用于共享血量战斗
	int32_t GetCombatPosition();             // 获取战斗站位
	int32_t QueryTeammateCountEnterCombat(); // 获取能进入战斗的队友数
	bool    TerminateCombat();
	bool    StartPVPCombat(int32_t combat_scene_id, playerdef::CombatPVPType type, playerdef::StartPVPResult& result);
    bool    FirstJoinPVPCombat(int32_t combat_id);
    void    HandleCombatPVPEnd(const playerdef::PVPEndInfo& info, playerdef::PVPEndExtraData& data);
	inline bool InCombat() const;
	inline bool CanCombat() const;

	///
	/// Buff相关
	///
	void    AddFilter(Filter* filter);
	void    DelFilter(int32_t filter_type);
    void    DelFilterByID(int32_t effectid);
	void    DelSpecFilter(int32_t filter_mask);
    void    ModifyFilterByID(int32_t effectid, bool is_add, bool reject_save = false);
    bool    IsFilterExistByID(int32_t effectid);

	///
	/// 暗雷相关
	///
	void    LandmineAccumulate();
	void    RejectLandmineOneTime();
	bool    CheckLandmineAccumulate(int landmine_interval);
	inline void IncLandmineAccumulate(int32_t count);
    inline void DecLandmineAccumulate(int32_t count);
	inline void ClrLandmineAccumulate();
	inline void SetLandmineAccumulate(int32_t count);
	
	///
	/// 设置被动标志
	///
	inline void RejectPassivityFlag();
	inline void ClrRejectPassivityFlag();

	///
	/// 组队相关（以下函数优先级：副本组队 > 本服普通组队）
	///
	int     GetTeamId() const;
	bool	IsInTeam() const;
	bool    IsInTeam(int32_t teamid) const;
	bool    IsTeamLeader() const;
	bool    IsTeammate(RoleID roleid) const;  // roleid不包括自己
	bool    GetTeammates(std::vector<RoleID>& members_vec); // 获取队友，不包含自己
	RoleID  GetLeaderId() const;  // 返回0表示没有组队或没有取到队长id
	int     GetPosInTeam(); // 返回值小于0表示没有队伍
    int     GetTeammateCount() const; // 不包含自己

	// 以下是普通组队（本服组队）
	void    ProcessTeamMsg(const MSG& msg);
	void    JoinTeamTransmit(RoleID other_roleid);
	void    JoinTeamResTransmit(bool invite, bool accept, RoleID requester);
	bool    CanJoinPlayerTeam();
	void    LeaveTeamBySelf();
	void    SendJoinTeamReq(RoleID requester, const std::string& first_name, 
                            const std::string& mid_name, const std::string& last_name, int32_t invite);
    void    TeamLeaderConvene(RoleID leader, int32_t world_id, const A2DVECTOR& pos);

	// 以下地图组队
	void    SyncMapTeamInfo(const MSG& msg);

	///
	/// 伙伴组队相关
	///
	void    ObtainBuddy(int32_t buddy_tid);
	void    TakeoutBuddy(int32_t buddy_tid);
	bool    GetBuddyMembers(std::vector<playerdef::BuddyMemberInfo>& buddy_vec) const;

	///
	/// 刷新状态
	///
	inline void    SetRefreshState();
	inline void    SetRefreshExtProp(size_t idx);
	inline void    ClrRefreshState();
	inline void    ClrRefreshExtProp();
	inline bool    GetRefreshState() const;
	inline int32_t GetRefreshExtProp() const;

	///
	/// ai相关
	///
	void    SayHelloToNpc(const XID& target);

	///
	/// 传送相关
	///
	bool    RegionTransport(int32_t world_id, const A2DVECTOR& pos);
	void    NotifySelfPos();
	void    ChangeMapError(bool is_change_gs_err);
    bool    IsChangeMapFillMapTeam(int32_t target_world_id) const;
	inline bool CanSwitch() const;

	///
	/// 任务相关
	///
	task::ActiveTask*     GetActiveTask();
	task::FinishTask*     GetFinishTask();
	task::FinishTimeTask* GetFinishTimeTask();
	task::TaskDiary*	  GetTaskDiary();
	task::TaskStorage*	  GetTaskStorage();
	bool    HasActiveTask(int32_t task_id);
	bool    HasFinishTask(int32_t task_id);
	void    KillMonster(int32_t monster_tid, int32_t monster_num, std::vector<playerdef::ItemEntry>& items);         // 刚刚结束的战斗杀死哪些怪
	void    KillMonsterLastTime(int32_t monster_tid, int32_t monster_num, std::vector<playerdef::ItemEntry>& items); // 玩家离线，上次杀了哪些怪
	int 	DeliverTask(int32_t task_id);
	void    DeliverAward(int32_t task_id, int32_t choice);
	int     CanDeliverTask(int32_t task_id) const;
	inline void SetRuningDramaScript(bool is_runing);
	void	UpdateTaskStorage(int32_t sid);
	bool    TaskMiniGameEnd(int32_t game_tid);
    void    StartRecvWorldTask();
    bool    CombatFail(int32_t task_id);
    bool    IsTaskFinish(int32_t task_id);

    ///
    /// 成就相关
    ///
    achieve::ActiveAchieve* GetActiveAchieve();
    achieve::FinishAchieve* GetFinishAchieve();
    achieve::AchieveData*   GetAchieveData();
    void    StorageTaskComplete(int32_t taskid, int8_t quality);
    void    FinishInstance(int32_t ins_id);
    int32_t GetInsFinishCount(int32_t ins_id) const;

	///
	/// 瞄类视觉相关
	///
	void    OpenCatVision();       //玩家开启瞄类视觉
	void    CloseCatVision();      //玩家关闭瞄类视觉
	void    DeActivateCatVision(); //服务器关闭瞄类视觉
	void    CatVisionGainExp(int32_t exp);
	void    CatVisionLevelUp();
	void    GainCatVisionPower(int32_t power);

	///
	/// 扩展状态相关
	///
	void SetImmuneActiveMonster();
	void ClrImmuneActiveMonster();
	bool IsImmuneActiveMonster() const;
	void SetImmuneLandmine();
	void ClrImmuneLandmine();
	bool IsImmuneLandmine() const;
	void SetImmuneTeamCombat();
	void ClrImmuneTeamCombat();
	bool IsImmuneTeamCombat() const;

	///
	/// 获取其它玩家数据
	///
	void    GetElsePlayerExtProp(int64_t target);   //请求其它玩家的扩展属性
	void    GetElsePlayerEquipCRC(int64_t target);  //请求其它玩家的装备CRC
	void    GetElsePlayerEquipment(int64_t target); //请求其它玩家的装备信息

	///
	/// Object黑白名单
	///
	bool    ObjectCanInteract(int32_t tid);
	void    ModifyNPCBWList(int32_t templ_id, bool is_black, bool is_add);
	void	ClearNPCBWList();

	///
	/// 副本相关
	///
	bool    GetRuntimeInsInfo(MapID target_world_id, common::protocol::global::InstanceInfo& insinfo) const;
	void    SetInstanceInfo(const world::instance_info& info);
	void    ResetInstanceInfo();
	bool    CheckInstanceCond(const playerdef::InsInfoCond& info) const;
	bool    ConsumeInstanceCond(int32_t ins_tid, bool is_create_ins);
	bool    GetInstancePos(A2DVECTOR& pos) const;
	bool    QueryInsEntrancePos(A2DVECTOR& pos) const;
	void    TaskTransferSoloIns(int32_t ins_tid) const;
	void    GeventTransferSoloIns(int32_t ins_tid) const; // 活动传送单人副本
	void    CalcInstanceRecord(int32_t ins_tid, int32_t clear_time, bool is_svr_record, int32_t last_record);
	void    FinishInstance(int32_t ins_id, bool succ) const;
    
    ///
    /// 战场相关
    ///
    bool    GetRuntimeBGInfo(MapID target_world_id, common::protocol::global::BattleGroundInfo& bginfo) const;
	void    SetBattleGroundInfo(const world::battleground_info& info);
	void    ResetBattleGroundInfo();
	bool    CheckBattleGroundCond(const playerdef::BGInfoCond& info) const;
	bool    ConsumeBattleGroundCond(int32_t bg_tid);
	bool    GetBattleGroundPos(A2DVECTOR& pos) const;
	bool    QueryBGEntrancePos(A2DVECTOR& pos) const;
    void    EnterPveBattleGround(int32_t bg_tid) const; // 进入PVE战场
    void    QuitBattleGround(int32_t bg_tid) const;

	///
	/// 玩家改名
	///
	void    ChangeName(playerdef::NameType name_type, const std::string& name);
	void    TaskChangeName(int32_t taskid, playerdef::NameType name_type, const std::string& name);
	void    ChangeNameSuccess(int8_t name_type, const std::string& name, int32_t task_id);

	///
	/// 邮件相关
	///
	void	SendSysMail(shared::net::ByteBuffer& data);
	void	SendSysMail(int32_t gold, int32_t item_id);
    static void SendSysMail(int32_t master_id, RoleID receiver, shared::net::ByteBuffer& data); // 会修改data的值
    static void SendSysMail(int32_t master_id, RoleID receiver, const playerdef::SysMail& sysmail);

	///
	/// 聊天相关
	///
	void	SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content);

	///
	/// 地图元素
	///
	void    CtrlMapElement(int32_t elem_id, bool open);

	///
	/// Implement
	///
	template <typename T>
	void    ChangePlayerImp();

	// 重置存盘间隔，强制玩家在下一个HeartBeat存盘
	void    ResetWriteTime();

    ///
    /// 拍卖相关
    ///
    void    AuctionItem(const playerdef::AuctionItemData& data);
    void    AuctionItemResult(int err, int64_t auction_id, const void* request);
    void    AuctionCancel(int64_t auction_id);
    void    AuctionCancelResult(int err_code, int64_t auction_id, const void* response) const;
    void    AuctionBuyout(int64_t auction_id, int32_t currency_type, int32_t price);
    void    AuctionBuyoutResult(int err_code, const void* request);
    void    AuctionBid(int64_t auction_id, int32_t currency_type, int32_t price);
    void    AuctionBidResult(int err_code, int32_t cur_price, int64_t cur_bidder, const void* request);
    void    AuctionInvalidQuery(const std::vector<int64_t>& auctionid_list);

    ///
    /// 好友相关
    ///
    bool    IsFriend(int64_t roleid) const;
    int32_t GetFriendNum() const;
    int32_t GetEnemyNum() const;
    void    AddNPCFriend(int32_t id);
    void    DelNPCFriend(int32_t id);
    void    OnlineNPCFriend(int32_t id);
    void    OfflineNPCFriend(int32_t id);

    ///
    /// 称号相关
    ///
    void    QueryTitleSkill(std::set<int32_t>& skills) const;
    bool    GainTitle(int32_t title_id);

    // 声望相关
    void    OpenReputation(int32_t reputation_id);
    int32_t GetReputation(int32_t reputation_id); // 返回-1表示该声望还没有开启
    void    ModifyReputation(int32_t reputation_id, int32_t delta);

    // UI配置相关
    void    ModifyUIConf(int32_t ui_id, bool show);

    // 成就相关
    void    RefineAchieve(int32_t level);

    // 属性强化相关
    void    PlayerGetPropReinforce();
    void    ResetPrimaryPropRF(); // just for debug
    void    AddPrimaryPropRFCurEnergy(); // just for debug

    // 附魔相关
    void    OpenEnhanceSlot(int8_t mode);
    void    ProtectEnhanceSlot(int8_t index);
    void    UnProtectEnhanceSlot(int8_t index);
    void    DoEnhance(int32_t enhance_gid, int32_t count);
	void    QueryEnhanceSkill(std::set<int32_t>& skills) const;
    int32_t GetUnProtectSlotNum();
    int32_t GetProtectSlotNum();
    int32_t RandSelectEnhanceId(int32_t enhance_gid);
    int32_t GetEnhanceNum(int32_t level, int32_t rank);

    // 星盘相关
    void    OpenStar(int32_t star_id);
    void    CloseStar(int32_t star_id);
    void    ActivateSpark(int32_t star_id);
    bool    AllSparkActivate(int32_t star_id) const;
    int32_t GetFullActivateStarNum() const;
    int32_t GetSparkNum() const;
    bool    IsStarOpen(int32_t star_id) const;

    // 卡牌相关
    bool    GainCard(int32_t card_item_id, int32_t card_item_idx);
    bool    RegisterCard(const playerdef::CardEntry& card);
	bool    UnRegisterCard(int32_t card_item_idx);
	bool    QueryCardInfo(int32_t card_item_idx, playerdef::CardEntry& card) const;
	void    QueryCardSkill(std::set<int32_t>& skills) const;
    int32_t GetEquipCardNum(int32_t rank, int32_t level) const;

    // 玩家计数器
    bool    RegisterPCounter(int32_t tid);
    bool    UnregisterPCounter(int32_t tid);
    bool    ModifyPCounter(int32_t tid, int32_t delta);
    bool    GetPCounter(int32_t tid, int32_t& value) const;
    bool    SetPCounter(int32_t tid, int32_t value);
    void    ModifyPCounter(int32_t id, int8_t op, int32_t value);

    // 全局计数器
    void    SubscribeGlobalCounter(int32_t index, bool is_subscribe);
    bool    GetGlobalCounter(int32_t index, int32_t& value) const;
    void    ModifyGlobalCounter(int32_t index, int8_t op, int32_t delta); // op是task::COUNTER_OP_ASSIGN

    // 地图计数器
    void    SubscribeMapCounter(int32_t world_id, int32_t index, bool is_subscribe);

    // 活跃度
    void    PlayerGetParticipation() const;
    void    ModifyParticipation(int32_t value);
    void    SetParticipation(int32_t value);

    // 活动
    void    PlayerGetGeventData() const;
    void    CompleteGevent(int32_t gevent_gid);
    void    SetGeventNum(int32_t gevent_gid, int32_t num); // just for debug
    void    ClearParticipationAward();  // just for debug

    // 坐骑
    void    PlayerGetMountData() const;
    void    RegisterMount(int32_t item_index, int32_t item_id);
    void    UnRegisterMount(int32_t item_index);
    void    OpenMountEquip(int32_t equip_index);
    int32_t GetMountCategory() const;
    int32_t GetMountEquipLevel(int32_t index) const;

    // 签到
    void    ResetPunchCard(); // just for debug

    // 界面BOSS
    void    WinBossChallenge(int8_t result, int32_t challenge_id, int32_t monster_gid);
    void    ClearBossChallenge(int32_t challenge_id);


private:
	bool    LoadPlayerDataFromTempl(common::PlayerData* pdata);// 首次登陆从模板中读取数据
	void    FillRumtimeData(const common::PlayerData& data);
	void    CopyRuntimeData(common::PlayerData* pdata);
	bool    LoadFromDatabaseFormat(const common::PlayerData& data);
	bool    SaveToDatabaseFormat(common::PlayerData* pdata);
	bool    GetPlayerDataOctetStr(shared::net::Buffer* buf, bool allow_logout = true); // 调用此函数返回false时，应该让玩家下线
	void    PlayerDataSyncToMaster(SaveDataOpType op_type);
	void    RefreshAreaRulesInfo(int32_t elem_id, bool is_added);
	void    ResetAreaRulesInfo();
	void    CheckTimeToSaveData();
	bool    LongJump(int32_t target_world_id, const A2DVECTOR& pos); // 地图跳转，包含不再本gs的地图及副本
	bool    LocalJump(const A2DVECTOR& pos); // 本地图跳跃，调用前需要做player的状态检查
	void    PlayerChangeMap(int32_t target_world_id, const A2DVECTOR& pos); // 普通地图切换gs
	bool    CheckSpecMapLongJumpRule(MapID target_world_id, MapID src_world_id) const;
	bool    CheckUnnormalMapLeaveRule(MapID target_world_id, MapID src_world_id) const;
	void    DriveRuntimeVolatileData();
	void    HandleWorldClose(const MSG& msg);
	void    CheckPassivityFlag();
	bool    GetDramaSavePos(int32_t& worldid, A2DVECTOR& pos) const; // 获取剧情脚本最初位置
    void    DoResurrect(); // 真正执行复活指令，调用前需要判断玩家是否死亡
    void    ClearLandmineTrigger();
    void    ClearMapCounterList();
    void    UpdatePropertyAndSpeed();
    void    ModifyPlayerExtraInfo();
    void    HandleKillMonster(int32_t monster_tid, int32_t monster_num);
    void    MonsterChangeCounter(int32_t index, int32_t type, int32_t delta, bool is_player);

	// 组队相关
	PlayerTeam*          GetTeamPtr();
	const PlayerTeam*    GetTeamPtr() const;
	PlayerMapTeam*       GetMapTeamPtr();
	const PlayerMapTeam* GetMapTeamPtr() const;


private:
	static CmdPacketDispatcher  s_cmd_dispatcher_;

// data needed to save to DB
	common::PlayerRoleInfo      role_info_;
	common::PlayerBaseInfo      base_info_;
	common::PlayerPreloadData   preload_data_;
	common::PlayerSpecMoney     spec_money_;

// Player runtime data
	PlayerController*    commander_;
	PlayerMoveControl*   move_ctrl_;
	PlayerSender*        sender_;
	SubSysIf*            subsys_if_;
	MasterRpcProxy*      rpc_master_;
	PlayerState          state_;
	ExtendState          extend_state_;
	PlayerVisibleState   visible_state_;

    ExtendProp           base_prop_[PROP_INDEX_HIGHEST];    // 玩家基本属性
    ExtendProp           cur_prop_[PROP_INDEX_HIGHEST];     // 玩家当前属性，不会变的属性则代表最大值
    ExtendProp           enh_point_[PROP_INDEX_HIGHEST];    // 按点数增强的属性
    ExtendProp           enh_scale_[PROP_INDEX_HIGHEST];    // 按百分比增强的属性(万分数)
    ExtendProp           equip_point_[PROP_INDEX_HIGHEST];  // 装备增强的点数属性
    ExtendProp           equip_scale_[PROP_INDEX_HIGHEST];  // 装备增强的百分比属性
    uint32_t             equip_crc_;                        // 装备的CRC(使用adler32码代替)
    bool                 refresh_state_;                    // 玩家基本属性发生变化
    int32_t              refresh_extprop_;                  // 玩家扩展属性发生变化

	int32_t              link_id_;
	int32_t              sid_in_link_;
	int32_t              master_id_;  // 现在masterid就是指master的session id，masterid相同表示来自同一个服务器

    int64_t              user_id_;    // 玩家的账号id
	int32_t              faction_;
	int32_t              write_timer_;
	int16_t              hp_gen_counter_;
	int16_t              ep_gen_counter_;
	int16_t              vip_level_;           // 玩家VIP等级
	bool                 cat_vision_state_;    // 瞄类视觉是否打开
    mutable int32_t      combat_value_;        // 上次计算战斗力的值
	LandmineRecord       landmine_accumulate_; // 暗雷累积值，area发过来的时间间隔小于该值时可以触发战斗

	SpecSavePos          spec_save_pos_;  // 特殊存盘位置，副本或战场里位置不能存盘，
	                                      // 该值保存进特殊地图前玩家在普通地图的位置，该值只有下线会清除，
										  // 副本或战场离开地图就是一次下线处理
	
	DramaSavePos         drama_save_pos_; // 记录执行脚本开始时的位置，如果脚本没有执行完毕就下线，
	                                      // 会把玩家设回该位置，下次上线播脚本时不会出现位置错误。
										  // 该值的优先级低于spec_save_pos_
                                          
    PlayerValueProp      value_prop_;     // 玩家的数值属性，一些简单数值又不好分到子系统的都放这里
	
	PlayerImp*           pimp_;           // player implement
	PassivityFlag        passivity_flag_; // 有被动标记时：不能进入战斗、不能进行传送

	typedef std::set<RoleID> ElsePlayersSet;
	ElsePlayersSet           else_players_set_;

	// area related
	typedef std::vector<int32_t> RuleAreaVec;
	RuleAreaVec              rule_areas_vec_;
	playerdef::AreaRulesInfo area_rules_info_;
};

///
/// template func
///
template <typename T>
void Player::ChangePlayerImp()
{
	DELETE_SET_NULL(pimp_);
	pimp_ = new T(*this);
	ASSERT(pimp_ != NULL);
}

template <typename T>
T* Player::GetSubSysPtr(int type)
{
    PlayerSubSystem* pSubSys = QuerySubSys(type);
	ASSERT(pSubSys);
	T* ptr = dynamic_cast<T*>(pSubSys);
    return ptr;
}

template <typename T>
const T* Player::GetSubSysPtr(int type) const
{
    const PlayerSubSystem* pSubSys = QuerySubSys(type);
	ASSERT(pSubSys);
	const T* ptr = dynamic_cast<const T*>(pSubSys);
    return ptr;
}


///
/// PlayerLockGuard
///
class PlayerLockGuard
{
public:
	explicit PlayerLockGuard(Player* player)
		: player_(player)
	{
		player_->Lock();
	}

	~PlayerLockGuard()
	{
		player_->Unlock();
	}

private:
	Player* player_;
};

#define PlayerLockGuard(x) SHARED_STATIC_ASSERT(false); // missing guard var name


/**
 * @brief PlayerImp
 *    （1）不要直接访问Player的成员变量（虽然是friend class）
 *         可以通过调用Player的public及protected函数来实现
 */
class PlayerImp
{
public:
	PlayerImp(Player& player)
		: player_(player)
	{ }

	virtual ~PlayerImp() { }

	virtual void OnGetAllData()       { }
	virtual void OnEnterWorld()       { }
	virtual void OnLeaveWorld()       { }
	virtual void OnHeartbeat()        { }
	virtual void OnChangeGsComplete() { }
	virtual void OnWorldClosing(const MSG& msg) { } 
	virtual void OnKillMonster(int32_t monster_tid, int32_t monster_num) { }

	virtual int  OnMessageHandler(const MSG& msg)
    { 
        LOG_ERROR << "PlayerImp has no msg_handler for msg:" << msg.message; 
        return 0; 
    }


protected:
	Player&    player_;
};

#include "player-inl.h"

} // namespace gamed

#endif // GAMED_GS_PLAYER_PLAYER_H_
