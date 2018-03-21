#ifndef GAMED_GS_PLAYER_PLAYER_SENDER_H_
#define GAMED_GS_PLAYER_PLAYER_SENDER_H_

#include <stdint.h>
#include <vector>

#include "shared/base/types.h"
#include "gs/global/math_types.h"
#include "gs/netmsg/send_to_link.h"

#include "gamed/client_proto/G2C_proto.h"

#include "player_def.h"


namespace shared 
{
	namespace net 
	{
		class Buffer;
		class ProtoPacket;
	} // namespace net
} // namespace shared

namespace common 
{
	class PlayerData;
} // namespace common

namespace masterRpc
{
	class ChangeNameProxyResponse;
} // namespace masterRpc


namespace gamed {

class Slice;
class Player;
struct itemdata;

class PlayerSender
{
	typedef shared::net::ProtoPacket& PacketRef;
public:
	PlayerSender(Player& player);
	~PlayerSender();

    // movement
	void    MoveStart(const A2DVECTOR& dest, uint16_t speed, uint8_t mode);
	void    Move(const A2DVECTOR& cur_pos, const A2DVECTOR& dest, uint16_t speed, uint8_t mode);
	void    MoveStop(const A2DVECTOR& pos, uint16_t speed, uint8_t mode, uint8_t dir);
	void    PullBackToValidPos(uint16_t move_seq, const A2DVECTOR& pos);
	void    BroadcastPlayerPullBack(const A2DVECTOR& pos);
	void    NotifyPlayerMoveProp(float run_speed);
	void    NotifySelfPos();

	void    ElsePlayerEnterView(RoleID playerid, int32_t link_id, int32_t sid_in_link);
	void    ElsePlayerLeaveView(RoleID id);
	void    ObjectEnterView(XID xid, int32_t tid, int32_t eid, uint8_t dir, const A2DVECTOR& pos);
	void    ObjectLeaveView(int64_t id);
	void    PlayerEnterEventArea();
	void    EnterRuleArea(int32_t elem_id);
	void    LeaveRuleArea(int32_t elem_id);

    // 通用接口
	void    SendCmd(PacketRef packet) const;
	void    BroadCastCmd(PacketRef packet) const;
	void    ErrorMessage(int err_no) const;
    void    ErrorMessageToOther(RoleID roleid, int32_t linkid, int32_t sid_in_link, int err_no) const;
	
	// 物品相关
	void    GetAllInventory();
	void    GainItem(int where, int type, int amount, 
                     int expire_date, size_t last_idx, int last_amount, int gain_mode, 
                     const void* content, size_t content_len, uint16_t content_crc);
	void    SelfItemData(int where, const itemdata& data, int update_type);
	void    SelfItemListBrief(int where, int inv_cap, const std::vector<itemdata>& list);
	void    SelfItemListDetail(int where, int inv_cap, const std::vector<itemdata>& list);
	void    DropInvItem(int type, size_t index, int count);
	void    MoveInvItem(size_t dest, size_t src, int count);
	void    SellInvItem(int8_t where, size_t index, int count);
	void    ExchangeInvItem(size_t index1, size_t index2);
	void    EquipItem(size_t idx_inv, size_t idx_equip);
	void    UndoEquip(size_t idx_inv, size_t idx_equip);
	void    UseItem(int where, int type, size_t index, int count);
    void    UseItemError(G2C::UseItemErrorCode err_num, int type);
	void    RefineEquipReply(int where, const itemdata& data);
	void	ElsePlayerQueryItem(int8_t result, const itemdata& data, int64_t query_roleid, int32_t linkid, int32_t sid_in_link);
    void    LevelUpCard(int8_t result, int8_t lvlup_type);

	// 金钱相关
	void    GetOwnMoney();
	void    GainMoney(int64_t money);
	void    SpendMoney(int64_t money);

	// 学分相关
	void	GetOwnScore();
	void    GainScore(int32_t score);
	void    SpendScore(int32_t score);

	// 冷去相关
	void    SetCoolDown(int cd_index, int cd_interval);

	// 玩家信息更新
	void    PlayerBaseInfo();
	void    PlayerGainExp(int32_t exp);
	void    PlayerLevelUp();
	void    PlayerVolatileInfo();
	void    PlayerExtendProp();
	void    PlayerDead();
	void    PlayerResurrect();
	//void    PlayerTransferCls();
	//void	PlayerTransferGender();
	void    PlayerVisibleInfoChange(int64_t visible_mask, const std::vector<int32_t>& visible_list);
    void    PlayerUpdateTitle(int32_t title_id);

	// 战斗
	void    EnterCombat(int32_t combat_id);
	void    LeaveCombat();
	void    JoinCombatFail(int32_t combat_id);
	void    TeammateEnterCombatFail(int64_t teammate_roleid);

	// 技能
	void    LearnSkill_Re(int skill_idx);
	void    SwitchSkill_Re(int skill_idx);
	void    UpdateSkillTree(int sk_tree_id, int skill_idx, int lvl_cur, int lvl_tmp, bool is_active);

	// Npc
	void    NpcGreeting(const XID& xid);

	// 组队
	void    TeamMemberQueryRe(RoleID query_roleid, int32_t linkid, int32_t sid_in_link);
	void    JoinTeamReq(RoleID requester, const std::string& first_name, const std::string& mid_name, const std::string& last_name, int32_t invite);

	// 副本组队
	void    MapTeamQueryMemberRe(RoleID query_roleid, int32_t linkid, int32_t sid_in_link);
	
	// 任务
	void    TaskNotifyClient(uint16_t type, const void* buf, size_t size);

	// 成就
	void    AchieveNotifyClient(uint16_t type, const void* buf, size_t size);

	// 传送
	void    TransferPrepare(int32_t elem_id);
	void    MapKickoutCountdown(int32_t secs);

	// matter矿相关
	void    GatherStart(int64_t matter_id, int32_t use_time, int32_t gather_seq_no, int32_t mattertid);
	void    GatherStop(int64_t matter_id, int8_t reason, int32_t mattertid);

	// 大世界Buff
	void    UpdateVisibleBuff(const std::vector<PlayerVisibleState::BuffInfo>& buff_vec);

	// 瞄类视觉
	void    CatVisionGainExp(int32_t exp);
	void    CatVisionLevelUp();
	void    CatVisionOpen_Re(int result);
	void    CatVisionClose_Re(int result);
	void    CatVisionClose();

	///
	/// 发生给其它玩家的接口
	///
	void    ElsePlayerQueryExtProp(RoleID else_playerid, int32_t link_id, int32_t sid_in_link);
	void    ElsePlayerQueryEquipCRC(RoleID else_playerid, int32_t link_id, int32_t sid_in_link);
	void    ElsePlayerQueryEquipment(RoleID else_playerid, int32_t link_id, int32_t sid_in_link);
	void    ElsePlayerQueryStaticRoleInfo(RoleID else_playerid, int32_t link_id, int32_t sid_in_link);

	///
	/// 黑白名单接口
	///
	void    ObjectBWList(const playerdef::ObjectBWList& obj_bw_list);
	void    ObjectBWListChange(int32_t templ_id, bool is_black, bool is_add);

	///
	/// 玩家改名字
	///
	void    ChangeNameRe(const masterRpc::ChangeNameProxyResponse& response, bool is_timeout);

	///
	/// 玩家角色信息
	///
	void    SelfStaticRoleInfo();

	///
	/// 副本相关
	///
	void    PlayerEnterInsMap(int32_t ins_create_time, int32_t ins_templ_id);
    void    PlayerLeaveInsMap(int32_t ins_templ_id);
	void    GetInstanceDataRe(int32_t ins_templ_id, bool is_cross_realm);

    ///
    /// 战场相关
    ///
	void    PlayerEnterBGMap(int32_t bg_create_time, int32_t bg_templ_id);
    void    PlayerLeaveBGMap(int32_t bg_templ_id);
    void    GetBattleGroundDataRe(int32_t bg_templ_id, bool is_cross_realm);

	///
	/// 商店购物 
	///
	void    ShopShoppingFailed(int ret, int32_t goods_id, int32_t goods_count, int32_t goods_remains);

    ///
    /// 决斗相关
    ///
    void    PlayerDuelPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet);

    ///
    /// 附魔相关
    ///
    void    EnhanceReply(int32_t err, int8_t slot_index, int32_t enhance_id, int32_t count);

    ///
    /// 签到
    ///
    void    PunchCardPacket(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet);

    ///
	/// 发往master的接口
	///
	void    SendToMaster(const shared::net::ProtobufCodec::MessageRef proto) const;
	void    PlayerDataSendToMaster(common::PlayerData* playerdata);
	void    DBSaveErrorLogout();
	void    PlayerFatalError();
	void    LogoutErrorOccurred();
	void    QueryTeamInfo();
	bool    PlayerChangeMap(int32_t target_world_id, const A2DVECTOR& pos, const shared::net::Buffer& playerdata);
	void    JoinTeam(RoleID other_roleid);
	void    JoinTeamRes(bool invite, bool accept, RoleID requester);
	void    LeaveTeam();
    void    ConveneTeammate();
	void	SendChatMsg(int8_t channel, int64_t sender, int64_t receiver, const std::string& sender_name, const std::string& msg);
	void    SendCmdByMaster(int64_t roleid, PacketRef packet) const; // 通过master中转发给别的玩家，需要确定该玩家和发送者是同服的


protected:
	void    SendToSelf(PacketRef packet) const;
	void    AutoBroadcastCSMsg(PacketRef packet) const;
	void    SendToOther(RoleID roleid, int32_t linkid, int32_t sid_in_link, PacketRef packet) const; 


private:
	Player& player_;

	typedef std::vector<NetToLink::MulticastPlayerInfo> ElsePlayerVec;
	typedef std::map<int32_t, ElsePlayerVec> ElsePlayersInFieldOfView;
	ElsePlayersInFieldOfView else_players_in_view_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_PLAYER_SENDER_H_
