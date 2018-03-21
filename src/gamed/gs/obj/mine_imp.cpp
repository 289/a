#include "mine_imp.h"

#include "shared/logsys/logging.h"
#include "gs/global/game_util.h"
#include "gs/template/map_data/mapdata_manager.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/mine_templ.h"
#include "gamed/client_proto/G2C_error.h"


namespace gamed {

using namespace mapDataSvr;
using namespace dataTempl;
using namespace matterdef;

MineImp::MineImp(Matter& matter)
	: MatterImp(matter),
	  mine_type_(MINE_INVALID)
{
}

MineImp::~MineImp()
{
	mine_type_ = MINE_INVALID;
}

bool MineImp::OnInit()
{
	// 数据模板配置
	const MineTempl* pdata = s_pDataTempl->QueryDataTempl<MineTempl>(matter_.templ_id());
	if (!pdata) 
	{
		LOG_ERROR << "没有找到Matter的DataTempl，elem_id=" << matter_.elem_id() 
			<< " templ_id=" << matter_.templ_id();
		return false;
	}
	if (pdata->turn_on_mode == MineTempl::TM_ERASING)
	{
		mine_type_ = MINE_ERASING;
	}
	else if (pdata->turn_on_mode == MineTempl::TM_PROGRESSING)
	{
		mine_type_ = MINE_COMMON;
	}
	else if (pdata->turn_on_mode == MineTempl::TM_BLANK_FILLING)
	{
		mine_type_ = MINE_BLANK_FILLING;
	}
	else
	{
		mine_type_ = MINE_INVALID;
		return false;
	}

	// 地图元素配置
	const BaseMapData* pmapdata = s_pMapData->QueryBaseMapDataTempl(matter_.elem_id());
	if (pmapdata == NULL)
	{
		LOG_ERROR << "MineImp::OnInit() failure! reason:mapelem not found, "
			<< "or elem type invalid! templ_id" << matter_.templ_id()
			<< " elem_id" << matter_.elem_id();
		return false;
	}

	int type = pmapdata->GetType();
	switch (type)
	{
		case MAPDATA_TYPE_SPOT_MINE:
			{
				const SpotMine* pmine = dynamic_cast<const SpotMine*>(pmapdata);
				if (pmine == NULL) 
					return false;

				FillElemInfo(pmine);
			}
			break;

		case MAPDATA_TYPE_AREA_MINE:
			{
				const AreaMine* pmine = dynamic_cast<const AreaMine*>(pmapdata);
				if (pmine == NULL) 
					return false;

				FillElemInfo(pmine);
			}
			break;

		default:
			{
				LOG_ERROR << "MineImp::OnInit() map element type invalid! type:" << type 
					<< " elem_id" << matter_.elem_id(); 
			}
			return false;
	}

	// always the last!
	if (!InitRuntimeData())
		return false;

	return true;
}

void MineImp::FillElemInfo(const mapDataSvr::SpotMine* pmine)
{
	elem_info_.birth_place         = matter_.pos();   
	elem_info_.birth_dir           = matter_.dir();          
	elem_info_.gather_player_num   = pmine->gather_player_num ? pmine->gather_player_num : kDefaultMaxGatherPlayer;
	elem_info_.is_gather_disappear = pmine->is_gather_disappear;
	elem_info_.is_auto_refresh     = pmine->is_auto_refresh;
	elem_info_.refresh_time        = pmine->refresh_time;
}

void MineImp::FillElemInfo(const mapDataSvr::AreaMine* pmine)
{
	elem_info_.birth_place         = matter_.pos();   
	elem_info_.birth_dir           = matter_.dir();          
	elem_info_.gather_player_num   = pmine->gather_player_num ? pmine->gather_player_num : kDefaultMaxGatherPlayer;
	elem_info_.is_gather_disappear = pmine->is_gather_disappear;
	elem_info_.is_auto_refresh     = pmine->is_auto_refresh;
	elem_info_.refresh_time        = pmine->refresh_time;
}

bool MineImp::InitRuntimeData()
{
	return true;
}

void MineImp::OnHeartbeat()
{
	GatheringPlayerMap::iterator it = gathering_player_map_.begin();
	while (it != gathering_player_map_.end())
	{
		if (--(it->second.gather_timeout) < 0)
		{
			gathering_player_map_.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

void MineImp::OnPlayerLeaveView(RoleID playerid)
{
	RemoveGatheringPlayer(playerid, GS_INT32_MAX);
}

int MineImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_GATHER_REQUEST:
			{
				ASSERT(msg.source.IsPlayer());
				HandleGatherRequest(msg);
			}
			break;

		case GS_MSG_GATHER_CANCEL:
			{
				ASSERT(msg.source.IsPlayer());
				RemoveGatheringPlayer(msg.source.id, msg.param);
			}
			break;

		case GS_MSG_GATHER_COMPLETE:
			{
				ASSERT(msg.source.IsPlayer());
				HandleGatherComplete(msg);
			}
			break;

		default:
			ASSERT(false);
			return -1;
	}

	return 0;
}

void MineImp::HandleGatherRequest(const MSG& msg)
{
	CHECK_CONTENT_PARAM(msg, msg_gather_request);
	const msg_gather_request& param = *(msg_gather_request*)msg.content;
	const XID playerxid = msg.source;

	// 检查状态
	if (!CanGather())
	{
		matter_.SendMsg(GS_MSG_ERROR_MESSAGE, playerxid, G2C::ERR_OUT_OF_RANGE);
		return;
	}

	// 检查距离
	if (msg.pos.squared_distance(matter_.pos()) > kMatterMaxGatherDisSquare)
	{
		matter_.SendMsg(GS_MSG_ERROR_MESSAGE, playerxid, G2C::ERR_OUT_OF_RANGE);
		return;
	}

	// 检查采集人数限制
	if (gathering_player_map_.size() >= elem_info_.gather_player_num)
	{
		matter_.SendMsg(GS_MSG_ERROR_MESSAGE, playerxid, G2C::ERR_MINE_HAS_BEEN_LOCKED);
		return;
	}

	bool success = false;
	GatherInfo info;
	info.gather_seq_no  = param.gather_seq_no;
	info.gather_timeout = param.gather_timeout + kStdGatherTimeoutDelay;

	GatheringPlayerMap::iterator it = gathering_player_map_.find(playerxid.id);
	if (it != gathering_player_map_.end())
	{
		if (info.gather_seq_no > it->second.gather_seq_no)
		{
			it->second.gather_seq_no  = info.gather_seq_no;
			it->second.gather_timeout = info.gather_timeout;
			success = true;
		}
	}
	else
	{
		success = gathering_player_map_.insert(std::make_pair(playerxid.id, info)).second;
	}

	if (success)
	{
		msg_gather_reply reply_param;
		reply_param.gather_seq_no = info.gather_seq_no;
		reply_param.templ_id      = matter_.templ_id();
		matter_.SendMsg(GS_MSG_GATHER_REPLY, playerxid, &reply_param, sizeof(reply_param));
	}
}

void MineImp::RemoveGatheringPlayer(RoleID playerid, int32_t gather_seq_no)
{
	GatheringPlayerMap::iterator it = gathering_player_map_.find(playerid);
	if (it != gathering_player_map_.end())
	{
		if (it->second.gather_seq_no <= gather_seq_no)
		{
			gathering_player_map_.erase(it);
		}
	}
}

void MineImp::HandleGatherComplete(const MSG& msg)
{
    const XID& playerxid  = msg.source;
    int32_t gather_seq_no = msg.param;

	// 检查状态
	if (!CanGather())
	{
		matter_.SendMsg(GS_MSG_ERROR_MESSAGE, playerxid, G2C::ERR_MINE_HAS_BEEN_LOCKED);
		return;
	}

    // 检查距离
	if (msg.pos.squared_distance(matter_.pos()) > kMatterMaxGatherDisSquare)
	{
		matter_.SendMsg(GS_MSG_ERROR_MESSAGE, playerxid, G2C::ERR_OUT_OF_RANGE);
		return;
	}

	GatheringPlayerMap::iterator it = gathering_player_map_.find(playerxid.id);
	if (it == gathering_player_map_.end())
	{
		return;
	}

	if (it->second.gather_seq_no != gather_seq_no)
	{
		return;
	}

	msg_gather_result result_param;
	result_param.gather_seq_no = gather_seq_no;
	result_param.templ_id      = matter_.templ_id();
	matter_.SendMsg(GS_MSG_GATHER_RESULT, playerxid, &result_param, sizeof(result_param));

	// 采集完成先删除成功玩家
	RemoveGatheringPlayer(playerxid.id, gather_seq_no);

	// 通知所有其他玩家，采集失败
	GatheringPlayerMap::iterator it_b = gathering_player_map_.begin();
	for (; it_b != gathering_player_map_.end(); ++it_b)
	{
		XID xid;
		xid.id   = it_b->first;
		xid.type = XID::TYPE_PLAYER;
		matter_.SendMsg(GS_MSG_MINE_HAS_BEEN_ROB, xid, it_b->second.gather_seq_no);
	}

	// 设置zombie状态
	if (elem_info_.is_gather_disappear)
	{
        matter_.RebornAtOnce();
	}
}

bool MineImp::CanAutoReborn() const
{
	return elem_info_.is_auto_refresh;
}
	
int MineImp::GetRebornInterval()
{
	return elem_info_.refresh_time;
}

bool MineImp::CanGather() const
{
	if (matter_.state() == STATE_ZOMBIE || matter_.state() == STATE_REBORN)
	{
		return false;
	}

	return true;
}

void MineImp::SetBirthPlace(const A2DVECTOR& pos)
{
    elem_info_.birth_place = pos;
}

bool MineImp::OnMapTeleport()
{
    // 状态已经在matter里检查
    return true;
}

} // namespace gamed
