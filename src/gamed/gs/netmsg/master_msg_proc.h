#ifndef GAMED_GS_NETMSG_MASTER_MSG_PROC_H_
#define GAMED_GS_NETMSG_MASTER_MSG_PROC_H_

#include "shared/base/singleton.h"
#include "shared/base/thread.h"
#include "shared/net/protobuf/dispatcher_proto.h"

#include "gs/netio/queue_msg_type.h"

#include "dispatcher_queued_msg.h"

// 协议的前向声明放在下面这个文件，cpp里才include对应协议的.h文件
#include "proto_forward_declare.inl"


namespace gamed {

using namespace shared;
using namespace shared::net;
using namespace common::protocol;

class Player;

namespace world {
	struct instance_info;
    struct battleground_info;
} // namespace world


/**
 * @brief 处理master发过来的消息
 */
class MasterMsgProcess : public shared::Singleton<MasterMsgProcess>
{
	friend class shared::Singleton<MasterMsgProcess>;
public:
	static inline MasterMsgProcess* GetInstance() {
		return &(get_mutable_instance());
	}

	network::MasterRecvQueue* get_recv_queue();

	bool    StartProcThread();
	void    StopProcThread();


protected:
	MasterMsgProcess();
	~MasterMsgProcess();

	static void* ThreadFunc(void*);
	void    RunInThread();
	void    HandleMasterMessage(network::QueuedRecvMsg& msg);

	///
    /// proto process func
	///
	void    StartupRegisterHandler();
	void    ProtoDefaultHandler(const MessageRef, const WrapRef);
	void    ServerStatusNotify(const global::ServerStatusNotify&, const WrapRef);
	void    MasterOnConnected(const global::ConnectEstablished&, const WrapRef);
	void    MasterConnDestroyed(const global::ConnectDestroyed&, const WrapRef);
	void    RpcMessageProcess(const shared::net::RpcMessage&, const WrapRef);
	void    AnnounceMasterInfo(const M2G::AnnounceMasterInfo&, const WrapRef);
	void    PlayerEnterWorld(const M2G::PlayerEnterWorld&, const WrapRef);
	void    PlayerChangeMap(const M2G::PlayerChangeMap&, const WrapRef);
	void    PlayerChangeMapError(const M2G::PlayerChangeMapError&, const WrapRef);
	void	QueryItemInfo(const M2G::QueryItemInfo&, const WrapRef);
	void	GetMailAttachRe(const M2G::GetMailAttach_Re&, const WrapRef);
	void	DeleteMailRe(const M2G::DeleteMail_Re&, const WrapRef);
	void	SendMailRe(const M2G::SendMail_Re&, const WrapRef);
	void	ReceiveNewMail(const M2G::AnnounceNewMail&, const WrapRef);
	// team proto process
	void    JoinTeam(const M2G::JoinTeam&, const WrapRef);
	void    LeaveTeam(const M2G::LeaveTeam&, const WrapRef);
	void    ChangeTeamLeader(const M2G::ChangeTeamLeader&, const WrapRef);
	void    ChangeTeamPos(const M2G::ChangeTeamPos&, const WrapRef);
	void    TeamInfo(const M2G::TeamInfo&, const WrapRef);
	void    QueryTeamMember(const M2G::QueryTeamMember&, const WrapRef);
	void    JoinTeamReq(const M2G::JoinTeamReq&, const WrapRef);
    void    TeamLeaderConvene(const M2G::TeamLeaderConvene&, const WrapRef);
	// instance proto
	void    EnterInsReply(const M2G::EnterInsReply&, const WrapRef);
	void    TeammemberLocalInsReply(const M2G::TeammemberLocalInsReply&, const WrapRef);
	void    TeamLeaderStartLocalIns(const M2G::TeamLeaderStartLocalIns&, const WrapRef);
	void    TeammemberQuitLocalIns(const M2G::TeammemberQuitLocalIns&, const WrapRef);
	void    TeamLocalInsInvite(const M2G::TeamLocalInsInvite&, const WrapRef);
    void    TeamCrossInsInvite(const M2G::TeamCrossInsInvite&, const WrapRef);
    void    TeamCrossInsInviteReply(const M2G::TeamCrossInsInviteReply&, const WrapRef);
    // battleground
    void    EnterBGReply(const M2G::EnterBGReply&, const WrapRef);
	// global counter
	void    GlobalCounterInfo(const M2G::GlobalCounterInfo&, const WrapRef);
	void    ModifyGlobalCounter(const M2G::ModifyGlobalCounter&, const WrapRef);
    void    SetGlobalCounter(const M2G::SetGlobalCounter&, const WrapRef);
	// global data
	void    GlobalDataChange(const global::GlobalDataChange&, const WrapRef);
    // auction
    void    AuctionInvalidQuery(const M2G::AuctionInvalidQuery&, const WrapRef);
    // punch card
    void    RePunchCardHelp(const global::RePunchCardHelp&, const WrapRef);
    void    RePunchCardHelpReply(const global::RePunchCardHelpReply&, const WrapRef);
    void    RePunchCardHelpError(const global::RePunchCardHelpError&, const WrapRef);


private:
    ///
    /// 副本相关
    ///
	bool    HandleEnterInstanceMap(const global::InstanceInfo& insInfo, Player* pPlayer, world::instance_info& ret_insinfo);
	bool    CreateInstanceMap(const global::InstanceInfo& ins_info, world::instance_info& ret_insinfo);
	bool    FindInstanceMap(const global::InstanceInfo& ins_info, world::instance_info& ret_insinfo);
	bool    InstanceEnterWorld(int32_t masterid, const M2G::PlayerEnterWorld& proto, Player* pPlayer, 
							   int32_t& des_world_id, int32_t& des_world_tag);
	bool    InstanceChangeMap(int32_t masterid, const M2G::PlayerChangeMap& proto, 
							  Player* pPlayer, int32_t& des_world_tag);
	void    SyncTeamInfo(const M2G::TeamInfo&, Player* pPlayer);
	void    SyncInsTeamInfo(const global::InstanceInfo& ins_info, Player* pPlayer);

    ///
    /// 战场相关
    ///
    bool    HandleEnterBattleGroundMap(const global::BattleGroundInfo& bgInfo, Player* pPlayer, world::battleground_info& ret_bginfo);
	bool    CreateBattleGroundMap(const global::BattleGroundInfo& bg_info, world::battleground_info& ret_bginfo);
	bool    FindBattleGroundMap(const global::BattleGroundInfo& bg_info, world::battleground_info& ret_bginfo);
    bool    BattleGroundEnterWorld(int32_t masterid, const M2G::PlayerEnterWorld& proto, Player* pPlayer, 
						  	       int32_t& des_world_id, int32_t& des_world_tag);
    bool    BattleGroundChangeMap(int32_t masterid, const M2G::PlayerChangeMap& proto, 
							      Player* pPlayer, int32_t& des_world_tag);
    void    SyncBGTeamInfo(const global::BattleGroundInfo& bg_info, Player* pPlayer);

    ///
    /// 处理函数
    ///
    bool    NonnormalEnterWorld(int32_t masterid, const M2G::PlayerEnterWorld& msg, Player* pPlayer, 
							    int32_t& des_world_id, int32_t& des_world_tag);
    bool    NonnormalChangeMap(int32_t masterid, const M2G::PlayerChangeMap& proto, 
							   Player* pPlayer, int32_t des_world_id, int32_t& des_world_tag);
    void    RedirectEnterWorld(int32_t masterid, const M2G::PlayerEnterWorld& proto, Player* pPlayer);


private:
	static network::MasterRecvQueue recv_queue_;

	Thread                proc_thread_;
	QueuedMsgDispatcher   dispatcher_;

	bool    is_done_;
};

#define s_pMasterMsgProc gamed::MasterMsgProcess::GetInstance()

} // namespace gamed

#endif // GAMED_GS_NETMSG_MASTER_MSG_PROC_H_
