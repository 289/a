#include "cmdprepare.h"

#include "shared/base/strtoken.h"

#include "gamed/client_proto/C2G_proto.h"


namespace gamed {

namespace {

struct cmd_node_t
{
	int command;
	const char* valid_state;
	const char* invalid_state; // 这些是要禁止的状态，和valid状态同时使用

	const char* spec_state;
	const char* spec_handler;
};

using namespace C2G;
typedef PlayerState T;
#define ALWAYS_INVALID_STATE (1 << T::STATE_OFFLINE)

struct MaskList
{
	const char*            name;
	PlayerState::MaskType  mask;
};

static MaskList mask_list[] =
{
	{"all",            0xFFFFFFFF                    },
	{"null",           0                             },
	{"normal",         (1 << T::STATE_NORMAL)        },
	{"dead",           (1 << T::STATE_DEAD)          },
	{"combat",         (1 << T::STATE_COMBAT)        },

	{NULL,             0                             },
};

static cmd_node_t cmd_list[] =
{
// 0
	{LOGOUT,                       "normal dead",           "null",               "null",            ""    },
	{START_MOVE,                   "normal",                "dead combat",        "null",            ""    },
	{MOVE_CONTINUE,                "normal combat",         "dead",               "null",            ""    },
	{STOP_MOVE,                    "normal combat",         "dead",               "null",            ""    },
	{MOVE_CHANGE,                  "normal",                "dead combat",        "null",            ""    },

// 5
	{GET_ALL_DATA,                 "all",                   "null",               "null",            ""    },
	{GET_ITEM_DATA,                "normal",                "dead combat",        "null",            ""    },
	{GET_ITEM_LIST_BRIEF,          "normal",                "dead combat",        "null",            ""    },
	{GET_ITEM_LIST_DETAIL,         "normal",                "dead combat",        "null",            ""    },
	{DROP_INVENTORY_ITEM,          "normal",                "dead combat",        "null",            ""    },

// 10
	{MOVE_INVENTORY_ITEM,          "normal",                "dead combat",        "null",            ""    },
	{SELL_INVENTORY_ITEM,          "normal",                "dead combat",        "null",            ""    },
	{EXCHANGE_INVENTORY_ITEM,      "normal",                "dead combat",        "null",            ""    },
	{TIDY_INVENTORY,               "normal",                "dead combat",        "null",            ""    },
	{EQUIP_ITEM,                   "normal",                "dead combat",        "null",            ""    },

// 15
	{UNDO_EQUIP,                   "normal",                "dead combat",        "null",            ""    },
	{USE_ITEM,                     "normal",                "dead combat",        "null",            ""    },
	{REFINE_EQUIP,                 "normal",                "dead combat",        "null",            ""    },
	{COMBAT_PLAYER_JOIN,           "normal",                "dead combat",        "null",            ""    },
	{COMBAT_PLAYER_TRIGGER,        "normal",                "dead combat",        "null",            ""    },

// 20
	{COMBAT_SELECT_SKILL,          "combat",                "dead normal",        "null",            ""    },
	{COMBAT_PET_ATTACK,            "combat",                "dead normal",        "null",            ""    },
	{FULL_PET_POWER,               "normal",                "dead combat",        "null",            ""    },
	{SERVICE_HELLO,                "normal",                "dead combat",        "null",            ""    },
	{SERVICE_SERVE,                "normal",                "dead combat",        "null",            ""    },

// 25
	{LEVELUP_PET_BLOODLINE,        "normal",                "dead combat",        "null",            ""    },
	{LEVELUP_PET_POWER_CAP,        "normal",                "dead combat",        "null",            ""    },
	{SET_COMBAT_PET,               "normal",                "dead combat",        "null",            ""    },
	{LEARN_SKILL,                  "normal",                "dead combat",        "null",            ""    },
	{LEVELUP_TALENT,               "normal",                "dead combat",        "null",            ""    },

// 30
	{TASK_NOTIFY,                  "all",                   "dead",               "null",            ""    },
	{TRANSFER_PREPARE_FINISH,      "normal",                "dead combat",        "null",            ""    },
	{SELECT_RESURRECT_POS,         "dead",                  "normal combat",      "null",            ""    },
	{JOIN_TEAM,                    "all",                   "null",               "null",            ""    },
	{JOIN_TEAM_RES,                "all",                   "null",               "null",            ""    },

// 35
	{CHANGE_BUDDY_TEAM_POS,        "normal",                "dead combat",        "null",            ""    },
	{GATHER_MATERIAL,              "normal",                "dead combat",        "null",            ""    },
	{GATHER_MINIGAME_RESULT,       "normal",                "dead combat",        "null",            ""    },
	{OPEN_CAT_VISION,              "normal",                "dead combat",        "null",            ""    },
	{CLOSE_CAT_VISION,             "normal",                "dead combat",        "null",            ""    },

//40
	{DRAMA_TRIGGER_COMBAT,         "normal",                "dead combat",        "null",            ""    },
	{DRAMA_SERVICE_SERVE,          "normal",                "dead combat",        "null",            ""    },
	{DRAMA_GATHER_MATERIAL,        "normal",                "dead combat",        "null",            ""    },
	{DRAMA_GATHER_MINIGAME_RESULT, "normal",                "dead combat",        "null",            ""    },
	{SWITCH_SKILL,                 "normal",                "dead combat",        "null",            ""    },

// 45
	{GET_ELSE_PLAYER_EXTPROP,      "normal combat",         "null",               "null",            ""    },
	{CHAT_MSG,					   "all",					"null",				  "null",            ""    },
	{TASK_REGION_TRANSFER,         "normal",                "dead combat",        "null",            ""    },
	{GET_ELSE_PLAYER_EQUIPCRC,     "normal combat",         "dead",               "null",            ""    },
	{GET_ELSE_PLAYER_EQUIPMENT,    "normal combat",         "dead",               "null",            ""    },

// 50
	{TASK_CHANGE_NAME,             "normal",                "dead combat",        "null",            ""    },
	{GET_STATIC_ROLE_INFO,         "all",                   "null",               "null",            ""    },
	{GET_INSTANCE_DATA,            "all",                   "null",               "null",            ""    },
	{UI_TASK_REQUEST,              "normal",                "dead combat",        "null",            ""    },
	{QUERY_NPC_ZONE_INFO,          "all",                   "null",               "null",            ""    },

// 55
	{OPEN_MALL,                    "normal",                "null",               "null",            ""    },
	{QUERY_MALL_GOODS_DETAIL,      "normal",                "null",               "null",            ""    },
	{MALL_SHOPPING,                "normal",                "null",               "null",            ""    },
	{SERVICE_GET_CONTENT,          "normal",                "dead combat",        "null",            ""    },
	{QUERY_SERVER_TIME,            "normal dead combat",    "null",               "null",            ""    },

// 60
	{GET_MAIL_LIST,                "normal",                "null",               "null",            ""    },
	{GET_MAIL_ATTACH,			   "normal",                "null",               "null",            ""    },
	{DELETE_MAIL,                  "normal",                "null",               "null",            ""    },
	{TAKEOFF_MAIL_ATTACH,          "normal",                "null",               "null",            ""    },
	{SEND_MAIL,                    "normal",                "null",               "null",            ""    },

// 65
	{TEAM_LOCAL_INS_INVITE,        "normal",                "null",               "null",            ""    },
	{TEAM_CROSS_INS_REQUEST,       "normal",                "null",               "null",            ""    },
	{LAUNCH_TEAM_LOCAL_INS,        "normal",                "null",               "null",            ""    },
	{QUIT_TEAM_LOCAL_INS_ROOM,     "normal",                "null",               "null",            ""    },
	{QUIT_TEAM_CROSS_INS_ROOM,     "normal",                "null",               "null",            ""    },
	
// 70
	{LAUNCH_SOLO_LOCAL_INS,        "normal",                "null",               "null",            ""    },
	{TEAM_LOCAL_INS_INVITE_RE,     "normal",                "null",               "null",            ""    },
	{TEAM_CROSS_INS_READY,         "normal",                "null",               "null",            ""    },
	{PLAYER_QUIT_INSTANCE,         "normal",                "null",               "null",            ""    },
	{MAP_TEAM_CHANGE_POS,          "normal",                "null",               "null",            ""    },
	
// 75
	{MAP_TEAM_CHANGE_LEADER,       "normal",                "null",               "null",            ""    },
	{MAP_TEAM_QUERY_MEMBER,        "normal",                "null",               "null",            ""    },
	{TASK_TRANSFER_GENDER_CLS,     "normal",                "dead combat",        "null",            ""    },
	{TASK_INS_TRANSFER,            "normal",                "dead combat",        "null",            ""    },
	{MOVE_COMBAT_PET,              "normal",                "dead combat",        "null",            ""    },

// 80
	{AUCTION_ITEM,                 "normal",                "dead combat",        "null",            ""    },
	{AUCTION_CANCEL,               "normal",                "dead combat",        "null",            ""    },
    {AUCTION_BUYOUT,               "normal",                "dead combat",        "null",            ""    },
	{AUCTION_BID,                  "normal",                "dead combat",        "null",            ""    },
	{AUCTION_QUERY_LIST,           "normal",                "dead combat",        "null",            ""    },

// 85
	{AUCTION_QUERY_BID_PRICE,      "normal",                "dead combat",        "null",            ""    },
	{AUCTION_QUERY_DETAIL,         "normal",                "dead combat",        "null",            ""    },
	{LEVELUP_CARD,                 "normal",                "dead combat",        "null",            ""    },
	{GET_SELF_AUCTION_ITEM,        "normal",                "dead combat",        "null",            ""    },
	{GET_SELF_BID_ITEM,            "normal",                "dead combat",        "null",            ""    },

// 90
	{ADD_FRIEND,                   "normal",                "dead combat",        "null",            ""    },
	{DELETE_FRIEND,                "normal",                "dead combat",        "null",            ""    },
	{QUERY_ROLEINFO,               "normal",                "dead combat",        "null",            ""    },
	{CONVENE_TEAMMATE,             "normal",                "dead combat",        "null",            ""    },
	{CONVENE_RESPONSE,             "normal",                "dead combat",        "null",            ""    },

// 95
	{TEAM_CROSS_INS_INVITE_RE,     "normal",                "null",               "null",            ""    },
	{DUEL_REQUEST,                 "normal",                "dead combat",        "null",            ""    },
	{DUEL_REQUEST_RE,              "normal",                "dead combat",        "null",            ""    },
	{TEAMMATE_DUEL_REQUEST_RE,     "normal",                "dead combat",        "null",            ""    },
	{SWITCH_TITLE,                 "normal",                "dead combat",        "null",            ""    },

// 100
	{QUERY_PRIMARY_PROP_RF,        "normal",                "dead combat",        "null",            ""    },
	{BUY_PRIMARY_PROP_RF_ENERGY,   "normal",                "dead combat",        "null",            ""    },
	{PRIMARY_PROP_REINFORCE,       "normal",                "dead combat",        "null",            ""    },
	{ACHIEVE_NOTIFY,               "normal",                "dead combat",        "null",            ""    },
	{OPEN_ENHANCE_SLOT,            "normal",                "dead combat",        "null",            ""    },

// 105
	{PROTECT_ENHANCE_SLOT,         "normal",                "dead combat",        "null",            ""    },
	{UNPROTECT_ENHANCE_SLOT,       "normal",                "dead combat",        "null",            ""    },
	{ACTIVATE_SPARK,               "normal",                "dead combat",        "null",            ""    },
	{PLAYER_QUIT_BATTLEGROUND,     "normal",                "null",               "null",            ""    },
	{GET_BATTLEGROUND_DATA,        "all",                   "null",               "null",            ""    },

// 110
	{EQUIP_CARD,                   "normal",                "dead combat",        "null",            ""    },
	{MAP_TEAM_JOIN_TEAM,           "normal",                "dead combat",        "null",            ""    },
	{MAP_TEAM_LEAVE_TEAM,          "normal",                "dead combat",        "null",            ""    },
	{MAP_TEAM_KICKOUT_MEMBER,      "normal",                "dead combat",        "null",            ""    },
	{MAP_TEAM_JOIN_TEAM_RES,       "normal",                "dead combat",        "null",            ""    },

// 115
	{UPDATE_TASK_STORAGE,          "normal",                "dead combat",        "null",            ""    },
	{ARENA_OD_QUERY_DATA,          "normal",                "dead combat",        "null",            ""    },
	{ARENA_OD_INVITE_ASSIST,       "normal",                "dead combat",        "null",            ""    },
	{ARENA_OD_ASSIST_SELECT,       "normal",                "dead combat",        "null",            ""    },
	{ARENA_OD_COMBAT_START,        "normal",                "dead combat",        "null",            ""    },

// 120
	{PLAYER_PUNCH_CARD,            "normal",                "dead combat",        "null",            ""    },
	{GAIN_PUNCH_CARD_AWARD,        "normal",                "dead combat",        "null",            ""    },
	{QUERY_INSTANCE_RECORD,        "normal",                "dead combat",        "null",            ""    },
	{QUERY_PUNCH_CARD_DATA,        "normal",                "dead combat",        "null",            ""    },
	{JOIN_GEVENT,                  "normal",                "dead combat",        "null",            ""    },

// 125
	{RE_PUNCH_CARD_HELP_RE,        "normal",                "dead combat",        "null",            ""    },
	{ITEM_DISASSEMBLE,             "normal",                "dead combat",        "null",            ""    },
	{MOUNT_MOUNT,                  "normal",                "dead combat",        "null",            ""    },
	{MOUNT_EXCHANGE,               "normal",                "dead combat",        "null",            ""    },
	{MOUNT_EQUIP_LEVELUP,          "normal",                "dead combat",        "null",            ""    },

// 130
	{GET_PARTICIPATION_AWARD,      "normal",                "dead combat",        "null",            ""    },
	{BOSS_CHALLENGE,               "normal",                "dead combat",        "null",            ""    },
	{GET_BOSS_CHALLENGE_AWARD,     "normal",                "dead combat",        "null",            ""    },
	{GET_CLEAR_CHALLENGE_AWARD,    "normal",                "dead combat",        "null",            ""    },
	{QUERY_WORLD_BOSS_RECORD,      "normal",                "dead combat",        "null",            ""    },

// 135
	{TASK_BG_TRANSFER,             "normal",                "dead combat",        "null",            ""    },

// debug command
  // 0
	{DEBUG_COMMON_CMD,             "null",                  "null",               "all",             "DebugHandler"  },
	{DEBUG_GAIN_ITEM,              "null",                  "null",               "all",             "DebugHandler"  },
	{DEBUG_CLEAN_ALL_ITEM,         "null",                  "null",               "all",             "DebugHandler"  },
	{DEBUG_CHANGE_PLAYER_POS,      "null",                  "null",               "all",             "DebugHandler"  },
	{DEBUG_CHANGE_FIRST_NAME,      "null",                  "null",               "all",             "DebugHandler"  },

  // 5

	{-1,                           NULL,                    NULL,                 NULL,              NULL  },
};

} // Anonymous


///
/// CmdPacketDispatcher
/// 
CmdPacketDispatcher::CmdPacketDispatcher()
{
	InitCommandList();
}

CmdPacketDispatcher::~CmdPacketDispatcher()
{
	CmdDefineMap::iterator it = standard_cmd_.begin();
	for (; it != standard_cmd_.end(); ++it)
	{
		DELETE_SET_NULL(it->second.spec_handler);
	}
	standard_cmd_.clear();
}

PlayerState::MaskType CmdPacketDispatcher::GetCommandState(const char* str)
{
	PlayerState::MaskType state = 0;

	std::string strbuf = str;
	shared::StrToken tok;
	const char* delim = " \t";
	std::vector<std::string> token_vec;
	if (tok.GetTokenData<std::string>(strbuf.c_str(), delim, token_vec))
	{
		__PRINTF("CmdPacket GetCommandState() error!");
		return state;
	}

	for (size_t i = 0; i < token_vec.size(); ++i)
	{
		for (size_t k = 0; mask_list[k].name; ++k)
		{
			if(strcmp(token_vec[i].c_str(), mask_list[k].name) == 0)
			{
				state |= mask_list[k].mask;
				break;
			}
		}
	}

	return state;
}

bool CmdPacketDispatcher::InitCommandList()
{
	ASSERT(standard_cmd_.size() == 0);
	int index = 0;
	for (; ; index++)
	{
		if (cmd_list[index].command < 0 || NULL == cmd_list[index].valid_state) break;

		size_t cmd     = cmd_list[index].command;
		cmd_define ent = {0, 0, 0, NULL};
		ent.state_mask         = GetCommandState(cmd_list[index].valid_state);
		ent.exclude_state_mask = GetCommandState(cmd_list[index].invalid_state) | ALWAYS_INVALID_STATE;
		ent.spec_state         = GetCommandState(cmd_list[index].spec_state);
		if (ent.spec_state)
		{
			if (strcmp(cmd_list[index].spec_handler, "UnlockHandler") == 0)
			{
				ent.spec_handler = new ExecCmd1();
			}
			else if (strcmp(cmd_list[index].spec_handler, "InvalidHandler") == 0)
			{
				ent.spec_handler = new ExecCmd2();
			}
			else if(strcmp(cmd_list[index].spec_handler, "DebugHandler") == 0)
			{
				ent.spec_handler = new ExecCmd3();
			}
			else
			{
				ASSERT(false);
			}
		}
		ASSERT(standard_cmd_.insert(std::pair<int32_t, cmd_define>(cmd, ent)).second);
	}
	
	return true;
}

} // namespace gamed
