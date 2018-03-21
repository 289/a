#include "master_msg_proc.h"

#include "shared/logsys/logging.h"
#include "shared/net/protobuf/rpc/rpc.pb.h"

#include "gs/netio/netio_if.h"
#include "gs/netmsg/send_to_link.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/global/global_counter.h"
#include "gs/global/msg_pack_def.h"
#include "gs/global/global_data.h"
#include "gs/scene/world_cluster.h"
#include "gs/scene/world_man.h"
#include "gs/item/item_data.h"
#include "gs/player/player_sender.h"
#include "gs/player/player.h"

#include "send_to_master.h"

// client packet
#include "gamed/client_proto/G2C_proto.h"

// recv proto
#include "common/protocol/gen/global/connection_status.pb.h"
#include "common/protocol/gen/global/server_status.pb.h"
#include "common/protocol/gen/global/global_data_change.pb.h"
#include "common/protocol/gen/global/player_punch_card.pb.h"
#include "common/protocol/gen/M2G/player_enter_world.pb.h"
#include "common/protocol/gen/M2G/team_msg.pb.h"
#include "common/protocol/gen/M2G/player_change_map.pb.h"
#include "common/protocol/gen/M2G/instance_msg.pb.h"
#include "common/protocol/gen/M2G/announce_master_info.pb.h"
#include "common/protocol/gen/M2G/global_counter.pb.h"
#include "common/protocol/gen/M2G/mail_msg.pb.h"
#include "common/protocol/gen/M2G/auction_msg.pb.h"
#include "common/protocol/gen/M2G/query_msg.pb.h"
#include "common/protocol/gen/M2G/battleground_msg.pb.h"

// send proto
#include "common/protocol/gen/G2M/announce_gs_info.pb.h"
#include "common/protocol/gen/G2M/player_abnormal_logout.pb.h"
#include "common/protocol/gen/G2M/player_change_map.pb.h"
#include "common/protocol/gen/G2M/team_msg.pb.h"
#include "common/protocol/gen/G2M/instance_msg.pb.h"
#include "common/protocol/gen/G2M/query_msg.pb.h"
#include "common/protocol/gen/G2L/player_enterworld_re.pb.h"


namespace gamed {

using namespace shared;

namespace 
{
	enum EnterWorldErr
	{
		CHANGE_MAP_ERR = 1,
		PLAYER_ALLOC_ERR,
		WORLD_NOT_IN_THIS_GS,
		PLAYER_POS_INVALID,
		UNKNOWN_ERR,
		NONMAP_ENTER_WORLD_ERR, // 非普通地图enter_world失败
	};

	void tidy_up_failure(EnterWorldErr errno_code, int32_t masterid, int64_t roleid, Player* pPlayer)
	{
		switch (errno_code)
		{
			case CHANGE_MAP_ERR:
				// 不发送PlayerAbnormalLogout，因为已经发送change map err
				GLog::log("玩家%ld切换地图失败！", roleid);
				break;

			case NONMAP_ENTER_WORLD_ERR:
				// 不发送PlayerAbnormalLogout，因为已经发送RedirectEnterWorld
				GLog::log("玩家%ld EnterWorld登陆非普通地图失败！", roleid);
				break;

			case PLAYER_ALLOC_ERR:
				GLog::log("用户%ld登录gs失败，在CreateModuleInstance()失败！", roleid);
				NetToMaster::PlayerAbnormalLogout(masterid, roleid, G2M::PlayerAbnormalLogout::PLAYER_ALLOC_ERR);
				break;

			case WORLD_NOT_IN_THIS_GS:
				GLog::log("用户%ld登录gs失败，地图不在这个gs上！", roleid);
				NetToMaster::PlayerAbnormalLogout(masterid, roleid, G2M::PlayerAbnormalLogout::WORLD_NOT_IN_THIS_GS);
				break;

			case PLAYER_POS_INVALID:
				GLog::log("用户%ld登录gs失败，无效位置！", roleid);
				NetToMaster::PlayerAbnormalLogout(masterid, roleid, G2M::PlayerAbnormalLogout::PLAYER_POS_INVALID);
				break;

			case UNKNOWN_ERR:
				GLog::log("用户%ld登录gs失败，发生未知错误！", roleid);
				NetToMaster::PlayerAbnormalLogout(masterid, roleid, G2M::PlayerAbnormalLogout::UNKNOWN_ERR);
				break;
			
			default:
				break;	
		}

		if (pPlayer != NULL)
		{
			Gmatrix::FreePlayer(pPlayer);
		}
		return;
	}

	inline std::string getProtoClassName(const std::string& full_name)
	{
		std::string tmpstr;
		std::size_t found = full_name.find_last_of(".");
		tmpstr = full_name.substr(found+1, full_name.length());
		return tmpstr;
	}

	enum PCreateType
	{
        CREATE_TYPE_NONE = 0,
		PLAYER_ENTER_WORLD,
		PLAYER_CHANGE_MAP,
		PLAYER_CHANGE_MAP_ERROR,
	};

	struct PlayerCreateInfo
	{
        PlayerCreateInfo()
            : create_type(CREATE_TYPE_NONE),
              master_id(-1),
              user_id(-1),
              role_id(-1),
              player_content(NULL),
              content_len(-1),
              link_id(-1),
              client_sid_in_link(-1)
        { }

		PCreateType create_type;
		int32_t     master_id;
        int64_t     user_id;
		RoleID      role_id;
		const char* player_content;
		int32_t     content_len;
		int32_t     link_id;
		int32_t     client_sid_in_link;
	};

	bool AllocLockedPlayer(Player*& pPlayer, const PlayerCreateInfo& info)
	{
		ASSERT(pPlayer == NULL);
		
		if (NULL != Gmatrix::FindPlayerFromMan(info.role_id))
		{
			__PRINTF("玩家%ld已经登陆，AllocLockedPlayer失败! createType:%d", info.role_id, info.create_type);
			G2L::PlayerEnterWorldAgain send_packet;
			send_packet.set_roleid(info.role_id);
			send_packet.set_client_sid_in_link(info.client_sid_in_link);
			NetToLink::SendProtocol(info.link_id, send_packet);
			return false;
		}

		pPlayer = Gmatrix::AllocPlayer(info.role_id);
		if (NULL == pPlayer)
		{
			return false;
		}

		// ---- lock pPlayer ----
		pPlayer->Lock();
		// 创建模块实例，比如controller，此函数需要在InsertPlayerToMan之前调用
		if (!pPlayer->InitRuntimeModule(info.role_id))
		{
			return false;
		}

		int ret = pPlayer->InitPlayerData(info.role_id, info.player_content, info.content_len);
		if (0 != ret)
		{
			return false;
		}

		if (pPlayer->role_id() != info.role_id)
		{
			return false;
		}

		pPlayer->set_link_id(info.link_id);
		pPlayer->set_sid_in_link(info.client_sid_in_link);
		pPlayer->set_master_id(info.master_id);
        pPlayer->set_user_id(info.user_id);
		return true;
	}

	class AutoUnlockPlayer
	{
	public:
		explicit AutoUnlockPlayer(Player* player)
			: player_(player)
		{
			ASSERT(player_->IsLocked());
		}

		~AutoUnlockPlayer()
		{
			player_->Unlock();
		}

	private:
		Player* player_;
	};
	#define AutoUnlockPlayer(x) SHARED_STATIC_ASSERT(false); // missing guard var name

	void go_back_to_src_map(int32_t masterid, int64_t userid, const global::PlayerChangeMapData& data, int32_t err_param)
	{
		ASSERT(IS_NORMAL_MAP(data.src_mapid()));
		G2M::PlayerChangeMapReply reply;
		reply.set_errcode(G2M::PlayerChangeMapReply::FAILURE);
		reply.set_errparam(err_param);
        reply.set_userid(userid);
		reply.mutable_data()->CopyFrom(data);
		NetToMaster::SendProtocol(masterid, reply);
	}

	void change_map_error(int32_t masterid, const M2G::PlayerChangeMap& proto, int32_t err_param)
	{
		go_back_to_src_map(masterid, proto.userid(), proto.data(), err_param);
	}

    void set_map_team_info(WorldManager* pManager, const M2G::MapTeamInfo& proto)
    {
        shared::net::ByteBuffer tmpbuf;
        tmpbuf.resize(proto.ByteSize());
        if (!proto.SerializeToArray((void*)tmpbuf.contents(), tmpbuf.size())) 
        {
            LOG_ERROR << "PlayerChangeMap M2G::MapTeamInfo Serialize error! size:" << tmpbuf.size();
            return;
        }

        pManager->SetMapTeamInfo((void*)tmpbuf.contents(), tmpbuf.size());
    }

} // namespace Anonymous


network::MasterRecvQueue MasterMsgProcess::recv_queue_;

void* MasterMsgProcess::ThreadFunc(void* pdata)
{
	Assert(pdata);
	MasterMsgProcess* pmasterproc = static_cast<MasterMsgProcess*>(pdata);
	pmasterproc->RunInThread();
	LOG_INFO << "Master msg process thread exit!";

	return NULL;
}

MasterMsgProcess::MasterMsgProcess()
	: proc_thread_(MasterMsgProcess::ThreadFunc, this),
	  dispatcher_(BIND_MEM_CB(&MasterMsgProcess::ProtoDefaultHandler, this)),
	  is_done_(false)
{
	StartupRegisterHandler();
}

MasterMsgProcess::~MasterMsgProcess()
{
	ASSERT(recv_queue_.empty());
}

bool MasterMsgProcess::StartProcThread()
{
	proc_thread_.Start();
	return true;
}

void MasterMsgProcess::StopProcThread()
{
	QueuedRecvMsg tmp;
	global::ServerStatusNotify* stop_msg = new global::ServerStatusNotify();
	stop_msg->set_type(global::ServerInfo::GAMESERVER_SVR);
	stop_msg->set_status(global::ServerInfo::SVR_SHUTDOWN);
	tmp.msg_ptr = stop_msg;
	recv_queue_.put(tmp); // FIXME: will invoke ProtoDefaultHandler(), and could be crash

	proc_thread_.Join();
}

void MasterMsgProcess::ProtoDefaultHandler(const MessageRef msg, const WrapRef wrap)
{
	LOG_ERROR << "MasterMsgProcess Error recv UnRegister pb msg [name: " << msg.GetTypeName() << "]";
}

network::MasterRecvQueue* MasterMsgProcess::get_recv_queue()
{
	return &recv_queue_;
}

void MasterMsgProcess::RunInThread()
{
	std::deque<network::QueuedRecvMsg> tmpList;
	while (!is_done_)
	{
		// locked inside
		NetIO::TakeAllMasterData(tmpList);
		while (!tmpList.empty())
		{
			network::QueuedRecvMsg& msg = tmpList.front();
			HandleMasterMessage(msg);
			SAFE_DELETE(msg.msg_ptr);
			tmpList.pop_front();
		}
	}
}

void MasterMsgProcess::HandleMasterMessage(QueuedRecvMsg& msg)
{
	dispatcher_.OnQueuedMessage(*msg.msg_ptr, msg);
}

#define REGISTER_PROC(protocol, func) \
	dispatcher_.Register<protocol>(BIND_MEM_CB(&func, this));
void MasterMsgProcess::StartupRegisterHandler()
{
	REGISTER_PROC(global::ServerStatusNotify, MasterMsgProcess::ServerStatusNotify);
	REGISTER_PROC(global::ConnectEstablished, MasterMsgProcess::MasterOnConnected);
	REGISTER_PROC(global::ConnectDestroyed, MasterMsgProcess::MasterConnDestroyed);
	REGISTER_PROC(global::GlobalDataChange, MasterMsgProcess::GlobalDataChange);
    REGISTER_PROC(global::RePunchCardHelp, MasterMsgProcess::RePunchCardHelp);
    REGISTER_PROC(global::RePunchCardHelpReply, MasterMsgProcess::RePunchCardHelpReply);
    REGISTER_PROC(global::RePunchCardHelpError, MasterMsgProcess::RePunchCardHelpError);
	REGISTER_PROC(shared::net::RpcMessage, MasterMsgProcess::RpcMessageProcess);
	REGISTER_PROC(M2G::AnnounceMasterInfo, MasterMsgProcess::AnnounceMasterInfo);
	REGISTER_PROC(M2G::PlayerEnterWorld, MasterMsgProcess::PlayerEnterWorld);
	REGISTER_PROC(M2G::JoinTeam, MasterMsgProcess::JoinTeam);
	REGISTER_PROC(M2G::LeaveTeam, MasterMsgProcess::LeaveTeam);
	REGISTER_PROC(M2G::ChangeTeamLeader, MasterMsgProcess::ChangeTeamLeader);
	REGISTER_PROC(M2G::ChangeTeamPos, MasterMsgProcess::ChangeTeamPos);
	REGISTER_PROC(M2G::TeamInfo, MasterMsgProcess::TeamInfo);
	REGISTER_PROC(M2G::QueryTeamMember, MasterMsgProcess::QueryTeamMember);
	REGISTER_PROC(M2G::JoinTeamReq, MasterMsgProcess::JoinTeamReq);
    REGISTER_PROC(M2G::TeamLeaderConvene, MasterMsgProcess::TeamLeaderConvene);
	REGISTER_PROC(M2G::PlayerChangeMap, MasterMsgProcess::PlayerChangeMap);
	REGISTER_PROC(M2G::PlayerChangeMapError, MasterMsgProcess::PlayerChangeMapError);
	REGISTER_PROC(M2G::EnterInsReply, MasterMsgProcess::EnterInsReply);
	REGISTER_PROC(M2G::TeammemberLocalInsReply, MasterMsgProcess::TeammemberLocalInsReply);
	REGISTER_PROC(M2G::TeamLeaderStartLocalIns, MasterMsgProcess::TeamLeaderStartLocalIns);
	REGISTER_PROC(M2G::TeammemberQuitLocalIns, MasterMsgProcess::TeammemberQuitLocalIns);
	REGISTER_PROC(M2G::TeamLocalInsInvite, MasterMsgProcess::TeamLocalInsInvite);
    REGISTER_PROC(M2G::TeamCrossInsInvite, MasterMsgProcess::TeamCrossInsInvite);
    REGISTER_PROC(M2G::TeamCrossInsInviteReply, MasterMsgProcess::TeamCrossInsInviteReply);
	REGISTER_PROC(M2G::GlobalCounterInfo, MasterMsgProcess::GlobalCounterInfo);
	REGISTER_PROC(M2G::ModifyGlobalCounter, MasterMsgProcess::ModifyGlobalCounter);
    REGISTER_PROC(M2G::SetGlobalCounter, MasterMsgProcess::SetGlobalCounter);
	REGISTER_PROC(M2G::QueryItemInfo, MasterMsgProcess::QueryItemInfo);
	REGISTER_PROC(M2G::GetMailAttach_Re, MasterMsgProcess::GetMailAttachRe);
	REGISTER_PROC(M2G::DeleteMail_Re, MasterMsgProcess::DeleteMailRe);
	REGISTER_PROC(M2G::SendMail_Re, MasterMsgProcess::SendMailRe);
	REGISTER_PROC(M2G::AnnounceNewMail, MasterMsgProcess::ReceiveNewMail);
    REGISTER_PROC(M2G::AuctionInvalidQuery, MasterMsgProcess::AuctionInvalidQuery);
    REGISTER_PROC(M2G::EnterBGReply, MasterMsgProcess::EnterBGReply);
}
#undef REGISTER_PROC

void MasterMsgProcess::ServerStatusNotify(const global::ServerStatusNotify& msg, const WrapRef wrap)
{
	if (msg.type() == global::ServerInfo::GAMESERVER_SVR)
	{
		if (msg.status() == global::ServerInfo::SVR_SHUTDOWN)
		{
			is_done_ = true;
			LOG_WARN << "GS shutdown id:" << s_pGmatrix->GetGameServerID()
				<< " master process thread quit";
		}
	}
}

void MasterMsgProcess::MasterOnConnected(const global::ConnectEstablished& msg, const WrapRef wrap)
{
	if (global::ConnectionInfo::MASTER_CONN != msg.type()) return;

	// gs info send to master
	G2M::AnnounceGSInfo announce;
	announce.set_gsid(Gmatrix::GetGameServerID());

	std::vector<gmatrixdef::WorldIDInfo> world_vec;
	Gmatrix::GetAllWorldInCharge(world_vec);
	for (size_t i = 0; i < world_vec.size(); ++i)
	{
		G2M::AnnounceGSInfo::WorldInfo* world_info = announce.add_world_info();
		world_info->set_world_id(world_vec[i].world_id);
		world_info->set_world_tag(world_vec[i].world_tag);
	}

	NetIO::SendToMasterBySid(wrap.sendback_sid, announce);
}

void MasterMsgProcess::MasterConnDestroyed(const global::ConnectDestroyed& msg, const WrapRef wrap)
{
	if (global::ConnectionInfo::MASTER_CONN != msg.type()) 
    {
        return;
    }

    if (msg.masterid() > 0)
    {
	    s_pGmatrix->MasterDisconnect(msg.masterid());
	    NetIO::RemoveMasterIdMapToSid(msg.masterid());
    }
}

void MasterMsgProcess::RpcMessageProcess(const shared::net::RpcMessage& msg, const WrapRef wrap)
{
	if (gmatrixdef::RPCCALL_TYPE_PLAYER == msg.caller_type())
	{
		Player* pplayer = Gmatrix::FindPlayerFromMan(msg.caller_id());
		if (pplayer != NULL)
		{
			PlayerLockGuard lock(pplayer);
			if (pplayer->IsActived())
			{
				pplayer->MasterRpcMessageProc(msg);
			}
		}
	}
}

void MasterMsgProcess::AnnounceMasterInfo(const M2G::AnnounceMasterInfo& msg, const WrapRef wrap)
{
	// set master id
	NetIO::SetMasterIdMapToSid(msg.master_id(), wrap.sendback_sid);

    // set svrid to masterid map
    Gmatrix::SvrIdMapToMasterId(msg.master_id(), msg.master_id());
    for (int i = 0; i < msg.server_in_charge_size(); ++i)
    {
        Gmatrix::SvrIdMapToMasterId(msg.server_in_charge(i), msg.master_id());
    }
}

bool MasterMsgProcess::InstanceEnterWorld(int32_t masterid, 
		                                  const M2G::PlayerEnterWorld& proto, 
										  Player* pPlayer, 
										  int32_t& des_world_id, 
										  int32_t& des_world_tag)
{
	ASSERT(proto.ins_info().ins_serial_num() != 0);
	ASSERT(IS_INS_MAP(proto.ins_info().world_id()));
	ASSERT(proto.ins_info().enter_type() == common::protocol::global::ET_INS_REENTER);

	world::instance_info ret_info;
	playerdef::InsInfoCond tmpinfo;
	tmpinfo.enter_type  = playerdef::InsInfoCond::REENTER;
	tmpinfo.serial_num  = proto.ins_info().ins_serial_num();
	tmpinfo.create_time = proto.ins_info().ins_create_time();
	tmpinfo.ins_tid     = proto.ins_info().ins_templ_id();

	bool is_success = true;
	if (!pPlayer->CheckInstanceCond(tmpinfo))
	{
		is_success = false;
	}
	else // check pass
	{
		if (!HandleEnterInstanceMap(proto.ins_info(), pPlayer, ret_info))
		{
			is_success = false;
		}
	}

	if (!is_success)
	{
		pPlayer->ResetInstanceInfo();
        return false;
	}

	//
	// success
	//
	pPlayer->SetInstanceInfo(ret_info);
	
	// src pos
	pPlayer->SetSpecSavePos(pPlayer->world_id(), pPlayer->pos());

	// ins pos
	A2DVECTOR ins_pos;
	ASSERT(pPlayer->GetInstancePos(ins_pos));
	pPlayer->set_pos(ins_pos);

	des_world_id  = ret_info.world_id;
	des_world_tag = ret_info.world_tag;
	ASSERT(pPlayer->ConsumeInstanceCond(ret_info.ins_templ_id, false));
	return true;
}

bool MasterMsgProcess::BattleGroundEnterWorld(int32_t masterid, 
                                              const M2G::PlayerEnterWorld& proto, 
                                              Player* pPlayer, 
						  	                  int32_t& des_world_id, 
                                              int32_t& des_world_tag)
{
    ASSERT(proto.bg_info().bg_serial_num() != 0);
	ASSERT(IS_BG_MAP(proto.bg_info().world_id()));
	ASSERT(proto.bg_info().enter_type() == common::protocol::global::ET_BG_REENTER);

	world::battleground_info ret_info;
	playerdef::BGInfoCond tmpinfo;
	tmpinfo.enter_type  = playerdef::BGInfoCond::REENTER;
	tmpinfo.serial_num  = proto.bg_info().bg_serial_num();
	tmpinfo.create_time = proto.bg_info().bg_create_time();
	tmpinfo.bg_tid      = proto.bg_info().bg_templ_id();

	bool is_success = true;
	if (!pPlayer->CheckBattleGroundCond(tmpinfo))
	{
		is_success = false;
	}
	else // check pass
	{
		if (!HandleEnterBattleGroundMap(proto.bg_info(), pPlayer, ret_info))
		{
			is_success = false;
		}
	}

	if (!is_success)
	{
		pPlayer->ResetBattleGroundInfo();
        return false;
	}

	//
	// success
	//
	pPlayer->SetBattleGroundInfo(ret_info);
	
	// src pos
	pPlayer->SetSpecSavePos(pPlayer->world_id(), pPlayer->pos());

	// bg pos
	A2DVECTOR bg_pos;
	ASSERT(pPlayer->GetBattleGroundPos(bg_pos));
	pPlayer->set_pos(bg_pos);

	des_world_id  = ret_info.world_id;
	des_world_tag = ret_info.world_tag;
	ASSERT(pPlayer->ConsumeBattleGroundCond(ret_info.bg_templ_id));
	return true;
}

void MasterMsgProcess::RedirectEnterWorld(int32_t masterid, const M2G::PlayerEnterWorld& proto, Player* pPlayer)
{
    shared::net::Buffer databuf;
    if (!pPlayer->GetPlayerDataContent(&databuf))
    {
        GLog::log("Player:%ld RedirectEnterWorld GetPlayerDataContent() error!", proto.roleid());
        return;
    }
    G2M::RedirectEnterWorld re_enterworld;
    re_enterworld.set_des_world_id(pPlayer->world_id());
    re_enterworld.set_roleid(proto.roleid());
    re_enterworld.set_link_id(proto.link_id());
    re_enterworld.set_client_sid_in_link(proto.client_sid_in_link());
    re_enterworld.set_content(databuf.peek(), databuf.ReadableBytes());
    re_enterworld.set_userid(proto.userid());

    // always at last
    tidy_up_failure(NONMAP_ENTER_WORLD_ERR, masterid, proto.roleid(), pPlayer);
    NetToMaster::SendProtocol(masterid, re_enterworld);
}

bool MasterMsgProcess::NonnormalEnterWorld(int32_t masterid, 
                                           const M2G::PlayerEnterWorld& msg, 
                                           Player* pPlayer, 
							               int32_t& des_world_id, 
                                           int32_t& des_world_tag)
{
    bool ret = true;

    ///
    /// 先尝试进战场，因为副本是可以出去后重复进入的
    ///
    // battleground map
    if (msg.has_bg_info())
    {
        // battleground team
        SyncBGTeamInfo(msg.bg_info(), pPlayer);

        if (!BattleGroundEnterWorld(masterid, msg, pPlayer, des_world_id, des_world_tag))
		{
			GLog::log("玩家%ld战场enterworld失败！", msg.roleid());
            ret = false;
		}
    }

    // instance map
	if (msg.has_ins_info())
	{
		// ins team
		SyncInsTeamInfo(msg.ins_info(), pPlayer);

		if (!InstanceEnterWorld(masterid, msg, pPlayer, des_world_id, des_world_tag))
		{
			GLog::log("玩家%ld副本enterworld失败！", msg.roleid());
            ret = false;
		}
	}

    return ret;
}

void MasterMsgProcess::PlayerEnterWorld(const M2G::PlayerEnterWorld& msg, const WrapRef wrap)
{
	int32_t masterid = NetIO::GetMasterIdBySid(wrap.sendback_sid);
	if (-1 == masterid)
	{
		__PRINTF("Master sid[%d]找不到对应的master_id，玩家%ld无法登陆！", wrap.sendback_sid, msg.roleid());
		return;
	}

	PlayerCreateInfo info;
	info.create_type        = PLAYER_ENTER_WORLD;
	info.master_id          = masterid;
    info.user_id            = msg.userid();
	info.role_id            = msg.roleid();
	info.player_content     = msg.content().c_str();
	info.content_len        = msg.content().size();
	info.link_id            = msg.link_id();
	info.client_sid_in_link = msg.client_sid_in_link();

	Player* pPlayer = NULL;
	if (!AllocLockedPlayer(pPlayer, info))
	{
        GLog::log("PlayerEnterWorld - AllocLockedPlayer() error! roleid=%ld", msg.roleid());
		if (pPlayer != NULL)
		{
			AutoUnlockPlayer tmp_unlock(pPlayer);
			tidy_up_failure(PLAYER_ALLOC_ERR, masterid, msg.roleid(), pPlayer);
		}
		return;
	}

	///
	/// auto unlock
	///
	AutoUnlockPlayer auto_unlock(pPlayer);

	// check team info
	if (msg.has_team_info())
	{
		SyncTeamInfo(msg.team_info(), pPlayer);
	}

	int32_t des_world_id  = WID_TO_MAPID(pPlayer->world_id());
	int32_t des_world_tag = 0;
    
    // non normal map
    if (!NonnormalEnterWorld(masterid, msg, pPlayer, des_world_id, des_world_tag))
    {
        GLog::log("玩家%ld登陆非正常地图enterworld失败！", msg.roleid());
        RedirectEnterWorld(masterid, msg, pPlayer);
        return;
    }
	
	// find world
	WorldManager* pManager = Gmatrix::FindWorldManager(des_world_id, des_world_tag);
	if (NULL == pManager)
	{
		tidy_up_failure(WORLD_NOT_IN_THIS_GS, masterid, msg.roleid(), pPlayer);
		return;
	}
	else 
	{
        // 检查地图人数上限
        if (!IS_NORMAL_MAP(des_world_id) && 
            !pManager->CheckPlayerCountLimit(world::BGET_ENTER_WORLD))
        {
            GLog::log("玩家%ld登陆非正常地图enterworld失败！地图超过人数上限！", msg.roleid());
            RedirectEnterWorld(masterid, msg, pPlayer);
            return;
        }

        // 插入地图
		if (pManager->InsertPlayer(pPlayer) < 0)
		{
			tidy_up_failure(UNKNOWN_ERR, masterid, msg.roleid(), pPlayer);
			return;
		}
	}

	///
	/// enter world has success!
	///
	
	// 将玩家放入管理器，可以开始接收心跳MSG
	Gmatrix::InsertPlayerToMan(pPlayer);
	
	// send enterworld success to link
	NetToLink::PlayerEnterWorldReply(pPlayer->link_id(), pPlayer->role_id(), pPlayer->sid_in_link());
	
	// send enterworld_re to client
	G2C::EnterWorld_Re enterre_to_client;
	enterre_to_client.map_id                  = pPlayer->world_id();
	enterre_to_client.self_info.roleid        = pPlayer->role_id();
	enterre_to_client.self_info.pos           = pPlayer->pos();
	enterre_to_client.self_info.equip_crc     = pPlayer->equip_crc();
	enterre_to_client.self_info.dir           = pPlayer->dir();
	enterre_to_client.self_info.role_class    = pPlayer->role_class();
	enterre_to_client.self_info.gender        = pPlayer->gender();
	enterre_to_client.self_info.level         = pPlayer->level();
	enterre_to_client.self_info.weapon_id     = pPlayer->GetWeaponID();
	enterre_to_client.self_info.visible_state = pPlayer->visible_state();
	NetToLink::SendS2CGameData(pPlayer->link_id(), 
			                   pPlayer->role_id(), 
				   		       pPlayer->sid_in_link(), 
						       enterre_to_client);

    // EnterWorld里会做一些处理，所以应该放在G2C::EnterWorld_Re之后
    // 保证客户端先收到G2C::EnterWorld_Re
	pPlayer->EnterWorld();
	
	// enter world success
	GLog::log("玩家 %ld 登录世界 world_id:%d", msg.roleid(), pPlayer->world_id());
	return;
}

bool MasterMsgProcess::InstanceChangeMap(int32_t masterid, 
		                                 const M2G::PlayerChangeMap& proto, 
										 Player* pPlayer, 
										 int32_t& des_world_tag)
{
	ASSERT(proto.has_ins_info());

	world::instance_info ret_info;
	bool is_create_ins = false;
	playerdef::InsInfoCond tmpinfo;
	if (proto.ins_info().enter_type() == common::protocol::global::ET_INS_CREATE)
	{
		tmpinfo.enter_type = playerdef::InsInfoCond::CREATE;
		is_create_ins = true;
	}
	else if (proto.ins_info().enter_type() == common::protocol::global::ET_INS_REENTER)
	{
		tmpinfo.enter_type = playerdef::InsInfoCond::REENTER;
	}
	else
	{
		tmpinfo.enter_type = playerdef::InsInfoCond::ENTER;
	}
	tmpinfo.serial_num  = proto.ins_info().ins_serial_num();
	tmpinfo.create_time = proto.ins_info().ins_create_time();
	tmpinfo.ins_tid     = proto.ins_info().ins_templ_id();

	bool is_success    = true;
	if (!pPlayer->CheckInstanceCond(tmpinfo))
	{
		is_success = false;
	}
	else // check pass
	{
		if (!HandleEnterInstanceMap(proto.ins_info(), pPlayer, ret_info))
		{
			is_success = false;
		}
	}

	if (!is_success)
	{
		pPlayer->ResetInstanceInfo();
        return false;
	}

	//
	// success
	//
	pPlayer->SetInstanceInfo(ret_info);

	// src pos
	A2DVECTOR src_pos(proto.ins_info().src_pos_x(), proto.ins_info().src_pos_y());
	pPlayer->SetSpecSavePos(proto.ins_info().src_world_id(), src_pos);

	// ins pos
	A2DVECTOR ins_pos;
	ASSERT(pPlayer->QueryInsEntrancePos(ins_pos));
	pPlayer->set_pos(ins_pos);

	des_world_tag = ret_info.world_tag;
	ASSERT(pPlayer->ConsumeInstanceCond(ret_info.ins_templ_id, is_create_ins));
	return true;
}

bool MasterMsgProcess::BattleGroundChangeMap(int32_t masterid, 
		                                     const M2G::PlayerChangeMap& proto, 
										     Player* pPlayer, 
										     int32_t& des_world_tag)
{
	ASSERT(proto.has_bg_info());

	world::battleground_info ret_info;
	playerdef::BGInfoCond tmpinfo;
	tmpinfo.enter_type  = playerdef::BGInfoCond::ENTER;
	tmpinfo.serial_num  = proto.bg_info().bg_serial_num();
	tmpinfo.create_time = proto.bg_info().bg_create_time();
	tmpinfo.bg_tid      = proto.bg_info().bg_templ_id();

	bool is_success = true;
	if (!pPlayer->CheckBattleGroundCond(tmpinfo))
	{
		is_success = false;
	}
	else // check pass
	{
		if (!HandleEnterBattleGroundMap(proto.bg_info(), pPlayer, ret_info))
		{
			is_success = false;
		}
	}

	if (!is_success)
	{
		pPlayer->ResetBattleGroundInfo();
        return false;
	}

	//
	// success
	//
	pPlayer->SetBattleGroundInfo(ret_info);

	// src pos
	A2DVECTOR src_pos(proto.bg_info().src_pos_x(), proto.bg_info().src_pos_y());
	pPlayer->SetSpecSavePos(proto.bg_info().src_world_id(), src_pos);

	// bgos
	A2DVECTOR bg_pos;
	ASSERT(pPlayer->QueryBGEntrancePos(bg_pos));
	pPlayer->set_pos(bg_pos);

	des_world_tag = ret_info.world_tag;
	ASSERT(pPlayer->ConsumeBattleGroundCond(ret_info.bg_templ_id));
	return true;
}

bool MasterMsgProcess::NonnormalChangeMap(int32_t masterid, 
                                          const M2G::PlayerChangeMap& proto, 
							              Player* pPlayer, 
                                          int32_t des_world_id, 
                                          int32_t& des_world_tag)
{
    // instance map
	if (IS_INS_MAP(des_world_id))
	{
		// ins team
		SyncInsTeamInfo(proto.ins_info(), pPlayer);

		if (!InstanceChangeMap(masterid, proto, pPlayer, des_world_tag))
		{
			GLog::log("玩家%ld副本ChangeMap失败！", proto.data().roleid());
			return false;
		}
	}
    else if (IS_BG_MAP(des_world_id)) // battleground map
    {
        // bg team
        SyncBGTeamInfo(proto.bg_info(), pPlayer);

        if (!BattleGroundChangeMap(masterid, proto, pPlayer, des_world_tag))
        {
            GLog::log("玩家%ld战场ChangeMap失败！", proto.data().roleid());
            return false;
        }
    }
    return true;
}

void MasterMsgProcess::PlayerChangeMap(const M2G::PlayerChangeMap& proto, const WrapRef wrap)
{
	int32_t masterid = NetIO::GetMasterIdBySid(wrap.sendback_sid);
	if (-1 == masterid)
	{
		__PRINTF("Master sid[%d]找不到对应的master_id，玩家%ld切换地图！", 
				wrap.sendback_sid, proto.data().roleid());
		return;
	}

	PlayerCreateInfo info;
	info.create_type        = PLAYER_CHANGE_MAP;
	info.master_id          = masterid;
    info.user_id            = proto.userid();
	info.role_id            = proto.data().roleid();
	info.player_content     = proto.data().content().c_str();
	info.content_len        = proto.data().content().size();
	info.link_id            = proto.data().link_id();
	info.client_sid_in_link = proto.data().client_sid_in_link();

	Player* pPlayer = NULL;
	if (!AllocLockedPlayer(pPlayer, info))
	{
	    GLog::log("PlayerChangeMap - AllocLockedPlayer() error! roleid:%ld", proto.data().roleid());
		if (pPlayer != NULL)
		{
			AutoUnlockPlayer tmp_unlock(pPlayer);
			tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
		}
		return change_map_error(masterid, proto, -1);
	}

	///
	/// auto unlock
	///
	AutoUnlockPlayer auto_unlock(pPlayer);

	// check team info
	if (proto.has_team_info())
	{
		SyncTeamInfo(proto.team_info(), pPlayer);
	}
	
	int32_t des_world_id  = WID_TO_MAPID(proto.data().des_mapid());
	int32_t des_world_tag = 0;

	// double check
	ASSERT(proto.data().src_mapid() == pPlayer->world_id());
	ASSERT((int)proto.data().src_x() == (int)pPlayer->pos().x);
	ASSERT((int)proto.data().src_y() == (int)pPlayer->pos().y);
    
    // non normal map
    if (!NonnormalChangeMap(masterid, proto, pPlayer, des_world_id, des_world_tag))
    {
        GLog::log("玩家%ld非普通地图ChangeMap失败！world_id:%d", proto.data().roleid(), des_world_id);
        tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
		return change_map_error(masterid, proto, -14);
    }
		
	// destination
	A2DVECTOR des_pos;
    if (IS_NORMAL_MAP(des_world_id))
    {
	    des_pos.x = proto.data().des_x();
	    des_pos.y = proto.data().des_y();
    }
    else
    {
        des_pos = pPlayer->pos();
    }
	
	// 找到对应的world
	WorldManager* pManager = Gmatrix::FindWorldManager(des_world_id, des_world_tag);
	if (NULL == pManager)
	{
		__PRINTF("玩家%ld 换地图切gs失败，reason：地图在本gs没有找到world_id:%d", 
				proto.data().roleid(), des_world_id);
		tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
		return change_map_error(masterid, proto, -2);
	}
	else 
	{
		// 检查位置是否合法
		if (!pManager->PosInWorld(des_pos) || !pManager->IsWalkablePos(des_pos))
		{
			__PRINTF("玩家%ld 换地图切gs失败，reason：位置是不可达点或不在World范围内，world_id:%d des_pos(%f %f)", 
					proto.data().roleid(), des_world_id, des_pos.x, des_pos.y);
			tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
			return change_map_error(masterid, proto, -3);
		}

        // 检查地图人数上限
        if (!IS_NORMAL_MAP(des_world_id) && 
            !pManager->CheckPlayerCountLimit(world::BGET_CHANGE_MAP))
        {
            GLog::log("玩家%ld非普通地图ChangeMap失败！超地图人数上限！world_id:%d", proto.data().roleid(), des_world_id);
            tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
            return change_map_error(masterid, proto, -17);
        }

        // 是否带了地图队伍
        if (proto.has_map_team_info())
        {
            set_map_team_info(pManager, proto.map_team_info());
        }

		// 插入地图
		if (pManager->InsertPlayer(pPlayer) < 0)
		{
			tidy_up_failure(CHANGE_MAP_ERR, masterid, proto.data().roleid(), pPlayer);
			return change_map_error(masterid, proto, -4);
		}
	}

	///
	/// change map success!
	///
	
	// send enterworld success to link
	NetToLink::RedirectPlayerGS(pPlayer->link_id(), pPlayer->role_id());
	
	// 将玩家放入管理器，可以开始接收心跳MSG
	Gmatrix::InsertPlayerToMan(pPlayer);

	// 更新为新位置，并通知客户端
	pPlayer->set_pos(des_pos);
	pPlayer->NotifySelfPos();
	pPlayer->EnterWorld();
	pPlayer->ChangeGsComplete();
	
	// reply master
	G2M::PlayerChangeMapReply reply;
    reply.set_userid(proto.userid());
	reply.set_errcode(G2M::PlayerChangeMapReply::SUCCESS);
	NetToMaster::SendProtocol(masterid, reply);

    // change map log
	GLog::log("玩家 %ld 切换地图成功 world_id:%d des_x:%f des_y:%f", pPlayer->role_id(), pPlayer->world_id(), des_pos.x, des_pos.y);
	return;
}

void MasterMsgProcess::PlayerChangeMapError(const M2G::PlayerChangeMapError& proto, const WrapRef wrap)
{
	GLog::log("玩家%ld传送失败-传回原图，target_world_id:%d, target_posx:%f, target_posy:%f, source_world_id:%d, errparam:%d",
			proto.data().roleid(), proto.data().des_mapid(), proto.data().des_x(), proto.data().des_y(), proto.data().src_mapid(), proto.errparam());
	
	int32_t masterid = NetIO::GetMasterIdBySid(wrap.sendback_sid);
	if (-1 == masterid)
	{
		__PRINTF("Master sid[%d]找不到对应的master_id，玩家%ld收到PlayerChangeMapError！", wrap.sendback_sid, proto.data().roleid());
		return;
	}

	PlayerCreateInfo info;
	info.create_type        = PLAYER_CHANGE_MAP_ERROR;
	info.master_id          = masterid;
    info.user_id            = proto.userid();
	info.role_id            = proto.data().roleid();
	info.player_content     = proto.data().content().c_str();
	info.content_len        = proto.data().content().size();
	info.link_id            = proto.data().link_id();
	info.client_sid_in_link = proto.data().client_sid_in_link();

	Player* pPlayer = NULL;
	if (!AllocLockedPlayer(pPlayer, info))
	{
		__PRINTF("PlayerChangeMapError - AllocLockedPlayer() error!");
		if (pPlayer != NULL)
		{
			AutoUnlockPlayer tmp_unlock(pPlayer);
			tidy_up_failure(PLAYER_ALLOC_ERR, masterid, proto.data().roleid(), pPlayer);
		}
		return;
	}

	///
	/// auto unlock
	///
	AutoUnlockPlayer auto_unlock(pPlayer);

	int32_t des_world_id  = WID_TO_MAPID(pPlayer->world_id());
	int32_t des_world_tag = 0;

	// check team info
	if (proto.has_team_info())
	{
		SyncTeamInfo(proto.team_info(), pPlayer);
	}

	// double check
	ASSERT(proto.data().src_mapid() == des_world_id);
	ASSERT((int)proto.data().src_x() == (int)pPlayer->pos().x);
	ASSERT((int)proto.data().src_y() == (int)pPlayer->pos().y);
	ASSERT(!IS_INS_MAP(proto.data().src_mapid())); // ChangeMapError不可能是传回副本地图

	// destination
	A2DVECTOR des_pos(pPlayer->pos());
	
	// 找到对应的world
	WorldManager* pManager = Gmatrix::FindWorldManager(des_world_id, des_world_tag);
	if (NULL == pManager)
	{
		__PRINTF("玩家%ld 回原地图失败，reason：地图在本gs没有找到world_id:%d", 
				pPlayer->role_id(), des_world_id);
		tidy_up_failure(WORLD_NOT_IN_THIS_GS, masterid, proto.data().roleid(), pPlayer);
		return;
	}
	else 
	{
		// 检查位置是否合法
		if (!pManager->PosInWorld(des_pos) || !pManager->IsWalkablePos(des_pos))
		{
			__PRINTF("玩家%ld 回原地图切失败，reason：位置是不可达点或不在World范围内，world_id:%d des_pos(%f %f)", 
					pPlayer->role_id(), des_world_id, des_pos.x, des_pos.y);
			tidy_up_failure(PLAYER_POS_INVALID, masterid, proto.data().roleid(), pPlayer);
			return;
		}

		// 插入地图
		if (pManager->InsertPlayer(pPlayer) < 0)
		{
			tidy_up_failure(UNKNOWN_ERR, masterid, proto.data().roleid(), pPlayer);
			return;
		}
	}

	///
	/// change map success!
	///
	
	// send enterworld success to link
	NetToLink::RedirectPlayerGS(pPlayer->link_id(), pPlayer->role_id());
	
	// 将玩家放入管理器，可以开始接收心跳MSG
	Gmatrix::InsertPlayerToMan(pPlayer);

	// 更新为新位置，并通知客户端
	pPlayer->ChangeMapError(true);
	pPlayer->NotifySelfPos();
	pPlayer->EnterWorld();
	pPlayer->ChangeGsComplete();

	return;
}

bool MasterMsgProcess::CreateInstanceMap(const global::InstanceInfo& ins_info, world::instance_info& ret_insinfo)
{
	if (!wcluster::CreateInsWorld(ins_info.world_id(), 
				                  ins_info.ins_serial_num(), 
				                  ins_info.ins_templ_id(), 
				                  ret_insinfo))
	{
		return false;
	}
	return true;
}

bool MasterMsgProcess::CreateBattleGroundMap(const global::BattleGroundInfo& bg_info, world::battleground_info& ret_bginfo)
{
    if (!wcluster::CreateBGWorld(bg_info.world_id(),
                                 bg_info.bg_serial_num(),
                                 bg_info.bg_templ_id(),
                                 ret_bginfo))
    {
        return false;
    }
    return true;
}

bool MasterMsgProcess::FindInstanceMap(const global::InstanceInfo& ins_info, world::instance_info& ret_insinfo)
{
	world::instance_info tmpinfo;
	tmpinfo.ins_serial_num  = ins_info.ins_serial_num();
	tmpinfo.ins_templ_id    = ins_info.ins_templ_id();
	tmpinfo.world_id        = ins_info.world_id();
	tmpinfo.world_tag       = ins_info.world_tag();
	tmpinfo.ins_create_time = ins_info.ins_create_time();
	if (!wcluster::FindInsWorld(tmpinfo, ret_insinfo))
	{
		return false;
	}
	return true;
}

bool MasterMsgProcess::FindBattleGroundMap(const global::BattleGroundInfo& bg_info, world::battleground_info& ret_bginfo)
{
    world::battleground_info tmpinfo;
    tmpinfo.bg_serial_num  = bg_info.bg_serial_num();
	tmpinfo.bg_templ_id    = bg_info.bg_templ_id();
	tmpinfo.world_id       = bg_info.world_id();
	tmpinfo.world_tag      = bg_info.world_tag();
	tmpinfo.bg_create_time = bg_info.bg_create_time();
	if (!wcluster::FindBGWorld(tmpinfo, ret_bginfo))
	{
		return false;
	}
	return true;
}

bool MasterMsgProcess::HandleEnterInstanceMap(const global::InstanceInfo& insInfo, Player* pPlayer, world::instance_info& ret_insinfo)
{
	global::InstanceInfo ins_info;
	ins_info.CopyFrom(insInfo);
	ins_info.set_world_id(WID_TO_MAPID(insInfo.world_id()));

	if (ins_info.enter_type() == global::ET_INS_CREATE)
	{
		if (!CreateInstanceMap(ins_info, ret_insinfo))
		{
			return false;
		}
	}
	else if (ins_info.enter_type() == global::ET_INS_ENTER || 
			 ins_info.enter_type() == global::ET_INS_REENTER)
	{
		if (!FindInstanceMap(ins_info, ret_insinfo))
		{
			return false;
		}
	}
	else if (ins_info.enter_type() == global::ET_INS_ENTER_CREATE)
	{
		if (!FindInstanceMap(ins_info, ret_insinfo))
		{
			if (!CreateInstanceMap(ins_info, ret_insinfo))
			{
				return false;
			}
		}
	}
	else
	{
		ASSERT(false);
	}

	return true;
}

bool MasterMsgProcess::HandleEnterBattleGroundMap(const global::BattleGroundInfo& bgInfo, Player* pPlayer, world::battleground_info& ret_bginfo)
{
	global::BattleGroundInfo bg_info;
	bg_info.CopyFrom(bgInfo);
	bg_info.set_world_id(WID_TO_MAPID(bgInfo.world_id()));

    if (bg_info.enter_type() == global::ET_BG_REENTER)
	{
		if (!FindBattleGroundMap(bg_info, ret_bginfo))
		{
			return false;
		}
	}
    else if (bg_info.enter_type() == global::ET_BG_ENTER_CREATE)
	{
		if (!FindBattleGroundMap(bg_info, ret_bginfo))
		{
			if (!CreateBattleGroundMap(bg_info, ret_bginfo))
			{
				return false;
			}
		}
	}
	else
	{
		ASSERT(false);
	}

	return true;
}

void MasterMsgProcess::SyncInsTeamInfo(const global::InstanceInfo& ins_info, Player* pPlayer)
{
	msgpack_map_team_info param;
	if (ins_info.has_team_info())
	{
		param.team_id = -1;
		param.leader  = ins_info.team_info().leader();
		int self_pos  = ins_info.team_info().self_pos();
		param.members[self_pos].roleid = pPlayer->role_id();
	}
	else
	{
		param.team_id = -1;
		param.leader  = 0;
		int self_pos  = 0;
		param.members[self_pos].roleid = pPlayer->role_id();
	}

	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_MAP_TEAM_INFO, XID(), XID(), 0, buf.contents(), buf.size());
	pPlayer->SyncMapTeamInfo(msg);
}

void MasterMsgProcess::SyncBGTeamInfo(const global::BattleGroundInfo& bg_info, Player* pPlayer)
{
    msgpack_map_team_info param;
	if (bg_info.has_team_info())
	{
        /*
		param.team_id = -1;
		param.leader  = ins_info.team_info().leader();
		int self_pos  = ins_info.team_info().self_pos();
		param.members[self_pos].roleid = pPlayer->role_id();
        */
	}
	else
	{
        // 没有组队信息时，设为-1，主要目的是把MapTeam的子系统创建出来
		param.team_id = -1;
		param.leader  = 0;
	}

	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_MAP_TEAM_INFO, XID(), XID(), 0, buf.contents(), buf.size());
	pPlayer->SyncMapTeamInfo(msg);
}

void MasterMsgProcess::SyncTeamInfo(const M2G::TeamInfo& proto, Player* pPlayer)
{
	msg_team_info param;
	param.teamid   = proto.teamid();
	param.leaderid = proto.leaderid();
	param.pos1     = proto.pos1();
	param.pos2     = proto.pos2();
	param.pos3     = proto.pos3();
	param.pos4     = proto.pos4();
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_TEAM_INFO, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->ProcessTeamMsg(msg);
}


#define FIND_AND_LOCK_PLAYER(pPlayer, roleid) \
	pPlayer = Gmatrix::FindPlayerFromMan(roleid); \
	if (NULL == pPlayer) \
	{ \
		__PRINTF("FindPlayerFromMan()没有找到玩家%ld", roleid); \
		return; \
	} \
	\
	PlayerLockGuard lock(pPlayer); \
	if (!pPlayer->IsActived()) \
	{ \
		return; \
	}

///
/// team proto process
///
void MasterMsgProcess::JoinTeam(const M2G::JoinTeam& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	msg_join_team param;
	param.new_memberid = proto.new_memberid();
	param.pos          = proto.pos();
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_JOIN_TEAM, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->ProcessTeamMsg(msg);
}

void MasterMsgProcess::LeaveTeam(const M2G::LeaveTeam& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_LEAVE_TEAM, XID(), XID(), proto.leave_roleid());
	pPlayer->ProcessTeamMsg(msg);
}

void MasterMsgProcess::ChangeTeamLeader(const M2G::ChangeTeamLeader& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_CHANGE_TEAM_LEADER, XID(), XID(), proto.new_leaderid());
	pPlayer->ProcessTeamMsg(msg);
}

void MasterMsgProcess::ChangeTeamPos(const M2G::ChangeTeamPos& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	msg_change_team_pos param;
	param.src_index = proto.src_index();
	param.des_index = proto.des_index();
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_CHANGE_TEAM_POS, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->ProcessTeamMsg(msg);
}

void MasterMsgProcess::TeamInfo(const M2G::TeamInfo& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	SyncTeamInfo(proto, pPlayer);
}

void MasterMsgProcess::QueryTeamMember(const M2G::QueryTeamMember& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	msg_query_team_member param;
	param.query_roleid = proto.query_roleid();
	param.link_id      = proto.link_id();
	param.client_sid_in_link = proto.client_sid_in_link();
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_QUERY_TEAM_MEMBER, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->ProcessTeamMsg(msg);
}

void MasterMsgProcess::JoinTeamReq(const M2G::JoinTeamReq& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	if (pPlayer->CanJoinPlayerTeam())
	{
		pPlayer->SendJoinTeamReq(proto.requester(), proto.first_name(), proto.mid_name(), proto.last_name(), proto.invite());
	}
	else
	{
		G2M::JoinTeamRes send_proto;
		send_proto.set_roleid(pPlayer->role_id());
		send_proto.set_invite(proto.invite());
		send_proto.set_accept(2);
		send_proto.set_requester(proto.requester());
		NetToMaster::SendProtocol(pPlayer->master_id(), send_proto);
	}
}

void MasterMsgProcess::TeamLeaderConvene(const M2G::TeamLeaderConvene& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
    FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

    A2DVECTOR pos(proto.pos_x(), proto.pos_y());
    pPlayer->TeamLeaderConvene(proto.leaderid(), proto.world_id(), pos);
}

void MasterMsgProcess::EnterInsReply(const M2G::EnterInsReply& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

	msg_enter_ins_reply param;
	param.enter_type     = (int8_t)proto.enter_type();
	param.err_code       = (int8_t)proto.err_code();
	param.ins_serial_num = proto.ins_serial_num();
	param.ins_templ_id   = proto.ins_templ_id();
	param.src_world_id   = proto.src_world_id();
	param.src_pos_x      = proto.src_pos_x();
	param.src_pos_y      = proto.src_pos_y();

	// reenter
	if (proto.has_reenter())
	{
		param.world_id   = proto.reenter().world_id();
		param.world_tag  = proto.reenter().world_tag();
		param.ins_create_time = proto.reenter().ins_create_time();
	}
	else
	{
		param.world_id   = proto.des_world_id();
		param.world_tag  = 0;
		param.ins_create_time = 0;
	}

	// team ins
	if (proto.has_team_info())
	{
		param.leader_id     = proto.team_info().leader();
		param.self_team_pos = proto.team_info().self_pos();
	}
	else
	{
		param.leader_id     = 0;
		param.self_team_pos = 0;
	}

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_ENTER_INS_REPLY, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeammemberLocalInsReply(const M2G::TeammemberLocalInsReply& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.leader_roleid());

	msgpack_teammeber_localins_reply param;
	param.member_roleid = proto.member_roleid();
	param.agreement     = proto.agreement();
	param.ins_group_id  = proto.ins_group_id();
	for (int i = 0; i < proto.ins_tid_array_size(); ++i)
	{
		param.ins_tid_vec.push_back(proto.ins_tid_array(i));
	}
	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_TEAMMEMBER_LOCALINS_REPLY, XID(), XID(), 0, buf.contents(), buf.size());
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeamLeaderStartLocalIns(const M2G::TeamLeaderStartLocalIns& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.member_roleid());

	// send sync msg to player
	MSG msg;
	msg_ins_transfer_prepare param;
	param.ins_templ_id = proto.ins_templ_id();
	param.request_type = G2M::IRT_UI_TEAM;
	BuildMessage(msg, GS_MSG_INS_TRANSFER_PREPARE, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeammemberQuitLocalIns(const M2G::TeammemberQuitLocalIns& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.leader_roleid());

	msg_quit_team_local_ins param;
	param.member_roleid = proto.member_roleid();
	param.ins_group_id  = proto.ins_group_id();
	
	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_QUIT_TEAM_LOCAL_INS, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeamLocalInsInvite(const M2G::TeamLocalInsInvite& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.member_roleid());

	msg_team_local_ins_invite param;
	param.leader_roleid = proto.leader_roleid();
	param.ins_group_id  = proto.ins_group_id();

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_TEAM_LOCAL_INS_INVITE, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeamCrossInsInvite(const M2G::TeamCrossInsInvite& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.member_roleid());

    msg_team_cross_ins_invite param;
    param.leader_roleid = proto.info().leader_roleid();
    param.ins_group_id  = proto.info().ins_group_id();
    param.ins_templ_id  = proto.info().ins_templ_id();

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_TEAM_CROSS_INS_INVITE, XID(), XID(), 0, &param, sizeof(param));
    pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::TeamCrossInsInviteReply(const M2G::TeamCrossInsInviteReply& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.info().leader_roleid());

    shared::net::ByteBuffer tmpbuf;
    tmpbuf.resize(proto.ByteSize());
    if (!proto.SerializeToArray((void*)tmpbuf.contents(), tmpbuf.size()))
    {
        LOG_ERROR << "TeamCrossInsInviteReply Serialize error! size:" << tmpbuf.size();
        return;
    }

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_TEAM_CROSS_INS_INVITE_RE, XID(), XID(), 0, tmpbuf.contents(), tmpbuf.size());
    pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::GlobalCounterInfo(const M2G::GlobalCounterInfo& proto, const WrapRef wrap)
{
	std::vector<GlobalCounter::Entry> ent_vec;
	if (proto.gc_info_size() == 0)
	{
		GlobalCounter::Entry ent;
		ent.index = 0;
		ent.value = 0;
		ent_vec.push_back(ent);
	}
	else
	{
		for (int i = 0; i < proto.gc_info_size(); ++i)
		{
			const M2G::GlobalCounterInfo::GCInfo& info = proto.gc_info(i);
			GlobalCounter::Entry ent;
			ent.index = info.index();
			ent.value = info.value();
			ent_vec.push_back(ent);
		}
	}

	s_pGCounter->SetCounterValue(proto.master_id(), ent_vec);
}

void MasterMsgProcess::ModifyGlobalCounter(const M2G::ModifyGlobalCounter& proto, const WrapRef wrap)
{
	s_pGCounter->ModifyGCounter(proto.master_id(), proto.index(), proto.delta(), false);
}

void MasterMsgProcess::SetGlobalCounter(const M2G::SetGlobalCounter& proto, const WrapRef wrap)
{
    s_pGCounter->SetGCounter(proto.master_id(), proto.index(), proto.value(), false);
}

void MasterMsgProcess::QueryItemInfo(const M2G::QueryItemInfo& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	itemdata item;
	int8_t result = G2C::QueryItemInfo_Re::ERR_SUCCESS;
	if (!pPlayer->QueryItem(proto.where(), proto.index(), item))
	{
		result = G2C::QueryItemInfo_Re::ERR_ITEM_NOT_EXIST;
	}
	else if (item.id != proto.templ_id())
	{
		result = G2C::QueryItemInfo_Re::ERR_ITEM_NOT_MATCH;
	}

	pPlayer->sender()->ElsePlayerQueryItem(result, item, proto.querier(), proto.link_id(), proto.client_sid_in_link());
}

void MasterMsgProcess::GetMailAttachRe(const M2G::GetMailAttach_Re& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

	msgpack_get_attach_reply param;
	param.mailid = proto.mailid();
	param.attach_cash = proto.attach_cash();
	param.attach_score = proto.attach_score();
	param.attach_item = proto.attach_item();
	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_GET_ATTACH_REPLY, XID(), XID(), 0, buf.contents(), buf.size());
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::DeleteMailRe(const M2G::DeleteMail_Re& proto, const WrapRef)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

	msg_delete_mail_reply param;
	param.mailid = proto.mailid();

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_DELETE_MAIL_REPLY, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::SendMailRe(const M2G::SendMail_Re& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

	msg_send_mail_reply param;
	param.mailid = proto.mailid();

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_SEND_MAIL_REPLY, XID(), XID(), 0, &param, sizeof(param));
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::ReceiveNewMail(const M2G::AnnounceNewMail& proto, const WrapRef wrap)
{
	Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.receiver());

	msgpack_announce_new_mail param;
	param.mailid = proto.mailid();
	param.sender = proto.sender();
	param.receiver = proto.receiver();
	param.attach_cash = proto.attach_cash();
	param.attach_score = proto.attach_score();
	param.name = proto.name();
	param.title = proto.title();
	param.content = proto.content();
	param.attach_item = proto.attach_item();
	param.time = proto.time();
	shared::net::ByteBuffer buf;
	MsgContentMarshal(param, buf);

	// send sync msg to player
	MSG msg;
	BuildMessage(msg, GS_MSG_ANNOUNCE_NEW_MAIL, XID(), XID(), 0, buf.contents(), buf.size());
	pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::GlobalDataChange(const global::GlobalDataChange& proto, const WrapRef wrap)
{
	int32_t masterid = NetIO::GetMasterIdBySid(wrap.sendback_sid);
	if (-1 == masterid)
	{
		__PRINTF("Master sid[%d]找不到对应的master_id，全局数据无法更改！", wrap.sendback_sid);
		return;
	}

	bool is_remove = (proto.optype() == global::GlobalDataChange::OP_DELETE) ? true : false;
	s_pGlobalData->SetGlobalData(masterid, proto.gdtype(), proto.key(), is_remove, 
			                     proto.content().c_str(), proto.content().size());
}

void MasterMsgProcess::RePunchCardHelp(const global::RePunchCardHelp& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.friend_roleid());
    
    shared::net::ByteBuffer tmpbuf;
    tmpbuf.resize(proto.ByteSize());
    if (!proto.SerializeToArray((void*)tmpbuf.contents(), tmpbuf.size()))
    {
        LOG_ERROR << "global::RePunchCardHelp Serialize error! size:" << tmpbuf.size();
        return;
    }

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_RE_PUNCH_CARD_HELP, XID(), XID(), 0, tmpbuf.contents(), tmpbuf.size());
    pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::RePunchCardHelpReply(const global::RePunchCardHelpReply& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.requester());
    
    shared::net::ByteBuffer tmpbuf;
    tmpbuf.resize(proto.ByteSize());
    if (!proto.SerializeToArray((void*)tmpbuf.contents(), tmpbuf.size()))
    {
        LOG_ERROR << "global::RePunchCardHelpReply Serialize error! size:" << tmpbuf.size();
        return;
    }

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_RE_PUNCH_CARD_HELP_REPLY, XID(), XID(), 0, tmpbuf.contents(), tmpbuf.size());
    pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::RePunchCardHelpError(const global::RePunchCardHelpError& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
    FIND_AND_LOCK_PLAYER(pPlayer, proto.friend_roleid());

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_RE_PUNCH_CARD_HELP_ERROR, XID(), XID(), 0, NULL, 0);
    pPlayer->SyncDispatchMsg(msg);
}

void MasterMsgProcess::AuctionInvalidQuery(const M2G::AuctionInvalidQuery& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

    std::vector<int64_t> auctionid_list;
    for (int i = 0; i < proto.auction_id_size(); ++i)
    {
        auctionid_list.push_back(proto.auction_id(i));
    }
    pPlayer->AuctionInvalidQuery(auctionid_list);
}

void MasterMsgProcess::EnterBGReply(const M2G::EnterBGReply& proto, const WrapRef wrap)
{
    Player* pPlayer = NULL;
	FIND_AND_LOCK_PLAYER(pPlayer, proto.roleid());

    shared::net::ByteBuffer tmpbuf;
    tmpbuf.resize(proto.ByteSize());
    if (!proto.SerializeToArray((void*)tmpbuf.contents(), tmpbuf.size()))
    {
        LOG_ERROR << "M2G::EnterBGReply Serialize error! size:" << tmpbuf.size();
        return;
    }

    // send sync msg to player
    MSG msg;
    BuildMessage(msg, GS_MSG_ENTER_BG_REPLY, XID(), XID(), 0, tmpbuf.contents(), tmpbuf.size());
    pPlayer->SyncDispatchMsg(msg);
}

#undef FIND_AND_LOCK_PLAYER

} // namespace gamed
