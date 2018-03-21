#include "player_service.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/service_npc_templ.h"
#include "gs/global/message.h"
#include "gs/global/game_util.h"
#include "gs/global/dbgprt.h"
#include "gs/scene/world.h"
#include "gs/player/subsys_if.h"
#include "gs/player/psession.h"
#include "gs/player/player_sender.h"
#include "gs/obj/service/service_man.h"
#include "gs/obj/service/service_executor.h"
#include "gs/obj/service/service_provider.h"


namespace gamed {

using namespace shared;
using namespace dataTempl;

PlayerService::PlayerService(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_SERVICE, player),
	  service_list_(kMaxUIServicePerPlayer)
{
}
	
PlayerService::~PlayerService()
{
}

int32_t PlayerService::GetNpcCatVision(int32_t tid)
{
	int32_t catvision = GS_INT32_MAX;
	const ServiceNpcTempl* ptmpl = s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(tid);
	if (ptmpl != NULL)
	{
		catvision = ptmpl->cat_vision;
	}
	return catvision;
}

void PlayerService::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::ServiceHello, PlayerService::CMDHandler_ServiceHello);
	REGISTER_NORMAL_CMD_HANDLER(C2G::ServiceServe, PlayerService::CMDHandler_ServiceServe);
	REGISTER_NORMAL_CMD_HANDLER(C2G::ServiceGetContent, PlayerService::CMDHandler_ServiceGetContent);
}

void PlayerService::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_SERVICE_GREETING, PlayerService::MSGHandler_ServiceGreeting);
	REGISTER_MSG_HANDLER(GS_MSG_SERVICE_REQUEST, PlayerService::MSGHandler_ServiceRequest);
	REGISTER_MSG_HANDLER(GS_MSG_SERVICE_DATA, PlayerService::MSGHandler_ServiceData);
	REGISTER_MSG_HANDLER(GS_MSG_SERVICE_ERROR, PlayerService::MSGHandler_ServiceError);
}

///
/// cmd handler
///
void PlayerService::CMDHandler_ServiceHello(const C2G::ServiceHello& cmd)
{
	ResetServiceProvider();

	XID target;
	MAKE_XID(cmd.obj_id, target);
	if (target.IsNpc())
	{
		// 查找npc
		world::worldobj_base_info info;
		if (!player_.world_plane()->QueryObject(target, info))
		{
			LOG_WARN << "找不到对应的npc";
			return;
		}

		// 检查喵类视觉
		if (player_.cat_vision_level() < GetNpcCatVision(info.tid))
		{
			LOG_WARN << "玩家喵类视觉小于npc的catvision";
			return;
		}

		// 检查obj黑名单
		if (!player_.ObjectCanInteract(info.tid))
		{
			LOG_WARN << "Npc在黑名单里，不能交互！tid=" << info.tid;
			return;
		}
		
		// 检查npc可见规则
		if (!CheckNpcVisibleRule(info.tid))
		{
			LOG_WARN << "Npc不可见，不能交互！tid=" << info.tid
				<< " player：" << player_.role_id();
			return;
		}

		PSayHelloSession* psession = new PSayHelloSession(&player_);
		psession->SetTarget(target);
		player_.AddSessionAndStart(psession);
	}
}

void PlayerService::CMDHandler_ServiceServe(const C2G::ServiceServe& cmd)
{
	const ServiceExecutor* executor = ServiceManager::GetExecutor(cmd.service_type);
	if (executor == NULL)
	{
		player_.sender()->ErrorMessage(G2C::ERR_SERVICE_UNAVILABLE);
		return;
	}

	// UI服务和NPC服务的接收方不同
	XID target;
	if (cmd.is_ui_srv)
	{
		// 发给自己
		target = player_.object_xid();
	}
	else // Npc service
	{
		target = service_provider_.xid;
	}

	// check provider
	if (!target.IsObject())
	{
		__PRINTF("CMD ServiceServe provider is not a object !");
		return;
	}

	// send request
	if (!executor->ServeRequest(&player_, target, cmd.content.c_str(), cmd.content.size()))
	{
		player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
		return;
	}
}

void PlayerService::CMDHandler_ServiceGetContent(const C2G::ServiceGetContent& cmd)
{
	const ServiceProvider* provider = ServiceManager::GetProvider(cmd.service_type);
	if (provider == NULL)
	{
		player_.sender()->ErrorMessage(G2C::ERR_SERVICE_UNAVILABLE);
		return;
	}

	if (!service_provider_.xid.IsObject())
	{
		__PRINTF("CMD ServiceGetContent provider is not a object !");
		return;
	}

	//不考虑UI服务了
	//通知ServiceNpc
	msg_service_query_content msqc;
	msqc.link_sid = player_.link_id();
	msqc.client_sid = player_.sid_in_link();
	player_.SendMsg(GS_MSG_SERVICE_QUERY_CONTENT, service_provider_.xid, cmd.service_type, &msqc, sizeof(msqc));
}

///
/// msg handler
///
int PlayerService::MSGHandler_ServiceGreeting(const MSG& msg)
{
	service_provider_.xid = msg.source;
	service_provider_.pos = msg.pos;
	player_.sender()->NpcGreeting(msg.source);

	// 拒绝一次暗雷触发战斗
	player_.RejectLandmineOneTime();
	return 0;
}

int PlayerService::MSGHandler_ServiceRequest(const MSG& msg)
{
	int service_type = msg.param;
	ServiceProvider* provider = service_list_.GetProvider(service_type);
	if (provider == NULL)
	{
		// 检查是否是可用的UI服务
		if (!ServiceManager::IsUISupportService(service_type))
		{
			player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
			return 0;
		}
		else
		{
			return -1;
			/*
			// 如果没找到则创建一个挂在player身上
			provider = ServiceManager::CreateProvider(service_type);
			if (provider == NULL) 
			{
				player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
				return -1;
			}

			// ????????????? provider init!
			if (!provider->Init(static_cast<WorldObject*>(&player_), NULL))
			{
				player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
				DELETE_SET_NULL(provider);
				return -1;
			}

			if (!service_list_.AddProvider(provider))
			{
				player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
				DELETE_SET_NULL(provider);
				return -1;
			}
			*/
		}
	}

	// 服务的请求到来
	provider->PayService(msg.source, msg.content, msg.content_len);
	return 0;
}
	
int PlayerService::MSGHandler_ServiceData(const MSG& msg)
{
	if (msg.source != service_provider_.xid && msg.source != player_.object_xid())
	{
		__PRINTF("MSGHandler_ServiceData 消息的source不是当前service_provider_");
		return 0;
	}

	int service_type = msg.param;
	const ServiceExecutor* executor = ServiceManager::GetExecutor(service_type);
	if (executor)
	{
		executor->Serve(&player_, msg.source, msg.content, msg.content_len);
	}
	return 0;
}

int PlayerService::MSGHandler_ServiceError(const MSG& msg)
{
	player_.sender()->ErrorMessage(msg.param);
	return 0;
}

void PlayerService::SayHelloToNpc(const XID& target)
{
	player_.SendMsg(GS_MSG_SERVICE_HELLO, target, player_.faction());
}

void PlayerService::ResetServiceProvider()
{
	service_provider_ = playerdef::ServiceProviderInfo();
}

bool PlayerService::CheckNpcVisibleRule(int32_t tid)
{
	const ServiceNpcTempl* ptmpl = s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(tid);
	if (ptmpl == NULL)
		return false;

	for (size_t i = 0; i < ptmpl->task_hide_npc.size(); ++i)
	{
		int32_t taskid = ptmpl->task_hide_npc[i];
		if (player_.HasActiveTask(taskid) || player_.HasFinishTask(taskid))
		{
			return false;
		}
	}

	// 初始不可见
	if (!ptmpl->initial_visible)
	{
		bool is_show = false;
		for (size_t i = 0; i < ptmpl->task_show_npc.size(); ++i)
		{
			int32_t taskid = ptmpl->task_show_npc[i];
			if (player_.HasActiveTask(taskid) || player_.HasFinishTask(taskid))
			{
				is_show = true;
				break;
			}
		}

		if (!is_show)
		{
			return false;
		}
	}

	return true;
}

} // namespace gamed
