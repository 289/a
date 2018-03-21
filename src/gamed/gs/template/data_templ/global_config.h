#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_CONFIG_H_

#include "base_datatempl.h"

#include "gconfig/player_lvlup_exp_config.h"
#include "gconfig/player_cat_vision_config.h"
#include "gconfig/pet_lvlup_exp_config.h"
#include "gconfig/pet_lvlup_blood_config.h"
#include "gconfig/pet_lvlup_power_config.h"
#include "gconfig/pet_combat_inv_config.h"
#include "gconfig/team_award_config.h"
#include "gconfig/obj_black_list_config.h"
#include "gconfig/primary_prop_reinforce_cfg.h"
#include "gconfig/enhance_config.h"
#include "gconfig/star_config.h"
#include "gconfig/player_punch_card_cfg.h"
#include "gconfig/mount_config.h"
#include "gconfig/participation_config.h"
#include "gconfig/boss_challenge_config.h"
#include "gconfig/card_config.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 *    （1）GlobalConfigTempl把所有的全局配置表都综合在一起，作为成员变量
 *    （2）全局配置表是指全局只有一份数据的配置
 */
class GlobalConfigTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(GlobalConfigTempl, TEMPL_TYPE_GLOBAL_CONFIG);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

public:
	PlayerLvlUpExpConfig  player_lvlup_exp_config;
	PlayerCatVisionConfig player_cat_vision_config;
	PetLvlUpExpConfig     pet_lvlup_exp_config;
	PetLvlUpBloodConfig   pet_lvlup_blood_config;
	PetLvlUpPowerConfig   pet_lvlup_power_config;
	PetCombatInvConfig    pet_combat_inv_config;
	TeamAwardConfig       team_award_config;
	ObjBlackListConfig    obj_black_list;
    PrimaryPropReinforceConfig primary_prop_rf;
    EnhanceConfig         enhance_config;
    StarConfig            star_config;
    PlayerPunchCardConfig punch_card_cfg;
    MountConfig           mount_config;
    ParticipationConfig   participation_config;
    BossChallengeConfig   boss_challenge_config;
    CardConfig            card_config;

protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(player_lvlup_exp_config);
		MARSHAL_TEMPLVALUE(player_cat_vision_config);
		MARSHAL_TEMPLVALUE(pet_lvlup_exp_config);
		MARSHAL_TEMPLVALUE(pet_lvlup_blood_config);
		MARSHAL_TEMPLVALUE(pet_lvlup_power_config);
		MARSHAL_TEMPLVALUE(pet_combat_inv_config);
		MARSHAL_TEMPLVALUE(team_award_config);
		MARSHAL_TEMPLVALUE(obj_black_list);
        MARSHAL_TEMPLVALUE(primary_prop_rf);
        MARSHAL_TEMPLVALUE(enhance_config);
        MARSHAL_TEMPLVALUE(star_config);
        MARSHAL_TEMPLVALUE(punch_card_cfg);
        MARSHAL_TEMPLVALUE(mount_config);
        MARSHAL_TEMPLVALUE(participation_config);
        MARSHAL_TEMPLVALUE(boss_challenge_config);
        MARSHAL_TEMPLVALUE(card_config);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(player_lvlup_exp_config);
		UNMARSHAL_TEMPLVALUE(player_cat_vision_config);
		UNMARSHAL_TEMPLVALUE(pet_lvlup_exp_config);
		UNMARSHAL_TEMPLVALUE(pet_lvlup_blood_config);
		UNMARSHAL_TEMPLVALUE(pet_lvlup_power_config);
		UNMARSHAL_TEMPLVALUE(pet_combat_inv_config);
		UNMARSHAL_TEMPLVALUE(team_award_config);
		UNMARSHAL_TEMPLVALUE(obj_black_list);
        UNMARSHAL_TEMPLVALUE(primary_prop_rf);
        UNMARSHAL_TEMPLVALUE(enhance_config);
        UNMARSHAL_TEMPLVALUE(star_config);
        UNMARSHAL_TEMPLVALUE(punch_card_cfg);
        UNMARSHAL_TEMPLVALUE(mount_config);
        UNMARSHAL_TEMPLVALUE(participation_config);
        UNMARSHAL_TEMPLVALUE(boss_challenge_config);
        UNMARSHAL_TEMPLVALUE(card_config);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (!player_lvlup_exp_config.CheckDataValidity())
			return false;
		if (!player_cat_vision_config.CheckDataValidity())
			return false;
		if (!pet_lvlup_exp_config.CheckDataValidity())
			return false;
		if (!pet_lvlup_blood_config.CheckDataValidity())
			return false;
		if (!pet_lvlup_power_config.CheckDataValidity())
			return false;
		if (!pet_combat_inv_config.CheckDataValidity())
			return false;
		if (!team_award_config.CheckDataValidity())
			return false;
		if (!obj_black_list.CheckDataValidity())
			return false;
        if (!primary_prop_rf.CheckDataValidity())
            return false;
        if (!enhance_config.CheckDataValidity())
            return false;
        if (!star_config.CheckDataValidity())
            return false;
        if (!punch_card_cfg.CheckDataValidity())
            return false;
        if (!mount_config.CheckDataValidity())
            return false;
        if (!participation_config.CheckDataValidity())
            return false;
        if (!boss_challenge_config.CheckDataValidity())
            return false;
        if (!card_config.CheckDataValidity())
            return false;
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GLOBAL_CONFIG_H_
