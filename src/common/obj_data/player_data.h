#ifndef COMMON_OBJDATAPOOL_PLAYER_DATA_H_
#define COMMON_OBJDATAPOOL_PLAYER_DATA_H_

#include "common/obj_data/player_attr_declar.h"
#include "common/obj_data/obj_data.h"
#include "common/obj_data/codec_obj_data.h"


namespace common {

/**
 * @brief PlayerData 
 *    1.玩家数据类，player在gs已经实现存盘数据的标准格式化（PlayerData），
 *      这样做的好处是player数据可以轻松的在各个服务器进程中传输，而且定义只有一份
 */
class PlayerData : public ObjectData
{
public:
	typedef ObjectDataCodec::CodecAttributeCB PlayerAttrDataCB;

	PlayerData();
	virtual ~PlayerData();

	virtual void Release();

	///
	/// ATTENTION: The caller needs to lock this player, when calling these two function
	///
	int    Serialize(shared::net::Buffer* pbuf);
	int    Deserialize(const char* pbuf, int32_t len);
	bool   ForeachPlayerAttr(const PlayerAttrDataCB& callback) const;


	/**
	 * @brief: attribute
	 *    以下属性需要在cpp里进行REGISTER_ATTRIBUTE
	 */
public:
	PlayerRoleInfo          role_info;
	PlayerBaseInfo          base_info;
	LocationInfo            location;
	PlayerInventoryData     inventory;
	PlayerPreloadData       preload_data;
	PlayerCoolDownData      cooldown_data;
	PlayerCombatData        combat_data;
	PlayerSkillData         skill_data;
	PlayerPetData           pet_data;
    PlayerBuffData          buff_data;
	PlayerTaskData		    task_data;
	PlayerObjectBWList      obj_bw_list;
	PlayerInstanceData      ins_data;
	PlayerMallData          mall_data;
	PlayerMailList		    mail_data;
	PlayerSpecMoney		    spec_money;
	PlayerTalentData        talent_data;
    PlayerAuctionData       auction_data;
    PlayerFriendData        friend_data;
    PlayerReputationData    reputation_data;
    PlayerAchieveData       achieve_data;
    PlayerEnhanceData       enhance_data;
    PlayerPropRFData        prop_rf_data;
    PlayerTitleData         title_data;
    PlayerStarData          star_data;
    BattleGroundData        bg_data;
    PlayerCounterData       counter_data;
    PlayerGeventData        gevent_data;
    PlayerPunchCardData     punch_card;
    PlayerParticipationData participation;
    PlayerMountData         mount_data;
    PlayerBossChallengeData challenge_data;
    PlayerArenaData         arena_data;


    // member
private:
	ObjectDataCodec     codec_;
	bool                is_inited_;
};

} // namespace common

#endif // COMMON_OBJDATAPOOL_PLAYER_DATA_H_
