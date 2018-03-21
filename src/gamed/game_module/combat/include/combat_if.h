#ifndef __GAME_MODULE_COMBAT_IF_H__
#define __GAME_MODULE_COMBAT_IF_H__

#include "combat_def.h"
#include "combat_types.h"
#include "combat_param.h"

namespace combat
{

class Combat;
class CombatUnit;
class CombatPlayer;
struct PlayerCMD;

///
/// 设置GS回调的函数
///
void SetCombatCMDSenderCallBack(const CombatSenderCallBack& cb);
void SetCombatPVEResultCallBack(const CombatPVEResultCallBack& cb);
void SetCombatPVPResultCallBack(const CombatPVPResultCallBack& cb);
void SetCombatStartCallBack(const CombatStartCallBack& cb);
void SetCombatPVEEndCallBack(const CombatPVEEndCallBack& cb);
void SetCombatPVPEndCallBack(const CombatPVPEndCallBack& cb);
void SetWorldBossStatusCallBack(const WorldBossStatusCallBack& cb);

///
/// 战斗系统对外提供的接口
///
bool Initialize(std::vector<int>& mapid_vec, std::string& npc_policy_path, const std::vector<cls_prop_sync_rule>& list);
void HeartBeat();
void Release();

bool CreatePVECombat(const pve_combat_param& param, ObjectID& combat_id, UnitID& unit_id);
bool CreatePVPCombat(const pvp_combat_param& param, ObjectID& combat_id, UnitID& unit_id);
bool JoinCombat(RoleID roleid, const player_data& data, RoleID role_to_join, ObjectID combat_id, UnitID& unit_id);
bool DispatchCmd(const PlayerCMD& cmd, ObjectID combat_id, UnitID unit_id);

int QueryCombatType(CombatID combat_id);
Combat* QueryCombat(CombatID combat_id);
CombatUnit* QueryCombatUnit(UnitID unit_id);
CombatPlayer* FindPlayer(RoleID roleid);

};

#endif // __GAME_MODULE_COMBAT_IF_H__
