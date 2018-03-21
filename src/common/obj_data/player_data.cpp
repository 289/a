#include "common/obj_data/player_data.h"

///
/// register attribute to codec 
/// example : codec_.RegisterAttr<PlayerBaseInfo>(static_cast<ObjectAttrPacket*>(&this->base_info));
///
#define REGISTER_ATTRIBUTE(attr_name, attr_member_var) \
	codec_.RegisterAttr<attr_name>(static_cast<ObjectAttrPacket*>(attr_member_var));


namespace common {

using namespace shared;
using namespace shared::net;

PlayerData::PlayerData()
	: is_inited_(false)
{
	//1
	REGISTER_ATTRIBUTE(PlayerRoleInfo, &role_info);
	REGISTER_ATTRIBUTE(PlayerBaseInfo, &base_info);
	REGISTER_ATTRIBUTE(LocationInfo, &location);
	REGISTER_ATTRIBUTE(PlayerInventoryData, &inventory);

	//5
	REGISTER_ATTRIBUTE(PlayerPreloadData, &preload_data);
	REGISTER_ATTRIBUTE(PlayerCoolDownData, &cooldown_data);
	REGISTER_ATTRIBUTE(PlayerCombatData, &combat_data);
	REGISTER_ATTRIBUTE(PlayerSkillData, &skill_data);
	REGISTER_ATTRIBUTE(PlayerPetData, &pet_data);

	//10
    REGISTER_ATTRIBUTE(PlayerBuffData, &buff_data);
	REGISTER_ATTRIBUTE(PlayerTaskData, &task_data);
	REGISTER_ATTRIBUTE(PlayerObjectBWList, &obj_bw_list);
	REGISTER_ATTRIBUTE(PlayerInstanceData, &ins_data);
	REGISTER_ATTRIBUTE(PlayerMallData, &mall_data);

	//15
	REGISTER_ATTRIBUTE(PlayerMailList, &mail_data);
	REGISTER_ATTRIBUTE(PlayerSpecMoney, &spec_money);
	REGISTER_ATTRIBUTE(PlayerTalentData, &talent_data);
    REGISTER_ATTRIBUTE(PlayerAuctionData, &auction_data);
    REGISTER_ATTRIBUTE(PlayerFriendData, &friend_data);

    // 20
    REGISTER_ATTRIBUTE(PlayerReputationData, &reputation_data);
    REGISTER_ATTRIBUTE(PlayerAchieveData, &achieve_data);
    REGISTER_ATTRIBUTE(PlayerEnhanceData, &enhance_data);
    REGISTER_ATTRIBUTE(PlayerPropRFData, &prop_rf_data);
    REGISTER_ATTRIBUTE(PlayerTitleData, &title_data);

    // 25
    REGISTER_ATTRIBUTE(PlayerStarData, &star_data);
    REGISTER_ATTRIBUTE(BattleGroundData, &bg_data);
    REGISTER_ATTRIBUTE(PlayerCounterData, &counter_data);
    REGISTER_ATTRIBUTE(PlayerGeventData, &gevent_data);
    REGISTER_ATTRIBUTE(PlayerPunchCardData, &punch_card);

    // 30
    REGISTER_ATTRIBUTE(PlayerParticipationData, &participation);
    REGISTER_ATTRIBUTE(PlayerMountData, &mount_data);
    REGISTER_ATTRIBUTE(PlayerBossChallengeData, &challenge_data);
    //REGISTER_ATTRIBUTE(PlayerArenaData, &arena_data);

	Release();
}

PlayerData::~PlayerData()
{
	Release();
}

void PlayerData::Release()
{
	is_inited_ = false;
	// release all attribute, that registered
	codec_.Release();
}

// ATTENTION: The caller needs to lock
int PlayerData::Serialize(Buffer* pbuf)
{
	try
	{
		if (0 != codec_.FillEmptyBuffer(pbuf))
		{
			LOG_ERROR << "PlayerData Serialize error";
			return -1;
		}
	}
	catch (const Exception& ex)
	{
		LOG_ERROR << "PlayerData Serialize throw exception, reason:" << ex.What();
		return -1;
	}

	return 0;
}

// ATTENTION: The caller needs to lock
int PlayerData::Deserialize(const char* pbuf, int32_t len)
{
	try
	{
		if (0 != codec_.ParseObjectData(pbuf, len))
		{
			LOG_ERROR << "PlayerData Deserialize error";
			return -1;
		}
	}
	catch (const Exception& ex)
	{
		LOG_ERROR << "PlayerData Deserialize throw exception, reason:" << ex.What();
		return -1;
	}
		
	is_inited_ = true;
	return 0;
}

bool PlayerData::ForeachPlayerAttr(const PlayerAttrDataCB& callback) const
{
	return codec_.ForeachAttribute(callback);
}

} // namespace common
