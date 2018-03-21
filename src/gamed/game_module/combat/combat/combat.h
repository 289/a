#ifndef __GAME_MODULE_COMBAT_H__
#define __GAME_MODULE_COMBAT_H__

#include <map>
#include <vector>

#include "object.h"
#include "combat_atb.h"
#include "combat_unit.h"
#include "player_cmd.h"
#include "combat_buffer.h"
#include "combat_def.h"
#include "combat_types.h"
#include "combat_state.h"

#include "shared/base/mutex.h"
#include "shared/base/callback_bind.h"
#include "shared/net/packet/proto_packet.h"

namespace shared {
namespace net {

class ProtoPacket;

}; //namespace shared
}; //namespace net

namespace combat
{

class Object;
class CombatATB;
class CombatUnit;
class CombatNpc;
class CombatPlayer;
class CombatControl;
class CombatBuffer;
class CombatSceneAI;
class CombatState;

///
/// 战斗基类
///
class Combat : public Object
{
protected:
    typedef int32_t MapID;
    typedef int64_t WorldID;
    typedef int32_t CombatSceneID;
    typedef std::set<CombatNpc*>       CombatPetSet;
    typedef std::set<CombatNpc*>       GolemSet;
    typedef std::vector<CombatUnit*>   CombatUnitVec;
    typedef std::vector<PlayerCMD>     CMDVec;

    enum
    {
        PARTY_INVALID,
        PARTY_ATTACKER,
        PARTY_DEFENDER,
    };

    template<class T>
        struct unit_finder
        {
            UnitID unit_id;
            unit_finder(UnitID id): unit_id(id) {}
            bool operator() (const T* obj) const
            {
                if (!obj) return false;
                return obj->GetID() == unit_id;
            }
        };

    MapID                   map_id_;                     // 地图ID
    WorldID                 world_id_;                   // 地图gs-ID
    CombatSceneID           scene_id_;                   // 战斗场景ID
    CombatUnitVec           attacker_list_;              // 攻方列表
    CombatUnitVec           defender_list_;              // 守方列表
    CombatUnitVec           recycle_list_;               // 战斗对象不使用后的回收列表
    CombatPetSet            pet_set_;                    // 战场所有宠物的集合
    GolemSet                golem_set_;                  // 战场所有魔偶的集合
    CombatSceneAI*          scene_ai_;                   // 场景策略脚本控制器
    CombatATB               atb_man_;                    // 战场ATB管理器
    CombatBuffer            combat_buffer_;              // 战场BUFF缓存器
    CombatState             combat_state_;               // 战场状态控制器
    RoleID                  creator_roleid_;             // 战场创建者的角色ID
    int32_t                 taskid_;                     // 战场对应的任务ID
    int32_t                 challengeid_;                // 战场对应的界面挑战组ID
    int32_t                 tick_counter_;               // 战场TICK计数
    int32_t                 round_counter_;              // 战场总回合计数
    bool                    is_boss_combat_;             // 是否是BOSS战斗
    bool                    attacker_win_;               // 标记是否攻方取胜
    bool                    attacker_sneaked_;           // 攻方被偷袭
    bool                    defender_sneaked_;           // 守方被偷袭
    int32_t                 force_combat_result_;        // 特殊战斗里的强制战斗结果

    // 用于将同时出手的单位错开，目前至少错开2个tick
    UnitID                  last_action_unit_[2];       // 上一次出手的对象
    uint32_t                last_action_tick_[2];       // 上一次出手的Tick

    CMDVec                  cmd_vec_;

public:
    static CombatSenderCallBack       CombatSenderCB;    // 战斗发送消息回调
    static CombatPVEResultCallBack    CombatPVEResultCB; // 同步PVE战斗结果回调
    static CombatPVPResultCallBack    CombatPVPResultCB; // 同步PVP战斗结果回调
    static CombatStartCallBack        CombatStartCB;     // 战斗开始回调
    static CombatPVEEndCallBack       CombatPVEEndCB;    // PVE战斗结束回调
    static CombatPVPEndCallBack       CombatPVPEndCB;    // PVP战斗结束回调
    static WorldBossStatusCallBack    WorldBossStatusCB; // 世界BOSS战斗状态回调

    static void SetCombatCMDSenderCallBack(const CombatSenderCallBack& cb);
    static void SetCombatPVEResultCallBack(const CombatPVEResultCallBack& cb);
    static void SetCombatPVPResultCallBack(const CombatPVPResultCallBack& cb);
    static void SetCombatStartCallBack(const CombatStartCallBack& cb);
    static void SetCombatPVEEndCallBack(const CombatPVEEndCallBack& cb);
    static void SetCombatPVPEndCallBack(const CombatPVPEndCallBack& cb);
    static void SetWorldBossStatusCallBack(const WorldBossStatusCallBack& cb);

public:
    Combat(char type);
    virtual ~Combat();

    friend class CombatUnit;
    friend class CombatNpc;
    friend class CombatPlayer;
    friend class CombatState;

public:
    bool Init(WorldID world_id, CombatSceneID combat_scene_id);
    bool AddCombatUnit(CombatUnit* unit, RoleID role_to_join=0);
    bool RmvCombatUnit(CombatUnit* unit);
    bool AddTeamNpc(TemplID npc_id, int npc_pos, CombatPlayer* player);
    bool StartCombat();
    void CombatEnd();
    void OnRoundStart(UnitID unit_id);
    void OnRoundEnd(UnitID unit_id);
    void AssertLocked();
    void Suspend(int msec);
    void WaitSelectSkill(int32_t skill_index, int msec, int talk_id);
    void WaitSelectPet(int32_t pet_index, int msec, int talk_id);
    bool Terminate();
    void MessageHandler(const MSG& msg);
    void HeartBeat();
    void Clear();
    void BuildBuffSyncData(shared::net::ProtoPacket&);

    virtual bool OnInit() {return false;}
    virtual void OnStartCombat(){}
    virtual void OnCombatStart(){}
    virtual void OnCombatEnd(){}
    virtual void OnSomeoneDead(UnitID deader, UnitID killer){}
    virtual void OnHeartBeat(){}
    virtual void BroadCastCMD(shared::net::ProtoPacket&, UnitID ex_id=0){}
    virtual bool TryCloseCombat() {return true;}
    virtual bool CanJoinCombat() const {return false;}
    virtual void DoCombatEnd(){}
    virtual void BuildCombatData(shared::net::ProtoPacket&){}
    virtual bool OnMessageHandler(const MSG& msg) {return false;}
    virtual void Trace() const;

    ///
    /// 查询接口
    ///
    CombatUnit* Find(UnitID unit_id);
    CombatUnit* Find(const XID& xid);
    const CombatUnit* Find(UnitID unit_id) const;
    const CombatUnit* Find(const XID& xid) const;
    CombatPlayer* QueryPlayer(RoleID roleid);
    int LocateParty(UnitID unit_id) const;
    CombatUnit* FindKiller(UnitID unit_id);
    UnitID GetOwnerID(UnitID unit_id);

    ///
    /// 战场基础信息接口
    ///
    UnitID  object_id() const;
    CombatSceneAI* scene_ai() const;
    MapID   GetMapID() const;
    WObjID  GetWorldObjectID() const;
    UnitID  GetCombatID() const;
    TemplID GetCombatSceneID() const;
    int32_t GetTickCounter() const;
    int32_t GetMobCount(bool mob = true) const;
    int32_t GetRoundCount() const;
    int32_t GetChallengeID() const;
    void    SetCreator(RoleID roleid);
    void    SetTaskID(int32_t taskid);
    void    SetChallengeID(int32_t challengid);
    void    GetTeamMate(UnitID unit_id, CombatUnitVec& list) const;
    void    GetEnemy(UnitID unit_id, CombatUnitVec& list) const;
    void    GetTeamMate(const XID& id, CombatUnitVec& list) const;
    void    GetEnemy(const XID& id, CombatUnitVec& list) const;
    void    GetGolem(CombatUnitVec& list) const;
    int     GetEnemiesAlive(const XID& xid) const;
    int     GetTeammatesAlive(const XID& xid) const;
    bool    IsEnemy(UnitID uid1, UnitID uid2) const;
    bool    IsTeammate(UnitID uid1, UnitID uid2) const;
    bool    HasSomeoneAction() const;
    void    GetMobList(std::vector<UnitID>& mob_list) const;
    void    GetRoleClsList(std::vector<int>& role_cls_list) const;
    void    AssertZombie(); //test

    ///
    /// 战场状态相关
    ///
    bool IsStateOpen() const;
    bool IsStateClose() const;
    bool IsStateRunning() const;
    bool IsStateSuspend() const;
    bool IsStateWaitSelectSkill() const;
    bool IsStateWaitSelectPet() const;

    ///
    /// 战斗结束相关
    ///
    int32_t IsCombatEnd();
    bool TestCombatEnd();
    bool TestCloseCombat();
    void ForceCombatResult(bool attacker_win);

    ///
    /// 错开出手间隔
    ///
    bool NeedReviseATB(CombatUnit* unit);
    void SetAction(CombatUnit* unit, uint32_t cur_tick);

    ///
    /// MSG发送接口
    ///
    void SendMSG(const MSG& msg, size_t tick_latency=0);
    void SendMSG(const ObjectVec& list, const MSG& msg, size_t tick_latency=0);
    void SendMSG(const XID* first, const XID* last, const MSG& msg, size_t tick_latency=0);

    ///
    /// MSG-Handler
    ///
    void MsgHandler_SomeOneDead(const MSG& msg);
    void MsgHandler_CombatEnd(const MSG& msg);

    ///
    /// 广播战斗结果
    ///
    void BroadcastSkillResult(CombatUnit* pattacker, const SkillDamageVec& damages);
    void BroadcastBuffResult(const skill::BuffDamageVec& damages);
    void BroadcastBuffData();

    ///
    /// 测试操作许可
    ///
    bool CanRegisterATB() const;
    bool CanUnRegisterATB() const;
    bool CanPlayerOperate() const;
    bool CanScriptOperate() const;
    bool CanStopCombat() const;
    bool CanSuspendCombat() const;
    bool CanWaitSelectSkill() const;
    bool CanWaitSelectPet() const;

    ///
    /// 战斗ATB相关
    ///
    void RegisterATB(CombatUnit* unit);
    void RegisterInstantATB(CombatUnit* unit);
    void ResetATB(CombatUnit* unit);
    void RegisterAllATB();
    void UnRegisterATB(UnitID unit_id);
    void UnRegisterAllATB();
    void ActivateATB();
    void DeActivateATB(int tick);
    void OnATBChange(CombatUnit* unit);

    ///
    /// 玩家CMD相关
    ///
    bool HandleCMD(const PlayerCMD& cmd);

    ///
    /// 战斗Buff相关
    ///
    inline void AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher, UnitID target);
    inline void DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher, UnitID target);

    ///
    /// 偷袭相关
    ///
    inline void SetAttackerSneaked();
    inline void SetDefenderSneaked();
    inline void ClrSneakedState();

    ///
    /// 宠物相关
    ///
    inline void AddCombatPet(CombatNpc* pet);
    inline void RmvCombatPet(CombatNpc* pet);
    CombatNpc* QueryCombatPet(UnitID unit_id);

    ///
    /// 魔偶相关
    ///
    inline void AddGolem(CombatNpc* golem);
    inline void RmvGolem(CombatNpc* golem);
    CombatNpc* QueryGolem(UnitID unit_id);

    ///
    /// BOSS战相关
    ///
    inline bool IsWorldBossCombat() const;
    inline void SetWorldBossCombat(bool flag);

protected:
    CombatNpc* CreateCombatNpc(const NpcInfo& npc, char npc_type);
    bool InsertCombatUnit(CombatUnitVec& list, CombatUnit* unit);
    bool DeleteCombatUnit(CombatUnitVec& list, CombatUnit* unit);
    void DoBroadCastCMD(CombatUnitVec& list, shared::net::ProtoPacket&, UnitID ex_id=0);
    void HeartBeatAllUnits();
    void ClearAllUnits();
    void HandleDelayCMD();
    static void RemoveUnitFromList(CombatUnitVec& list, bool clear);
};


///
/// PVE战斗类
///
class CombatPVE : public Combat
{
private:
    TemplID mob_group_id_;               // 当前怪物组ID
    TemplID next_mob_group_id_;          // 下一波怪物组ID
    int16_t wave_counter_;               // 连续战斗中的第几波
    int16_t wave_total_;                 // 连续战斗时总共有多少波
    UnitID  world_boss_id_;              // 被触发怪物(BOSS)在战斗系统中的object_id
    int32_t world_monster_id_;           // 被触发怪物在大世界的object_id
    int32_t init_player_count_;          // 战斗开始时玩家个数

    typedef std::map<TemplID, int> KilledMobMap;
    KilledMobMap killed_mob_map_;

public:
    CombatPVE();
    ~CombatPVE();

    void SetInitPlayerCount(int32_t player_count);
    void SetMobGroupId(TemplID mob_group_id);
    void SetWorldMonsterID(int32_t world_monster_object_id);
    void SetWorldBossID(UnitID world_boss_id);
    void OnMobKilled(TemplID mob_tid, int32_t money, int32_t exp, const std::vector<ItemEntry>& items, UnitID killer);
    int32_t GetWorldBossID() const;
    CombatNpc* GetBoss(int pos);
    void FindMob(TemplID mob_tid, std::vector<CombatUnit*>& list);
    bool IsMonster(UnitID unit_id);
    void Clear();

    virtual bool OnInit();
    virtual void OnStartCombat();
    virtual void OnHeartBeat();
    virtual void OnCombatStart();
    virtual void OnCombatEnd();
    virtual void OnSomeoneDead(UnitID deader, UnitID killer);
    virtual bool OnMessageHandler(const MSG& msg);
    virtual void BroadCastCMD(shared::net::ProtoPacket&, UnitID ex_id=0);
    virtual bool TryCloseCombat();
    virtual bool CanJoinCombat() const;
    virtual void DoCombatEnd();
    virtual void BuildCombatData(shared::net::ProtoPacket&);
    virtual void Trace() const;

    ///
    /// MSG-Handler
    ///
    void MsgHandler_CombatContinue(const MSG& msg);
    void MsgHandler_ResumeSceneScript(const MSG& msg);
    void MsgHandler_TriggerTransform(const MSG& msg);
    void MsgHandler_StartTransform(const MSG& msg);
    void MsgHandler_TriggerEscape(const MSG& msg);
    void MsgHandler_StartEscape(const MSG& msg);

    ///
    /// 为场景脚本提供
    ///
    void MobSpeak(UnitID mob_uid, int id_talk, int msec);
    void MultiMobSpeak(UnitID mob_tid, int id_talk, int msec);
    void PlayerSpeak(int id_talk, int msec);
    void PlayerActivateSkill(int id_skgrp, int lvl_skgrp, int msec);
    void SummonMob(int mob_type, TemplID mob_tid, int pos);
    void CastInstantSkill(TemplID mob_tid, SkillID skill_id);
    void Shake(int x_amplitude, int y_amplitude, int duration, int interval);
    int  GetUnitProp(int party, int pos, int prop, int& value);
    int  GetUnitType(UnitID uid);
    //void GetMobList(std::vector<UnitID>& mob_list) const;
    //void GetRoleClsList(std::vector<int>& role_cls_list) const;
};

///
/// PVP战斗类
///
class CombatPVP : public Combat
{
private:
    int32_t combat_flag_;

public:
    CombatPVP();
    ~CombatPVP();

    void SetCombatFlag(int flag);
    void Clear();

    virtual bool OnInit();
    virtual void OnStartCombat();
    virtual void OnHeartBeat();
    virtual void OnCombatStart();
    virtual void OnCombatEnd();
    virtual void OnSomeoneDead(UnitID deader, UnitID killer);
    virtual bool OnMessageHandler(const MSG& msg);
    virtual bool CanJoinCombat() const;
    virtual void DoCombatEnd();
    virtual void BuildCombatData(shared::net::ProtoPacket&);
    virtual void BroadCastCMD(shared::net::ProtoPacket&, UnitID ex_id=0);
    virtual void Trace() const;
};

#include "combat.inl"

};

#endif // __GAME_MODULE_COMBAT_H__
