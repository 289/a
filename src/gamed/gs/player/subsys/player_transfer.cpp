#include "player_transfer.h"

#include "gs/template/map_data/mapdata_manager.h"
#include "gs/global/dbgprt.h"
#include "gs/player/psession.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"

// proto
#include "common/protocol/gen/G2M/instance_msg.pb.h"


namespace gamed {

using namespace mapDataSvr;
using namespace common::protocol;

PlayerTransfer::PlayerTransfer(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_TRANSFER, player),
	  retry_counter_(0),
	  waiting_counter_(0)
{
	reset_trans_param();
}

PlayerTransfer::~PlayerTransfer()
{
}

void PlayerTransfer::OnRelease()
{
	retry_counter_   = 0;
	waiting_counter_ = 0;
	reset_trans_param();
}

void PlayerTransfer::ChangeMapError(bool is_change_gs_err)
{
	if (is_change_gs_err)
	{
		retry_counter_ = kFailureRetryCounter;
	}
	else
	{
		retry_counter_ = kFailureTryPlaneSwitch;
	}
}

void PlayerTransfer::OnHeartbeat(time_t cur_time)
{
	if (retry_counter_ >= 0)
	{
		--retry_counter_;
	}

	if (transport_param_.target_world_id != 0)
	{
        if (--waiting_counter_ < 0)
        {
            __PRINTF("玩家:%ld客户端没有回复TransferPrepareFinish，强制进行传送到地图:%d！", 
                    player_.role_id(), transport_param_.target_world_id);
            ExecuteTransport();
        }
    }
}

void PlayerTransfer::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::TransferPrepareFinish, PlayerTransfer::CMDHandler_TransferPrepareFinish);
}

void PlayerTransfer::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_PLAYER_REGION_TRANSPORT, PlayerTransfer::MSGHandler_RegionTransfer);
	REGISTER_MSG_HANDLER(GS_MSG_PLAYER_SWITCH_ERROR, PlayerTransfer::MSGHandler_PlayerSwitchError);
	REGISTER_MSG_HANDLER(GS_MSG_INS_TRANSFER_START, PlayerTransfer::MSGHandler_InsTransferStart);
    REGISTER_MSG_HANDLER(GS_MSG_BG_TRANSFER_START, PlayerTransfer::MSGHandler_BGTransferStart);
}

void PlayerTransfer::CMDHandler_TransferPrepareFinish(const C2G::TransferPrepareFinish& cmd)
{
	ExecuteTransport();
}

bool PlayerTransfer::IsLocalMap(int32_t target_wid) const
{
	if (player_.world_id() == target_wid)
		return true;

    int32_t src_wid = player_.world_id() & LOW_16_MASK;
    int32_t des_wid = target_wid & LOW_16_MASK;
    if (src_wid == des_wid && src_wid != 0)
        return true;

	return false;
}

int PlayerTransfer::MSGHandler_RegionTransfer(const MSG& msg)
{
	if (retry_counter_ > 0)
	{
		__PRINTF("玩家%ld传送失败counter:%d大于0", player_.role_id(), retry_counter_);
		return 0;
	}

	if (transport_param_.target_world_id != 0)
	{
		__PRINTF("玩家%ld正在传送中，不能再次执行传送", player_.role_id());
		return 0;
	}

	if (!player_.CanSwitch())
	{
		__PRINTF("玩家%ld当前状态无法传送，MSGHandler_RegionTransfer()", player_.role_id());
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_player_region_transport);
	msg_player_region_transport& param = *(msg_player_region_transport*)msg.content;
	if (param.source_world_id == player_.world_id())
	{
		// 检查来自传送区域的传送消息
		if (param.elem_id > 0 && !CheckTransferCond(param.elem_id))
		{
			player_.sender()->ErrorMessage(G2C::ERR_TRANSFER_NOT_MEET_COND);
			return 0;
		}

		// 非本图的副本地图
		if (!IsLocalMap(param.target_world_id) && 
			IS_INS_MAP(param.target_world_id))
		{
			if (IS_INS_MAP(player_.world_id()))
			{
				__PRINTF("玩家%ld试图通过传送从副本地图进入另一个副本地图", player_.role_id());
				return 0;
			}
			msg_ins_transfer_prepare ins_param;
			ins_param.ins_templ_id = param.ins_templ_id;
			ins_param.request_type = G2M::IRT_TRANSFER;
			player_.SendMsg(GS_MSG_INS_TRANSFER_PREPARE, player_.object_xid(), &ins_param, sizeof(ins_param));
		}
        else if (IS_NORMAL_MAP(param.target_world_id) ||
                 IsLocalMap(param.target_world_id)) // 普通地图或者本图
		{
			// 跨服可以需要对world_id做调整
			if (IsLocalMap(param.target_world_id) &&
				IS_CR_INS(player_.world_id()))
			{
				param.target_world_id = player_.world_id();
			}
			transport_param_ = param;
			waiting_counter_ = kWaitingClientReply;
			player_.sender()->TransferPrepare(param.elem_id);
		}
        else
        {
            LOG_ERROR << "传送到不是本地图的非普通地图，而且不是副本，target_world_id:" << param.target_world_id;
            return 0;
        }
	}
	
	return 0;
}

int PlayerTransfer::MSGHandler_PlayerSwitchError(const MSG& msg)
{
	ChangeMapError(false);
	player_.sender()->ErrorMessage(G2C::ERR_CANNOT_REGION_TRANSPORT);
	return 0;
}

int PlayerTransfer::MSGHandler_InsTransferStart(const MSG& msg)
{
	if (transport_param_.target_world_id != 0)
	{
		__PRINTF("玩家%ld正在副本传送中，不能再次执行副本传送", player_.role_id());
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_player_region_transport);
	msg_player_region_transport& param = *(msg_player_region_transport*)msg.content;
	if (param.source_world_id == player_.world_id())
	{
		transport_param_ = param;
		waiting_counter_ = kWaitingClientReply;
		player_.sender()->TransferPrepare(0);
	}

	return 0;
}

int PlayerTransfer::MSGHandler_BGTransferStart(const MSG& msg)
{
    if (transport_param_.target_world_id != 0)
	{
		__PRINTF("玩家%ld正在战场传送中，不能再次执行战场传送", player_.role_id());
		return 0;
	}

	CHECK_CONTENT_PARAM(msg, msg_player_region_transport);
	msg_player_region_transport& param = *(msg_player_region_transport*)msg.content;
	if (param.source_world_id == player_.world_id())
	{
		transport_param_ = param;
		waiting_counter_ = kWaitingClientReply;
		player_.sender()->TransferPrepare(0);
	}

    return 0;
}

void PlayerTransfer::ExecuteTransport()
{
	if (transport_param_.source_world_id == player_.world_id())
	{
		// 可能正在执行脚本不能进行传送
		if (player_.CanSwitch())
		{
			PRegionTransportSession* psession = new PRegionTransportSession(&player_);
			psession->SetTransportTarget(transport_param_.target_world_id, transport_param_.target_pos);
			player_.AddSessionAndStart(psession);

            // 是复活传送，要加上无敌
            if (transport_param_.is_resurrect)
            {
                //添加无敌Buff
                player_.AddSessionAndStart(new PInvincibleSession(&player_, INVINCIBLE_BUFF_TIME));
            }
        }
        reset_trans_param();
	}
}

bool PlayerTransfer::CheckTransferCond(int32_t area_elem_id)
{
	const TransferArea* pelem = s_pMapData->QueryMapDataTempl<TransferArea>(area_elem_id);
	if (pelem == NULL)
	{
		LOG_ERROR << "没有找到对应的传送区域：elem_id=" << area_elem_id;
		return false;
	}

	bool is_success = false;
	if (pelem->require_tasks.size() > 0)
	{
		for (size_t i = 0; i < pelem->require_tasks.size(); ++i)
		{
			int32_t taskid = pelem->require_tasks[i];
			if (player_.HasActiveTask(taskid) || player_.HasFinishTask(taskid))
			{
				is_success = true;
				continue;
			}
		}
	}
	else
	{
		is_success = true;
	}

	return is_success;
}

} // namespace gamed
