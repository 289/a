#include "link_msg_proc.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/C2G_proto.h"

#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/netio/netio_if.h"
#include "gs/player/player.h"
#include "gs/scene/world_man.h"

// recv proto
#include "common/protocol/gen/global/connection_status.pb.h"
#include "common/protocol/gen/global/server_status.pb.h"
#include "common/protocol/gen/L2G/c2sgamedatasend.pb.h"
#include "common/protocol/gen/L2G/announce_link_info.pb.h"
#include "common/protocol/gen/L2G/player_logout.pb.h"
#include "common/protocol/gen/L2G/redirect_player_gs_err.pb.h"

// send proto
#include "common/protocol/gen/G2L/announce_gs_info.pb.h"


namespace gamed {

namespace 
{
	inline Timestamp getCurTime()
	{
		if (Gmatrix::GetServerParam().permit_debug_print)
		{
			return Timestamp::Now();
		}
		return Timestamp();
	}

	inline int32_t convTypeNo(int32_t cmdTypeNo)
	{
		if (cmdTypeNo >= C2G_DEBUG_CMD_LOWER_LIMIT && cmdTypeNo <= C2G_DEBUG_CMD_UPPER_LIMIT)
			return cmdTypeNo;
		return cmdTypeNo - C2G_CMD_LOWER_LIMIT;
	}

} // Anonymous

network::LinkRecvQueue LinkMsgProcess::recv_queue_;

void* LinkMsgProcess::ThreadFunc(void* pdata)
{
	Assert(pdata);
	LinkMsgProcess* plinkproc = static_cast<LinkMsgProcess*>(pdata);
	plinkproc->RunInThread();
	LOG_INFO << "Link msg process thread exit!";

	return NULL;
}

LinkMsgProcess::LinkMsgProcess()
	: proc_thread_(LinkMsgProcess::ThreadFunc, this),
	  dispatcher_(BIND_MEM_CB(&LinkMsgProcess::ProtoDefaultHandler, this)),
	  is_done_(false)
{
	StartupRegisterHandler();
}

LinkMsgProcess::~LinkMsgProcess()
{
	ASSERT(recv_queue_.empty());
}

bool LinkMsgProcess::StartProcThread()
{
	proc_thread_.Start();
	return true;
}

void LinkMsgProcess::StopProcThread()
{
	QueuedRecvMsg tmp;
	global::ServerStatusNotify* stop_msg = new global::ServerStatusNotify();
	stop_msg->set_type(global::ServerInfo::GAMESERVER_SVR);
	stop_msg->set_status(global::ServerInfo::SVR_SHUTDOWN);
	tmp.msg_ptr = stop_msg;
	recv_queue_.put(tmp); // FIXME: will invoke ProtoDefaultHandler(), and could be crash

	proc_thread_.Join();
}

network::LinkRecvQueue* LinkMsgProcess::get_recv_queue()
{
	return &recv_queue_;
}

void LinkMsgProcess::RunInThread()
{
	std::deque<network::QueuedRecvMsg> tmpList;
	while (!is_done_)
	{
		while (!NetIO::HasRecvLinkData()) { usleep(2000); }

		// locked inside
		NetIO::TakeAllLinkData(tmpList);
		while (!tmpList.empty())
		{
			network::QueuedRecvMsg& msg = tmpList.front();
			HandleLinkMessage(msg);
			SAFE_DELETE(msg.msg_ptr);
			tmpList.pop_front();
		}
	}
}

void LinkMsgProcess::ProtoDefaultHandler(const MessageRef msg, const WrapRef wrap)
{
	LOG_ERROR << "LinkMsgProcess Error recv UnRegister pb msg [name: " << msg.GetTypeName() << "]";
}

void LinkMsgProcess::HandleLinkMessage(network::QueuedRecvMsg& msg)
{
	dispatcher_.OnQueuedMessage(*msg.msg_ptr, msg);
}

#define REGISTER_PROC(protocol, func) \
	dispatcher_.Register<protocol>(BIND_MEM_CB(&func, this));
void LinkMsgProcess::StartupRegisterHandler()
{
	REGISTER_PROC(global::ServerStatusNotify, LinkMsgProcess::ServerStatusNotify);
	REGISTER_PROC(global::ConnectEstablished, LinkMsgProcess::LinkOnConnected);
	REGISTER_PROC(global::ConnectDestroyed, LinkMsgProcess::LinkConnDestroyed);
	REGISTER_PROC(L2G::C2SGamedataSend, LinkMsgProcess::C2SGamedataSend);
	REGISTER_PROC(L2G::AnnounceLinkInfo, LinkMsgProcess::AnnounceLinkInfo);
	REGISTER_PROC(L2G::PlayerLogout, LinkMsgProcess::LinkPlayerLogout);
	REGISTER_PROC(L2G::RedirectPlayerGSError, LinkMsgProcess::RedirectPlayerGSError);
}

void LinkMsgProcess::ServerStatusNotify(const global::ServerStatusNotify& msg, const WrapRef wrap)
{
	if (msg.type() == global::ServerInfo::GAMESERVER_SVR)
	{
		if (msg.status() == global::ServerInfo::SVR_SHUTDOWN)
		{
			is_done_ = true;
			LOG_WARN << "GS shutdown id:" << s_pGmatrix->GetGameServerID()
				<< " link process thread quit";
		}
	}
}

void LinkMsgProcess::LinkOnConnected(const global::ConnectEstablished& msg, const WrapRef wrap)
{
	if (global::ConnectionInfo::LINK_CONN != msg.type()) return;

	G2L::AnnounceGSInfo announce;
	announce.set_gsid(Gmatrix::GetGameServerID());
	
	NetIO::SendToLinkBySid(wrap.sendback_sid, announce);
}

void LinkMsgProcess::LinkConnDestroyed(const global::ConnectDestroyed& msg, const WrapRef wrap)
{
	if (global::ConnectionInfo::LINK_CONN != msg.type()) return;

	s_pGmatrix->LinkDisconnect(msg.linkid());

	NetIO::RemoveLinkIdMapToSid(msg.linkid());
}

void LinkMsgProcess::AnnounceLinkInfo(const L2G::AnnounceLinkInfo& msg, const WrapRef wrap)
{
	NetIO::SetLinkIdMapToSid(msg.link_id(), wrap.sendback_sid);
}

void LinkMsgProcess::LinkPlayerLogout(const L2G::PlayerLogout& msg, const WrapRef wrap)
{
	Player* pPlayer = Gmatrix::FindPlayerFromMan(msg.roleid());
	if (NULL == pPlayer)
	{
		// 现在先做下线处理 ????????
		//GMSV::SendDisconnect(cs_index, uid, sid,0);
		__PRINTF("LinkPlayerLogout() 玩家%ld没有登录", msg.roleid());
		return;
	}

	PlayerLockGuard lock(pPlayer);
	if (!pPlayer->IsActived())
		return;

	if (pPlayer->link_id() != msg.linkid())
	{
		__PRINTF("LinkPlayerLogout() linkid 不匹配player:%d, msg:%d", pPlayer->link_id(), msg.linkid());
		return;
	}

	pPlayer->PlayerLogout(playerdef::LT_DISCONNECT_ON_LINK);
	GLog::log("玩家 %ld 收到L2G::PlayerLogout下线！", msg.roleid());
}

void LinkMsgProcess::RedirectPlayerGSError(const L2G::RedirectPlayerGSError& proto, const WrapRef wrap)
{
	Player* pPlayer = Gmatrix::FindPlayerFromMan(proto.roleid());
	if (NULL == pPlayer)
	{
		// 现在先做下线处理 ????????
		//GMSV::SendDisconnect(cs_index, uid, sid,0);
		__PRINTF("RedirectPlayerGSError() 玩家%ld没有登录", proto.roleid());
		return;
	}

	PlayerLockGuard lock(pPlayer);
	if (!pPlayer->IsActived())
		return;

	if (pPlayer->link_id() != proto.linkid())
	{
		__PRINTF("RedirectPlayerGSError() 玩家%ld linkid 不匹配player:%d, proto:%d", 
				proto.roleid(), pPlayer->link_id(), proto.linkid());
		return;
	}

	pPlayer->PlayerLogout(playerdef::LT_DISCONNECT_ON_LINK);
	GLog::log("玩家%ld收到L2G::RedirectPlayerGSError下线！", proto.roleid());
}

#define DEBUG_PRINTF(cmd_no, fmt, ...) \
    if (__builtin_expect(__PRINT_FLAG, 1)) \
    { \
        if (cmd_no == C2G::START_MOVE || \
            cmd_no == C2G::MOVE_CONTINUE || \
            cmd_no == C2G::STOP_MOVE) \
        { \
            if (!Gmatrix::GetServerParam().forbid_move_print) \
            { \
                __PRINTF(fmt, __VA_ARGS__); \
            } \
        } \
        else \
        { \
            __PRINTF(fmt, __VA_ARGS__); \
        } \
    }

void LinkMsgProcess::C2SGamedataSend(const L2G::C2SGamedataSend& msg, const WrapRef wrapref)
{
	Player* pPlayer = Gmatrix::FindPlayerFromMan(msg.roleid());
	if (NULL == pPlayer)
	{
		//GLog::log("user_cmd::用户%d已经不再服务器中，断线处理", msg.roleid());

		// 现在先做下线处理 ????????
		//GMSV::SendDisconnect(cs_index, uid, sid,0);
		__PRINTF("玩家%ld已经不在线，收到L2G::C2SGamedataSend", msg.roleid());
		return;
	}

	// lock player
	PlayerLockGuard lock(pPlayer);

	if (!pPlayer->IsActived() ||
		pPlayer->link_id() != msg.link_id() ||
		pPlayer->sid_in_link() != msg.client_sid_in_link() ||
		pPlayer->object_id() != msg.roleid())
	{
		return;
	}
	
	// new packet
	ProtoPacketCodec::PacketPtr packet = NULL;
	packet = ProtoPacketCodec::UnmarshalPacket(msg.cmd_type_no(), msg.content().c_str(), msg.content().size());
	if (NULL == packet)
	{
		LOG_ERROR << "C2SGamedataSend packet_content unmarshal error! role_id:" << msg.roleid() 
			<< "cmd_type:" << convTypeNo(msg.cmd_type_no());
		return ;
	}

	// start
	Timestamp start(getCurTime());
	// process
	if (pPlayer->DispatchCommand(msg.cmd_type_no(), packet))
	{
		LOG_ERROR << "user cmd:" << convTypeNo(msg.cmd_type_no()) << " handler error!";
	}
	// end
	Timestamp end(getCurTime());

	// delete packet
	SAFE_DELETE(packet);

	double execution_time = TimeMsecsDifference(end, start);
	//__PRINTF("player %ld command %5d use ---------------- %6.4lf", msg.roleid(), convTypeNo(msg.cmd_type_no()), execution_time);
    DEBUG_PRINTF(msg.cmd_type_no(), "player %ld command %5d use ---------------- %6.4lf", msg.roleid(), 
            convTypeNo(msg.cmd_type_no()), execution_time);
	return;
}

#undef DEBUG_PRINTF

} // namespace gamed
