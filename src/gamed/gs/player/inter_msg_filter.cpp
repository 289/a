#include "inter_msg_filter.h"

#include "gs/global/dbgprt.h"
#include "gs/global/timer.h"

#include "player.h"


namespace gamed {

#define PD_ InterMsgFilter::MSG_OP_DENY
#define PA_ InterMsgFilter::MSG_OP_ACCEPT
#define PS_ InterMsgFilter::MSG_OP_SPECIAL_HANDLE

/*
0  STATE_NORMAL
1  STATE_OFFLINE
2  STATE_DEAD
3  STATE_COMBAT
*/

namespace {

// 这个列表定义了player处于正常登陆状态时处理的消息和状态的组合
int player_msg_state_table[][PlayerState::STATE_COUNT+1] =
{
	// MSG type                              0    1    2    3
	{GS_MSG_NULL,                            PD_, PD_, PD_, PD_},
	{GS_MSG_OBJ_HEARTBEAT,                   PA_, PS_, PA_, PA_},
	{GS_MSG_OBJ_ENTER_VIEW,                  PA_, PD_, PA_, PA_},
	{GS_MSG_OBJ_LEAVE_VIEW,                  PA_, PD_, PA_, PA_},
	{GS_MSG_OBJ_SESSION_REPEAT,              PA_, PD_, PA_, PA_},

	// 5
	{GS_MSG_OBJ_SESSION_END,                 PA_, PD_, PA_, PA_},
	{GS_MSG_ENTER_RULES_AREA,                PA_, PD_, PA_, PA_},
	{GS_MSG_LEAVE_RULES_AREA,                PA_, PD_, PA_, PA_},
	{GS_MSG_COMBAT_SEND_CMD,                 PD_, PD_, PD_, PA_},
	{GS_MSG_OBJ_TRIGGER_COMBAT,              PA_, PD_, PA_, PA_},

	// 10
	{GS_MSG_COMBAT_PVE_RESULT,               PD_, PD_, PD_, PA_},
	{GS_MSG_COMBAT_START,                    PD_, PD_, PD_, PA_},
	{GS_MSG_COMBAT_PVE_END,                  PD_, PD_, PD_, PA_},
	{GS_MSG_PLAYER_TRIGGER_COMBAT_RE,        PA_, PD_, PD_, PD_},
	{GS_MSG_HATE_YOU,                        PA_, PD_, PD_, PD_},

	// 15
	{GS_MSG_SERVICE_GREETING,                PA_, PD_, PD_, PD_},
	{GS_MSG_SERVICE_REQUEST,                 PA_, PD_, PD_, PD_},
	{GS_MSG_SERVICE_DATA,                    PA_, PD_, PD_, PD_},
	{GS_MSG_ERROR_MESSAGE,                   PA_, PD_, PD_, PD_},
	{GS_MSG_JOIN_TEAM,                       PA_, PD_, PA_, PA_},

	// 20
	{GS_MSG_LEAVE_TEAM,                      PA_, PD_, PA_, PA_},
	{GS_MSG_CHANGE_TEAM_LEADER,              PA_, PD_, PA_, PA_},
	{GS_MSG_CHANGE_TEAM_POS,                 PA_, PD_, PA_, PA_},
	{GS_MSG_TEAM_INFO,                       PA_, PD_, PA_, PA_},
	{GS_MSG_QUERY_TEAM_MEMBER,               PA_, PD_, PA_, PA_},

	// 25
	{GS_MSG_COMPANION_ENTER_COMBAT,          PA_, PD_, PD_, PD_},
	{GS_MSG_PLAYER_REGION_TRANSPORT,         PA_, PD_, PD_, PD_},
	{GS_MSG_PLAYER_SWITCH_ERROR,             PA_, PD_, PD_, PA_},
	{GS_MSG_GATHER_REPLY,                    PA_, PD_, PA_, PA_},
	{GS_MSG_GATHER_RESULT,                   PA_, PD_, PA_, PA_},

	// 30
	{GS_MSG_GATHER_REQUEST,                  PA_, PD_, PD_, PD_},
	{GS_MSG_GATHER_CANCEL,                   PA_, PD_, PA_, PA_},
	{GS_MSG_GATHER_COMPLETE,                 PA_, PD_, PD_, PD_},
	{GS_MSG_MINE_HAS_BEEN_ROB,               PA_, PD_, PD_, PD_},
	{GS_MSG_ADD_FILTER,                      PA_, PD_, PD_, PD_},

	// 35
	{GS_MSG_DEL_FILTER,                      PA_, PD_, PD_, PD_},
	{GS_MSG_SYS_TRIGGER_COMBAT,              PA_, PD_, PD_, PD_},
	{GS_MSG_QUERY_EXTPROP,                   PA_, PD_, PD_, PA_},
	{GS_MSG_DRAMA_GATHER,                    PA_, PD_, PD_, PD_},
	{GS_MSG_DRAMA_GATHER_MG_RESULT,          PA_, PD_, PD_, PD_},

	// 40
	{GS_MSG_INS_TRANSFER_PREPARE,            PA_, PD_, PD_, PD_},
	{GS_MSG_INS_TRANSFER_START,              PA_, PD_, PD_, PD_},
	{GS_MSG_QUERY_EQUIPCRC,                  PA_, PD_, PD_, PA_},
	{GS_MSG_QUERY_EQUIPMENT,                 PA_, PD_, PD_, PA_},
	{GS_MSG_GET_STATIC_ROLE_INFO,            PA_, PD_, PA_, PA_},
	
	// 45
	{GS_MSG_ENTER_INS_REPLY,                 PA_, PD_, PD_, PD_},
	{GS_MSG_WORLD_CLOSING,                   PA_, PD_, PA_, PA_},
	{GS_MSG_SERVICE_QUERY_CONTENT,           PA_, PD_, PD_, PD_},
	{GS_MSG_SERVICE_ERROR,                   PA_, PD_, PD_, PD_},
	{GS_MSG_SHOP_SHOPPING_FAILED,            PA_, PD_, PD_, PD_},

	// 50
	{GS_MSG_WORLD_DELIVER_TASK,              PA_, PD_, PA_, PA_},
	{GS_MSG_GET_ATTACH_REPLY,                PA_, PD_, PA_, PA_},
	{GS_MSG_DELETE_MAIL_REPLY,               PA_, PD_, PA_, PA_},
	{GS_MSG_SEND_MAIL_REPLY,                 PA_, PD_, PA_, PA_},
	{GS_MSG_ANNOUNCE_NEW_MAIL,               PA_, PD_, PA_, PA_},

	// 55
	{GS_MSG_QUERY_TEAM_COMBAT,               PA_, PD_, PA_, PA_},
	{GS_MSG_QUERY_TEAM_COMBAT_RE,            PA_, PD_, PD_, PD_},
	{GS_MSG_TEAMMEMBER_LOCALINS_REPLY,       PA_, PD_, PD_, PD_},
	{GS_MSG_QUIT_TEAM_LOCAL_INS,             PA_, PD_, PD_, PD_},
	{GS_MSG_TEAM_LOCAL_INS_INVITE,           PA_, PD_, PD_, PD_},

	// 60
	{GS_MSG_PLAYER_QUIT_INS,                 PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_INFO,                   PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_JOIN,                   PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_LEAVE,                  PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_CHANGE_POS,             PA_, PD_, PA_, PA_},

	// 65
	{GS_MSG_MAP_TEAM_CHANGE_LEADER,          PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_QUERY_MEMBER,           PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_STATUS_CHANGE,          PA_, PD_, PA_, PA_},
	{GS_MSG_INS_FINISH_RESULT,               PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_QUERY_PLAYER_INFO,           PA_, PD_, PA_, PA_},

    // 70
    {GS_MSG_TEAM_CROSS_INS_INVITE,           PA_, PD_, PD_, PD_},
    {GS_MSG_TEAM_CROSS_INS_INVITE_RE,        PA_, PD_, PD_, PD_},
	{GS_MSG_COMBAT_PVP_RESULT,               PD_, PD_, PD_, PA_},
	{GS_MSG_COMBAT_PVP_END,                  PD_, PD_, PD_, PA_},
    {GS_MSG_DUEL_REQUEST,                    PA_, PD_, PD_, PD_},

    // 75
    {GS_MSG_DUEL_REQUEST_RE,                 PA_, PD_, PD_, PD_},
    {GS_MSG_TEAMMATE_DUEL_REQUEST,           PA_, PD_, PD_, PD_},
    {GS_MSG_TEAMMATE_DUEL_REQUEST_RE,        PA_, PD_, PD_, PD_},
    {GS_MSG_DUEL_PREPARE,                    PA_, PD_, PD_, PD_},
    {GS_MSG_START_JOIN_DUEL_COMBAT,          PA_, PD_, PD_, PD_},

    // 80
	{GS_MSG_ENHANCE_FAILED,                  PA_, PD_, PD_, PD_},
    {GS_MSG_BG_TRANSFER_PREPARE,             PA_, PD_, PD_, PD_},
    {GS_MSG_ENTER_BG_REPLY,                  PA_, PD_, PD_, PD_},
    {GS_MSG_BG_TRANSFER_START,               PA_, PD_, PD_, PD_},
    {GS_MSG_NOTIFY_LANDMINE_INFO,            PA_, PA_, PA_, PA_},

    // 85
	{GS_MSG_PLAYER_QUIT_BG,                  PA_, PD_, PA_, PA_},
	{GS_MSG_BG_FINISH_RESULT,                PA_, PD_, PA_, PA_},
    {GS_MSG_MAP_TEAM_JOIN_TEAM,              PA_, PD_, PD_, PD_},
	{GS_MSG_GLOBAL_COUNTER_CHANGE,           PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_COUNTER_CHANGE,              PA_, PD_, PA_, PA_},

    // 90
	{GS_MSG_RE_PUNCH_CARD_HELP,              PA_, PD_, PA_, PA_},
	{GS_MSG_RE_PUNCH_CARD_HELP_REPLY,        PA_, PD_, PA_, PA_},
	{GS_MSG_RE_PUNCH_CARD_HELP_ERROR,        PA_, PD_, PA_, PA_},
	{GS_MSG_MAP_TEAM_TIDY_POS,               PA_, PD_, PA_, PA_},
	{GS_MSG_WORLD_BOSS_DEAD,                 PA_, PD_, PA_, PA_},

    // 95


	// GM 采用的消息
	{GS_MSG_GM_GETPOS,                       PA_, PA_, PA_, PA_},

	// MSG 的最大值
	{GS_MSG_MAX,                             PD_, PD_, PD_, PD_}
};

PlayerMsgDispatcher* msg_dispatcher_map[GS_MSG_MAX][PlayerState::STATE_COUNT];

int InitMsgDispatcherMap()
{
	size_t i = 0;
	while (true)
	{
		int msg = player_msg_state_table[i][0];
		if (msg == GS_MSG_MAX || msg < GS_MSG_NULL) break;
		ASSERT(msg < GS_MSG_MAX);
		for (size_t j = 0; j < PlayerState::STATE_COUNT; ++j)
		{
			PlayerMsgDispatcher* dis = NULL;
			switch (player_msg_state_table[i][j+1])
			{
			case PD_:
				dis = PlayerMsgDispatcher::DENY();
				break;
			case PA_:
				dis = PlayerMsgDispatcher::ACCEPT(); 
				break;
			case PS_:
				dis = PlayerMsgDispatcher::SPECIAL_HANDLE();
				break;
			default:
				ASSERT(false && "不认识的类型");
			}
			msg_dispatcher_map[msg][j] = dis;
		}
		++i;
	}

	return 0;
}

class StateNormalMsgFilter : public InterMsgFilter
{
public:
	virtual int HandleMessage(Player* a_player, const MSG& a_msg)
	{
		switch (a_msg.message)
		{
			default:
				ASSERT(false && "未知的MSG类型");
				return -1;
		};
		return 0;
	}
};

class StateOfflineMsgFilter : public InterMsgFilter
{
public:
	virtual int HandleMessage(Player* a_player, const MSG& a_msg)
	{
		switch (a_msg.message)
		{
			case GS_MSG_OBJ_HEARTBEAT:
				{
					time_t cur_time = g_timer->GetSysTime();
					// rpc timeout check
					a_player->CheckRpcTimeout(cur_time);
				}
				break;
			default:
				ASSERT(false && "不认识的MSG类型");
				return -1;
		}
		return 0;
	}
};

class StateDeadMsgFilter : public InterMsgFilter
{
public:
	virtual int HandleMessage(Player* a_player, const MSG& a_msg)
	{
		return 0;
	}
};

class StateCombatMsgFilter : public InterMsgFilter
{
public:
	virtual int HandleMessage(Player* a_player, const MSG& a_msg)
	{
		return 0;
	}
};

///
/// accept, deny, special_handle
///
class MsgDispatcherAccept : public PlayerMsgDispatcher
{
public:
	virtual int DispatchMsg(Player* a_player, InterMsgFilter* a_special, const MSG& a_msg)
	{
		return a_player->MessageHandler(a_msg);
	}
};

class MsgDispatcherDeny : public PlayerMsgDispatcher
{
public:
	virtual int DispatchMsg(Player* a_player, InterMsgFilter* a_special, const MSG& a_msg)
	{
		return 0;
	}
};

class MsgDispatcherSpecialHandle : public PlayerMsgDispatcher
{
public:
	virtual int DispatchMsg(Player* a_player, InterMsgFilter* a_special, const MSG& a_msg)
	{
		return a_special->HandleMessage(a_player, a_msg);
	}
};

} // namespace Anonymous


///
/// InterMsgFilter: 初始化顺序和状态定义顺序一致
///
static StateNormalMsgFilter  s_state_normal_msg_filter;
static StateOfflineMsgFilter s_state_offline_msg_filter;
static StateDeadMsgFilter    s_state_dead_msg_filter;
static StateCombatMsgFilter  s_state_combat_msg_filter;
InterMsgFilter* InterMsgFilter::s_special_filter_[PlayerState::STATE_COUNT] = 
{
	&s_state_normal_msg_filter,
	&s_state_offline_msg_filter,
	&s_state_dead_msg_filter,
	&s_state_combat_msg_filter,
};

int InterMsgFilter::DispatchMessage(Player* player, const MSG& msg)
{
	PlayerMsgDispatcher* dispatcher = PlayerMsgDispatcher::GetMsgDispatcher(msg, player->GetState());
	int rst = -1;
	if (dispatcher)
	{
		rst = dispatcher->DispatchMsg(player, s_special_filter_[player->GetState()], msg);
	}
	else
	{
		__PRINTF("MSG:%d no found in msg_state_table[][]\n", msg.message);
	}

	return rst;
}

int InterMsgFilter::HandleMessage(Player* a_player, const MSG& a_msg)
{
	return -1;
}


///
/// PlayerMsgDispatcher
///
static MsgDispatcherDeny          s_msg_deny_handler;
static MsgDispatcherAccept        s_msg_accept_handler;
static MsgDispatcherSpecialHandle s_msg_spec_handler;
// 以下静态成员初始化顺序有要求，s_init_msg_map_排最后
PlayerMsgDispatcher* PlayerMsgDispatcher::s_deny_           = &s_msg_deny_handler;
PlayerMsgDispatcher* PlayerMsgDispatcher::s_accept_         = &s_msg_accept_handler;
PlayerMsgDispatcher* PlayerMsgDispatcher::s_special_handle_ = &s_msg_spec_handler;
int PlayerMsgDispatcher::s_init_msg_map_                    = InitMsgDispatcherMap();

PlayerMsgDispatcher* PlayerMsgDispatcher::DENY()
{
	return s_deny_;
}

PlayerMsgDispatcher* PlayerMsgDispatcher::ACCEPT()
{
	return s_accept_;
}

PlayerMsgDispatcher* PlayerMsgDispatcher::SPECIAL_HANDLE()
{
	return s_special_handle_;
}

PlayerMsgDispatcher* PlayerMsgDispatcher::GetMsgDispatcher(const MSG& msg, PlayerState::MaskType state)
{
	MSG::MsgType idx = msg.message;
	if (idx >= GS_MSG_MAX || idx < GS_MSG_NULL)
	{
		return DENY();
	}

	return msg_dispatcher_map[idx][state];
}

int PlayerMsgDispatcher::DispatchMsg(Player* a_player, InterMsgFilter* a_special, const MSG& a_msg)
{
	return -1;
}

} // namespace gamed
