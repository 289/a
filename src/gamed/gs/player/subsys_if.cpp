#include "subsys_if.h"

#include "gs/global/dbgprt.h"
#include "gs/player/subsys/player_inventory.h"
#include "gs/player/subsys/player_combat.h"
#include "gs/player/subsys/player_skill.h"
#include "gs/player/subsys/player_task.h"
#include "gs/player/subsys/player_team.h"
#include "gs/player/subsys/player_service.h"
#include "gs/player/subsys/player_pet.h"
#include "gs/player/subsys/player_transfer.h"
#include "gs/player/subsys/player_instance.h"
#include "gs/player/subsys/player_buddy.h"
#include "gs/player/subsys/player_buff.h"
#include "gs/player/subsys/player_gather.h"
#include "gs/player/subsys/player_drama.h"
#include "gs/player/subsys/player_chat.h"
#include "gs/player/subsys/player_mall.h"
#include "gs/player/subsys/player_mail.h"
#include "gs/player/subsys/player_map_team.h"
#include "gs/player/subsys/player_talent.h"
#include "gs/player/subsys/player_auction.h"
#include "gs/player/subsys/player_friend.h"
#include "gs/player/subsys/player_cooldown.h"
#include "gs/player/subsys/player_duel.h"
#include "gs/player/subsys/player_reputation.h"
#include "gs/player/subsys/player_prop_reinforce.h"
#include "gs/player/subsys/player_achieve.h"
#include "gs/player/subsys/player_enhance.h"
#include "gs/player/subsys/player_title.h"
#include "gs/player/subsys/player_star.h"
#include "gs/player/subsys/player_battleground.h"
#include "gs/player/subsys/player_landmine.h"
#include "gs/player/subsys/player_card.h"
#include "gs/player/subsys/player_arena.h"
#include "gs/player/subsys/player_bw_list.h"
#include "gs/player/subsys/player_counter.h"
#include "gs/player/subsys/player_punch_card.h"
#include "gs/player/subsys/player_gevent.h"
#include "gs/player/subsys/player_mount.h"
#include "gs/player/subsys/player_participation.h"
#include "gs/player/subsys/player_boss_challenge.h"
#include "gs/player/subsys/player_world_boss.h"


namespace gamed {

SHARED_STATIC_ASSERT(SUBSYS_MASK_COUNT > PlayerSubSystem::SUB_SYS_TYPE_MAX);

///
/// SubSysIf
/// 
SubSysIf::SubSysIf(Player& player)
	: player_(player)
{
}

SubSysIf::~SubSysIf()
{
	ASSERT(!subsys_vec_.size());
	ASSERT(!dispatch_map_.size());
}

SubSysMask SubSysIf::GetRequiredSubSys()
{
    SubSysMask tmpbitset;
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_INVENTORY);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_COMBAT);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_SKILL);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_SERVICE);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_PET);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_TRANSFER);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_TASK);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_INSTANCE); 
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_BUDDY);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_BUFF);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_GATHER);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_DRAMA);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_CHAT);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_MALL);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_MAIL);
	tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_TALENT); 
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_AUCTION);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_FRIEND);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_COOLDOWN); 
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_DUEL);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_REPUTATION);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_PROP_REINFORCE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_ACHIEVE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_ENHANCE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_TITLE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_STAR);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_BATTLEGROUND);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_LANDMINE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_CARD);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_ARENA);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_BW_LIST);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_PLAYER_COUNTER);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_PUNCH_CARD);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_GEVENT);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_MOUNT);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_PARTICIPATION);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_BOSS_CHALLENGE);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_WORLD_BOSS);
    return tmpbitset;
}

SubSysMask SubSysIf::GetDynamicSubSys()
{
    SubSysMask tmpbitset;
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_TEAM);
    tmpbitset.set(PlayerSubSystem::SUB_SYS_TYPE_MAP_TEAM);
    return tmpbitset;
}

bool SubSysIf::InitSubSys()
{
	/*创建默认子系统*/
	ASSERT(subsys_vec_.size() == 0);
	CreateMultiSubSys(GetRequiredSubSys());
	return true;
}

bool SubSysIf::InitRuntimeSubSys(const SubSysMask& m)
{
	/**
	 * 创建玩家独有的子系统
	 * 注意: 部分默认子系统已创建
	 */
	SubSysMask required_mask = GetRequiredSubSys();
	SubSysMask mask = m & (~required_mask);
	CreateMultiSubSys(mask);
	return true;
}

bool SubSysIf::AddSubSys(SubSysType type)
{
	SubSysVec::iterator it = std::find_if(subsys_vec_.begin(), subsys_vec_.end(), SubSysFinder(type));
	ASSERT(it == subsys_vec_.end());

	PlayerSubSystem* pSubSys = CreateSubSys(type);
	if (NULL == pSubSys) return false;
	subsys_vec_.push_back(pSubSys);
    ASSERT(!subsys_mask_.test(type));
	subsys_mask_.set(type);
	return true;
}

bool SubSysIf::AddSubSys(PlayerSubSystem* pSubSys)
{
	ASSERT(pSubSys);
	SubSysType type = pSubSys->GetType();
	SubSysVec::iterator it = std::find_if(subsys_vec_.begin(), subsys_vec_.end(), SubSysFinder(type));
	ASSERT(it == subsys_vec_.end());

	subsys_vec_.push_back(pSubSys);
    ASSERT(!subsys_mask_.test(type));
	subsys_mask_.set(type);
	return true;
}

#define CASE_CREATE_SPEC_SUBSYS(class_name, TYPE) \
	case PlayerSubSystem::TYPE: \
	{ \
		pSubSys = new class_name(player_); \
		ASSERT(pSubSys); \
		pSubSys->Initialize(); \
	} \
	break;

PlayerSubSystem* SubSysIf::CreateSubSys(SubSysType type)
{
	PlayerSubSystem* pSubSys = NULL;
	switch (type)
	{
		CASE_CREATE_SPEC_SUBSYS(PlayerInventory, SUB_SYS_TYPE_INVENTORY);
		CASE_CREATE_SPEC_SUBSYS(PlayerCombat, SUB_SYS_TYPE_COMBAT);
		CASE_CREATE_SPEC_SUBSYS(PlayerSkill, SUB_SYS_TYPE_SKILL);
		CASE_CREATE_SPEC_SUBSYS(PlayerService, SUB_SYS_TYPE_SERVICE);
		CASE_CREATE_SPEC_SUBSYS(PlayerTeam, SUB_SYS_TYPE_TEAM);
		CASE_CREATE_SPEC_SUBSYS(PlayerPet, SUB_SYS_TYPE_PET);
		CASE_CREATE_SPEC_SUBSYS(PlayerTransfer, SUB_SYS_TYPE_TRANSFER);
		CASE_CREATE_SPEC_SUBSYS(PlayerTask, SUB_SYS_TYPE_TASK);
		CASE_CREATE_SPEC_SUBSYS(PlayerInstance, SUB_SYS_TYPE_INSTANCE);
		CASE_CREATE_SPEC_SUBSYS(PlayerBuddy, SUB_SYS_TYPE_BUDDY);
		CASE_CREATE_SPEC_SUBSYS(PlayerBuff, SUB_SYS_TYPE_BUFF);
		CASE_CREATE_SPEC_SUBSYS(PlayerGather, SUB_SYS_TYPE_GATHER);
		CASE_CREATE_SPEC_SUBSYS(PlayerDrama, SUB_SYS_TYPE_DRAMA);
		CASE_CREATE_SPEC_SUBSYS(PlayerChat, SUB_SYS_TYPE_CHAT);
		CASE_CREATE_SPEC_SUBSYS(PlayerMall, SUB_SYS_TYPE_MALL);
		CASE_CREATE_SPEC_SUBSYS(PlayerMail, SUB_SYS_TYPE_MAIL);
		CASE_CREATE_SPEC_SUBSYS(PlayerMapTeam, SUB_SYS_TYPE_MAP_TEAM);
		CASE_CREATE_SPEC_SUBSYS(PlayerTalent, SUB_SYS_TYPE_TALENT);
        CASE_CREATE_SPEC_SUBSYS(PlayerAuction, SUB_SYS_TYPE_AUCTION);
        CASE_CREATE_SPEC_SUBSYS(PlayerFriend, SUB_SYS_TYPE_FRIEND);
        CASE_CREATE_SPEC_SUBSYS(PlayerCoolDown, SUB_SYS_TYPE_COOLDOWN);
        CASE_CREATE_SPEC_SUBSYS(PlayerDuel, SUB_SYS_TYPE_DUEL);
        CASE_CREATE_SPEC_SUBSYS(PlayerReputation, SUB_SYS_TYPE_REPUTATION);
        CASE_CREATE_SPEC_SUBSYS(PlayerPropReinforce, SUB_SYS_TYPE_PROP_REINFORCE);
        CASE_CREATE_SPEC_SUBSYS(PlayerAchieve, SUB_SYS_TYPE_ACHIEVE);
        CASE_CREATE_SPEC_SUBSYS(PlayerEnhance, SUB_SYS_TYPE_ENHANCE);
        CASE_CREATE_SPEC_SUBSYS(PlayerTitle, SUB_SYS_TYPE_TITLE);
        CASE_CREATE_SPEC_SUBSYS(PlayerStar, SUB_SYS_TYPE_STAR);
        CASE_CREATE_SPEC_SUBSYS(PlayerBattleGround, SUB_SYS_TYPE_BATTLEGROUND);
        CASE_CREATE_SPEC_SUBSYS(PlayerLandmine, SUB_SYS_TYPE_LANDMINE);
        CASE_CREATE_SPEC_SUBSYS(PlayerCard, SUB_SYS_TYPE_CARD);
        CASE_CREATE_SPEC_SUBSYS(PlayerArena, SUB_SYS_TYPE_ARENA);
        CASE_CREATE_SPEC_SUBSYS(PlayerBWList, SUB_SYS_TYPE_BW_LIST);
        CASE_CREATE_SPEC_SUBSYS(PlayerCounter, SUB_SYS_TYPE_PLAYER_COUNTER);
        CASE_CREATE_SPEC_SUBSYS(PlayerPunchCard, SUB_SYS_TYPE_PUNCH_CARD);
        CASE_CREATE_SPEC_SUBSYS(PlayerGevent, SUB_SYS_TYPE_GEVENT);
        CASE_CREATE_SPEC_SUBSYS(PlayerMount, SUB_SYS_TYPE_MOUNT);
        CASE_CREATE_SPEC_SUBSYS(PlayerParticipation, SUB_SYS_TYPE_PARTICIPATION);
        CASE_CREATE_SPEC_SUBSYS(PlayerBossChallenge, SUB_SYS_TYPE_BOSS_CHALLENGE);
        CASE_CREATE_SPEC_SUBSYS(PlayerWorldBoss, SUB_SYS_TYPE_WORLD_BOSS);
		default:
		{
			ASSERT(false);
			return NULL;
		}
	}
	return pSubSys;
}

PlayerSubSystem* SubSysIf::QuerySubSys(SubSysType type)
{
	SubSysVec::iterator it = std::find_if(subsys_vec_.begin(), subsys_vec_.end(), SubSysFinder(type));
	return it != subsys_vec_.end() ? *it : NULL;
}

void SubSysIf::Release()
{
	DispatchMap::iterator it_map = dispatch_map_.begin();
	for (; it_map != dispatch_map_.end(); ++it_map)
	{
		DELETE_SET_NULL(it_map->second);
	}
	dispatch_map_.clear();

	SubSysVec::iterator it_vec = subsys_vec_.begin();
	for (; it_vec != subsys_vec_.end(); ++it_vec)
	{
		(*it_vec)->OnRelease();
		DELETE_SET_NULL(*it_vec);
	}
	subsys_vec_.clear();

	msg_dispatch_map_.clear();
	subsys_mask_.reset();
}

bool SubSysIf::SaveToDB(PlayerData* pdata)
{
	return pdata->ForeachPlayerAttr(BIND_MEM_CB(&SubSysIf::SavePlayerAttrToDB, this));
}

bool SubSysIf::LoadFromDB(const PlayerData& data)
{
	subsys_mask_ = GetRequiredSubSys();
	if (!InitRuntimeSubSys(subsys_mask_))
	{
		LOG_ERROR << "InitRuntimeSubSys() error in SubSysIf::LoadFromDB()";
		return false;
	}
	return data.ForeachPlayerAttr(BIND_MEM_CB(&SubSysIf::LoadPlayerAttrFromDB, this));
}

bool SubSysIf::SavePlayerAttrToDB(ObjectAttrPacket* pattr)
{
	DispatchMap::const_iterator it = dispatch_map_.find(pattr->GetType());
	if (it != dispatch_map_.end())
	{
		return it->second->OnSaveToDB(pattr);
	}
	return true;
}

bool SubSysIf::LoadPlayerAttrFromDB(ObjectAttrPacket* pattr)
{
	DispatchMap::const_iterator it = dispatch_map_.find(pattr->GetType());
	if (it != dispatch_map_.end())
	{
		return it->second->OnLoadFromDB(*pattr);
	}
	return true;
}

void SubSysIf::RegisterMsgHandler(MSG::MsgType msg_type, const PlayerSubSystem::MsgDispatcherCB& disp_cb)
{
	MsgDispatchMap::const_iterator it = msg_dispatch_map_.find(msg_type);
	if (it != msg_dispatch_map_.end())
	{
		ASSERT(false);
		return;
	}
	msg_dispatch_map_[msg_type] = disp_cb;
}

int SubSysIf::DispatchMsg(const MSG& msg)
{
	MsgDispatchMap::const_iterator it = msg_dispatch_map_.find(msg.message);
	if (it != msg_dispatch_map_.end())
	{
		// 如果处理失败这里只打印错误，不return错误
		int ret = it->second(msg);
		if (ret != 0)
		{
			__PRINTF("MSG:%d player-subsys handler return error! retCode=%d", msg.message, ret);
		}
		return kMsgHasBeenHandled;
	}
	// not found
	return kMsgHandlerNotFound;
}

void SubSysIf::CreateMultiSubSys(const SubSysMask& mask)
{
    for (int i = 0; i < (int)PlayerSubSystem::SUB_SYS_TYPE_MAX; ++i)
    {
        if (mask.test(i))
        {
            //创建子系统
            PlayerSubSystem* pSubSys = CreateSubSys(static_cast<SubSysType>(i));
            ASSERT(pSubSys);
            subsys_vec_.push_back(pSubSys);
        }
    }
}

} // namespace gamed
