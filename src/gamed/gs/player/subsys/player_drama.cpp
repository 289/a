#include "player_drama.h"

#include "gamed/game_module/task/include/task.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/service_npc_templ.h"
#include "gs/template/data_templ/mine_templ.h"
#include "gs/global/dbgprt.h"
#include "gs/global/game_util.h"
#include "gs/obj/service/service_provider.h"
#include "gs/obj/service/service_executor.h"
#include "gs/obj/service/service_man.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

using namespace dataTempl;

PlayerDrama::PlayerDrama(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_DRAMA, player),
	  cur_mine_tid_(0),
	  cur_gather_seq_no_(0),
	  gather_timeout_(0)
{
}
	
PlayerDrama::~PlayerDrama()
{
	NpcServiceMap::iterator it = npc_service_map_.begin();
	for (; it != npc_service_map_.end(); ++it)
	{
		DELETE_SET_NULL(it->second);
	}
	npc_service_map_.clear();

	ResetGatherInfo();
}

void PlayerDrama::ResetGatherInfo()
{
	cur_mine_tid_      = 0;
	cur_gather_seq_no_ = 0;
	gather_timeout_    = 0;
}

void PlayerDrama::OnHeartbeat(time_t cur_time)
{
	if (gather_timeout_ > 0)
	{
		if (--gather_timeout_ <= 0)
		{
			ResetGatherInfo();
		}
	}
}

int32_t PlayerDrama::GetNpcCatVision(int32_t tid)
{
	int32_t catvision = GS_INT32_MAX;
	const ServiceNpcTempl* ptmpl = s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(tid);
	if (ptmpl != NULL)
	{
		catvision = ptmpl->cat_vision;
	}
	return catvision;
}

void PlayerDrama::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::DramaTriggerCombat, PlayerDrama::CMDHandler_DramaTriggerCombat);
	REGISTER_NORMAL_CMD_HANDLER(C2G::DramaServiceServe, PlayerDrama::CMDHandler_DramaServiceServe);
	REGISTER_NORMAL_CMD_HANDLER(C2G::DramaGatherMaterial, PlayerDrama::CMDHandler_DramaGatherMaterial);
	REGISTER_NORMAL_CMD_HANDLER(C2G::DramaGatherMiniGameResult, PlayerDrama::CMDHandler_DramaGatherMiniGameResult);
}

void PlayerDrama::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_GATHER_REQUEST, PlayerDrama::MSGHandler_GatherRequest);
	REGISTER_MSG_HANDLER(GS_MSG_GATHER_CANCEL, PlayerDrama::MSGHandler_GatherCancel);
	REGISTER_MSG_HANDLER(GS_MSG_GATHER_COMPLETE, PlayerDrama::MSGHandler_GatherComplete);
}

void PlayerDrama::CMDHandler_DramaTriggerCombat(const C2G::DramaTriggerCombat& cmd)
{
	///
	/// check validity
	///
	if (!player_.HasActiveTask(cmd.task_id) || player_.IsTaskFinish(cmd.task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能发起战斗！", cmd.task_id);
		return;
	}
	int32_t battle_scene_id = 0;
	if (!task::IsTaskMonster(cmd.task_id, cmd.monster_tid, cmd.monster_group_id, battle_scene_id))
	{
		__PRINTF("不是任务刷出来的怪，不能触发战斗！");
		return;
	}

	if (cmd.monster_group_id <= 0)
	{
		__PRINTF("CMDHandler_DramaTriggerCombat 任务:%d 所填的怪物组id非法:%d", cmd.task_id, cmd.monster_group_id);
		return;
	}
	
	///
	/// success
	///
	msg_sys_trigger_combat param;
    param.task_id = cmd.task_id;
	param.monster_group_id = cmd.monster_group_id;
	param.battle_scene_id  = battle_scene_id;
	player_.SendMsg(GS_MSG_SYS_TRIGGER_COMBAT, player_.object_xid(), &param, sizeof(param));
}

void PlayerDrama::CMDHandler_DramaServiceServe(const C2G::DramaServiceServe& cmd)
{
	///
	/// check validity
	///
	if (!player_.HasActiveTask(cmd.task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能提供npc服务！", cmd.task_id);
		return;
	}
	if (!task::IsTaskNPC(cmd.task_id, cmd.npc_tid))
	{
		__PRINTF("不是任务刷出来的npc，不能提供服务！");
		return;
	}
	if (player_.cat_vision_level() < GetNpcCatVision(cmd.npc_tid))
	{
		LOG_WARN << "玩家喵类视觉小于npc的catvision";
		return;
	}
	if (!player_.ObjectCanInteract(cmd.npc_tid))
	{
		LOG_WARN << "Npc在黑名单里，不能交互！tid=" << cmd.npc_tid;
		return;
	}

	
	///
	/// service param check
	///
	const ServiceExecutor* executor = ServiceManager::GetExecutor(cmd.service_type);
	if (executor == NULL)
	{
		player_.sender()->ErrorMessage(G2C::ERR_SERVICE_UNAVILABLE);
		return;
	}

	// send request
	if (!executor->CheckRequestParam(cmd.content.c_str(), cmd.content.size()))
	{
		__PRINTF("PlayerDrama 服务参数不合法！");
		player_.sender()->ErrorMessage(G2C::ERR_SERVICE_ERR_REQUEST);
		return;
	}

	///
	/// success service serve
	///
	NpcServiceMap::iterator it = npc_service_map_.find(cmd.npc_tid);
	if (it == npc_service_map_.end())
	{
		if (!CreateService(cmd.npc_tid))
		{
			__PRINTF("PlayerDrama npc 服务创建失败！tid:%d service_type:%d", cmd.npc_tid, cmd.service_type);
			return;
		}

		it = npc_service_map_.find(cmd.npc_tid);
		ASSERT(it != npc_service_map_.end());
	}

	ProviderList* service_list = it->second;
	ServiceProvider* provider  = service_list->GetProvider(cmd.service_type);
	if (provider == NULL)
	{
		__PRINTF("PlayerDrama Npc没有这项服务！npc_tid:%d service_type:%d", cmd.npc_tid, cmd.service_type);
		return;
	}

	// 服务的请求到来
	provider->PayService(player_.object_xid(), cmd.content.c_str(), cmd.content.size());
}

bool PlayerDrama::CreateService(int32_t npc_tid)
{
	const ServiceNpcTempl* ptmpl = s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(npc_tid);
	if (ptmpl == NULL)
	{
		LOG_ERROR << "PlayerDrama::CreateService() failure! reason:template not found, "
			<< "or tamplate type invalid! templ_id" << npc_tid;
		return false;
	}

	ProviderList* service_list = new ProviderList(kMaxServiceProviderPerNpc);
	if (service_list == NULL)
	{
		return false;
	}

	// 以下代码如果有错误不能直接return，因为要删除service_vec
	bool bRst = true;
	std::vector<serviceDef::ServiceTempl*> service_vec;
	if (!serviceDef::ParseBuffer(ptmpl->service_content, service_vec))
	{
		LOG_ERROR << "PlayerDrama::CreateService() error! templ_id:" << npc_tid; 
		bRst = false;
	}
	else // parse success
	{
		for (size_t i = 0; i < service_vec.size(); ++i)
		{
			int service_type = service_vec[i]->GetType();
			ServiceProvider* provider = ServiceManager::CreateProvider(service_type);
			if (provider != NULL) 
			{
				// if success
				if (provider->Init(&player_, service_vec[i]))
				{
					if (service_list->AddProvider(provider))
					{
						continue;
					}
				}

				LOG_ERROR << "PlayerDrama::CreateService() error! Provider Init() or AddProvider() error! templ_id:" 
					<< npc_tid << "service_type:" << service_type; 

				// error
				bRst = false;
				break;
			}
			else // error
			{
				bRst = false;
				break;
			}
		}
	}

	// insert to map
	if (bRst)
	{
		if (!npc_service_map_.insert(std::make_pair(npc_tid, service_list)).second)
		{
			bRst = false;
		}
	}

	// delete all ServiceTempl
	for (size_t i = 0; i < service_vec.size(); ++i)
	{
		DELETE_SET_NULL(service_vec[i]);
	}

	// error occured
	if (!bRst)
	{
		DELETE_SET_NULL(service_list);
	}

	return bRst;
}

void PlayerDrama::CMDHandler_DramaGatherMaterial(const C2G::DramaGatherMaterial& cmd)
{
	///
	/// check validity
	///
	if (!player_.HasActiveTask(cmd.task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能采集！", cmd.task_id);
		return;
	}
	if (!task::IsTaskMine(cmd.task_id, cmd.mine_tid))
	{
		__PRINTF("不是任务刷出来的矿，不能采集！");
		return;
	}
	if (!player_.ObjectCanInteract(cmd.mine_tid))
	{
		LOG_WARN << "矿在黑名单里，不能交互！tid=" << cmd.mine_tid;
		return;
	}

	
	// 查看状态
	if (!player_.state().CanGather())
	{
		__PRINTF("当前状态无法采集 C2G::DramaGatherMaterial");
		return;
	}

	// 找到数据模板
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(cmd.mine_tid);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Mine的DataTempl，templ_id=" << cmd.mine_tid;
		return;
	}

	// 正在采集中
	if (cur_gather_seq_no_ != 0)
	{
		__PRINTF("gather_seq_no大于0，当前无法采集C2G::DramaGatherMaterial");
		return;
	}

	player_.SendMsg(GS_MSG_DRAMA_GATHER, player_.object_xid(), cmd.mine_tid);

	// templ id
	cur_mine_tid_ = cmd.mine_tid;
}

void PlayerDrama::CMDHandler_DramaGatherMiniGameResult(const C2G::DramaGatherMiniGameResult& cmd)
{
	///
	/// check validity
	///
	if (!player_.HasActiveTask(cmd.task_id))
	{
		__PRINTF("玩家没有该任务task_id%d，不能采集！", cmd.task_id);
		return;
	}
	if (!task::IsTaskMine(cmd.task_id, cmd.mine_tid))
	{
		__PRINTF("不是任务刷出来的矿，不能采集！");
		return;
	}
	if (!player_.ObjectCanInteract(cmd.mine_tid))
	{
		LOG_WARN << "矿在黑名单里，不能交互！tid=" << cmd.mine_tid;
		return;
	}

	
	// 查看状态
	if (!player_.state().CanGather())
	{
		__PRINTF("当前状态无法采集 C2G::DramaGatherMiniGameResult");
		return;
	}

	// 找到数据模板
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(cmd.mine_tid);
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Mine的DataTempl，templ_id=" << cmd.mine_tid;
		return;
	}

	msg_drama_erase_result param;
	param.gather_seq_no = cmd.gather_seq_no;
	param.is_success    = cmd.is_success;
	player_.SendMsg(GS_MSG_DRAMA_GATHER_MG_RESULT, player_.object_xid(), &param, sizeof(param));
}

int PlayerDrama::MSGHandler_GatherRequest(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_gather_request);
	const msg_gather_request& param = *(msg_gather_request*)msg.content;
	const XID playerxid = msg.source;
	if (player_.object_xid() != playerxid)
	{
		__PRINTF("PlayerDrama 不是自己发的GatherRequest！！！");
		return 0;
	}

	if (param.gather_seq_no > cur_gather_seq_no_)
	{
		cur_gather_seq_no_ = param.gather_seq_no;
		gather_timeout_    = param.gather_timeout + kGatherTimeoutDelay;

		msg_gather_reply reply_param;
		reply_param.gather_seq_no = cur_gather_seq_no_;
		reply_param.templ_id      = cur_mine_tid_;
		player_.SendMsg(GS_MSG_GATHER_REPLY, playerxid, &reply_param, sizeof(reply_param));
	}

	return 0;
}

int PlayerDrama::MSGHandler_GatherCancel(const MSG& msg)
{
	if (player_.object_xid() != msg.source)
	{
		__PRINTF("PlayerDrama 不是自己发的GatherCancel！！！");
		return 0;
	}

	if (msg.param >= cur_gather_seq_no_)
	{
		ResetGatherInfo();
	}

	return 0;
}

int PlayerDrama::MSGHandler_GatherComplete(const MSG& msg)
{
	if (player_.object_xid() != msg.source)
	{
		__PRINTF("PlayerDrama 不是自己发的GatherComplete！！！");
		return 0;
	}

	if (msg.param != cur_gather_seq_no_)
	{
		return 0;
	}

	const int32_t gather_seq_no = msg.param;
	const XID playerxid         = msg.source;

	// gather success
	msg_gather_result result_param;
	result_param.gather_seq_no = gather_seq_no;
	result_param.templ_id      = cur_mine_tid_;
	player_.SendMsg(GS_MSG_GATHER_RESULT, playerxid, &result_param, sizeof(result_param));

	// clear
	ResetGatherInfo();
	return 0;
}

} // namespace gamed
