#include "G2C_proto.h"


// 类名和协议号的对应关系与G2C_proto.h 否则启动时会报错
// TODO: 下面这段代码应该由lua程序读取G2C_proto.h自动生成
namespace G2C {

	// 0
	INIT_STATIC_PROTOPACKET(ErrorMessage, ERROR_MESSAGE);
	INIT_STATIC_PROTOPACKET(PlayerVolatileInfo, PLAYER_VOLATILE_INFO);
	INIT_STATIC_PROTOPACKET(PlayerVisibleInfoNotify, PLAYER_VISIBLE_INFO_NOTIFY);
	INIT_STATIC_PROTOPACKET(EnterWorld_Re, ENTER_WORLD_RE);
	INIT_STATIC_PROTOPACKET(PlayerStartMove, PLAYER_START_MOVE);

	// 5
	INIT_STATIC_PROTOPACKET(PlayerMove, PLAYER_MOVE);
	INIT_STATIC_PROTOPACKET(PlayerStopMove, PLAYER_STOP_MOVE);
	INIT_STATIC_PROTOPACKET(PlayerVisibleInfoChange, PLAYER_VISIBLE_INFO_CHANGE);
	INIT_STATIC_PROTOPACKET(PlayerEnterView, PLAYER_ENTER_VIEW);
	INIT_STATIC_PROTOPACKET(PlayerLeaveView, PLAYER_LEAVE_VIEW);

	// 10
	INIT_STATIC_PROTOPACKET(ObjectVisibleInfoNotify, OBJECT_VISIBLE_INFO_NOTIFY);
	INIT_STATIC_PROTOPACKET(ObjectVisibleInfoList, OBJECT_VISIBLE_INFO_LIST);
	INIT_STATIC_PROTOPACKET(PullPlayerBackToValidPos, PULL_PLAYER_BACK_TO_VALID_POS);
	INIT_STATIC_PROTOPACKET(EnterEventArea, ENTER_EVENT_AREA);
	INIT_STATIC_PROTOPACKET(NotifyPlayerMoveProp, NOTIFY_PLAYER_MOVE_PROP);

	// 15
	INIT_STATIC_PROTOPACKET(SelfItemDetail, SELF_ITEM_DETAIL);
	INIT_STATIC_PROTOPACKET(SelfItemInfoList, SELF_ITEM_INFO_LIST);
	INIT_STATIC_PROTOPACKET(SelfItemDetailList, SELF_ITEM_DETAIL_LIST);
	INIT_STATIC_PROTOPACKET(DropInvItem, DROP_INV_ITEM);
	INIT_STATIC_PROTOPACKET(MoveInvItem, MOVE_INV_ITEM);

	// 20
	INIT_STATIC_PROTOPACKET(SellInvItem, SELL_INV_ITEM);
	INIT_STATIC_PROTOPACKET(ExchangeInvItem, EXCHANGE_INV_ITEM);
	INIT_STATIC_PROTOPACKET(EquipItem, EQUIP_ITEM);
	INIT_STATIC_PROTOPACKET(UndoEquip, UNDO_EQUIP);
	INIT_STATIC_PROTOPACKET(UseItem, USE_ITEM);

	// 25
	INIT_STATIC_PROTOPACKET(RefineEquip_Re, REFINE_EQUIP_RE);
	INIT_STATIC_PROTOPACKET(ObjectEnterView, OBJECT_ENTER_VIEW);
	INIT_STATIC_PROTOPACKET(ObjectLeaveView, OBJECT_LEAVE_VIEW);
	INIT_STATIC_PROTOPACKET(CoolDownData, COOL_DOWN_DATA);
	INIT_STATIC_PROTOPACKET(SetCoolDown, SET_COOL_DOWN);

	// 30
	INIT_STATIC_PROTOPACKET(BroadcastPlayerPullBack, BROADCAST_PLAYER_PULLBACK);
	INIT_STATIC_PROTOPACKET(CombatPlayerJoinFail, COMBAT_PLAYER_JOIN_FAIL);
	INIT_STATIC_PROTOPACKET(CombatSelectSkill_Re, COMBAT_SELECT_SKILL_RE);
	INIT_STATIC_PROTOPACKET(CombatPVEStart, COMBAT_PVE_START);
	INIT_STATIC_PROTOPACKET(CombatPVPStart, COMBAT_PVP_START);

	// 35
	INIT_STATIC_PROTOPACKET(CombatPVEEnd, COMBAT_PVE_END);
	INIT_STATIC_PROTOPACKET(CombatPlayerBaseProp, COMBAT_PLAYER_BASE_PROP);
	INIT_STATIC_PROTOPACKET(CombatSkillResult, COMBAT_SKILL_RESULT);
	INIT_STATIC_PROTOPACKET(CombatBuffResult, COMBAT_BUFF_RESULT);
	INIT_STATIC_PROTOPACKET(CombatAward, COMBAT_AWARD);

	// 40
	INIT_STATIC_PROTOPACKET(CombatMobDead, COMBAT_MOB_DEAD);
	INIT_STATIC_PROTOPACKET(CombatPlayerState, COMBAT_PLAYER_STATE);
	INIT_STATIC_PROTOPACKET(CombatPlayerExtProp, COMBAT_PLAYER_EXTPROP);
	INIT_STATIC_PROTOPACKET(CombatUnitVolatileProp, COMBAT_UNIT_VOLATILE_PROP);
	INIT_STATIC_PROTOPACKET(CombatPVEContinue, COMBAT_PVE_CONTINUE);

	// 45
	INIT_STATIC_PROTOPACKET(CombatPVPContinue, COMBAT_PVP_CONTINUE);
	INIT_STATIC_PROTOPACKET(PlayerBaseInfo, PLAYER_BASE_INFO);
	INIT_STATIC_PROTOPACKET(PlayerExtendProp, PLAYER_EXTEND_PROP);
	INIT_STATIC_PROTOPACKET(PlayerGainExp, PLAYER_GAIN_EXP);
	INIT_STATIC_PROTOPACKET(PlayerLevelUp, PLAYER_LEVEL_UP);

	// 50
	INIT_STATIC_PROTOPACKET(ElsePlayerEnterCombat, ELSE_PLAYER_ENTER_COMBAT);
	INIT_STATIC_PROTOPACKET(ElsePlayerLeaveCombat, ELSE_PLAYER_LEAVE_COMBAT);
	INIT_STATIC_PROTOPACKET(ObjectMove, OBJECT_MOVE);
	INIT_STATIC_PROTOPACKET(ObjectStopMove, OBJECT_STOP_MOVE);
	INIT_STATIC_PROTOPACKET(SkillData, SKILL_DATA);

	// 55
	INIT_STATIC_PROTOPACKET(CombatBuffData, COMBAT_BUFF_DATA);
	INIT_STATIC_PROTOPACKET(NpcGreeting, NPC_GREETING);
	INIT_STATIC_PROTOPACKET(QueryTeamMemberRe, QUERY_TEAM_MEMBER_RE);
	INIT_STATIC_PROTOPACKET(GainPet, GAIN_PET);
	INIT_STATIC_PROTOPACKET(LostPet, LOST_PET);

	// 60
	INIT_STATIC_PROTOPACKET(PetGainExp, PET_GAIN_EXP);
	INIT_STATIC_PROTOPACKET(PetLevelUp, PET_LEVEL_UP);
	INIT_STATIC_PROTOPACKET(SetCombatPet_Re, SET_COMBAT_PET_RE);
	INIT_STATIC_PROTOPACKET(LevelUpBloodline_Re, LEVELUP_BLOODLINE_RE);
	INIT_STATIC_PROTOPACKET(LevelUpPetPowerCap_Re, LEVELUP_PET_POWER_CAP_RE);

	// 65
	INIT_STATIC_PROTOPACKET(CombatRoundEnd, COMBAT_ROUND_END);
	INIT_STATIC_PROTOPACKET(CombatPlayerJoin, COMBAT_PLAYER_JOIN);
	INIT_STATIC_PROTOPACKET(GetOwnMoney, GET_OWN_MONEY);
	INIT_STATIC_PROTOPACKET(GainMoney, GAIN_MONEY);
	INIT_STATIC_PROTOPACKET(SpendMoney, SPEND_MONEY);

	// 70
	INIT_STATIC_PROTOPACKET(NotifySelfPos, NOTIFY_SELF_POS);
	INIT_STATIC_PROTOPACKET(CombatPlayerGainGolem, COMBAT_PLAYER_GAIN_GOLEM);
	INIT_STATIC_PROTOPACKET(CombatPlayerSwitchGolem, COMBAT_PLAYER_SWITCH_GOLEM);
	INIT_STATIC_PROTOPACKET(UpdateSkillTree, UPDATE_SKILL_TREE);
	INIT_STATIC_PROTOPACKET(TaskNotifyClient, TASK_NOTIFY_CLIENT);

	// 75
	INIT_STATIC_PROTOPACKET(EnterRuleArea, ENTER_RULE_AREA);
	INIT_STATIC_PROTOPACKET(LeaveRuleArea, LEAVE_RULE_AREA);
	INIT_STATIC_PROTOPACKET(TransferPrepare, TRANSFER_PREPARE);
	INIT_STATIC_PROTOPACKET(PlayerDead, PLAYER_DEAD);
	INIT_STATIC_PROTOPACKET(PlayerResurrect, PLAYER_RESURRECT);

	// 80
	INIT_STATIC_PROTOPACKET(JoinTeamReq, JOIN_TEAM_REQ);
	INIT_STATIC_PROTOPACKET(BuddyJoinTeam, BUDDY_JOIN_TEAM);
	INIT_STATIC_PROTOPACKET(BuddyLeaveTeam, BUDDY_LEAVE_TEAM);
	INIT_STATIC_PROTOPACKET(BuddyTeamInfo, BUDDY_TEAM_INFO);
	INIT_STATIC_PROTOPACKET(ChangeBuddyTeamPos_Re, CHANGE_BUDDY_TEAM_POS_RE);

	// 85
	INIT_STATIC_PROTOPACKET(GatherStart, GATHER_START);
	INIT_STATIC_PROTOPACKET(SelfGatherStart, SELF_GATHER_START);
	INIT_STATIC_PROTOPACKET(GatherStop, GATHER_STOP);
	INIT_STATIC_PROTOPACKET(TeammateEnterCombatFail, TEAMMATE_ENTER_COMBAT_FAIL);
	INIT_STATIC_PROTOPACKET(UpdateVisibleBuff, UPDATE_VISIBLE_BUFF);

	// 90
	INIT_STATIC_PROTOPACKET(PlayerGainItem, PLAYER_GAIN_ITEM);
	INIT_STATIC_PROTOPACKET(CatVisionGainExp, CAT_VISION_GAIN_EXP);
	INIT_STATIC_PROTOPACKET(CatVisionLevelUp, CAT_VISION_LEVEL_UP);
	INIT_STATIC_PROTOPACKET(OpenCatVision_Re, OPEN_CAT_VISION_RE);
	INIT_STATIC_PROTOPACKET(CloseCatVision_Re, CLOSE_CAT_VISION_RE);

	//95
	INIT_STATIC_PROTOPACKET(CloseCatVision, CLOSE_CAT_VISION);
	INIT_STATIC_PROTOPACKET(LearnSkill_Re, LEARN_SKILL_RE);
	INIT_STATIC_PROTOPACKET(SwitchSkill_Re, SWITCH_SKILL_RE);
	INIT_STATIC_PROTOPACKET(ElsePlayerExtProp, ELSE_PLAYER_EXTPROP);
	INIT_STATIC_PROTOPACKET(CombatUnitSpeak, COMBAT_UNIT_SPEAK);

	//100
	INIT_STATIC_PROTOPACKET(TaskData, TASK_DATA);
	INIT_STATIC_PROTOPACKET(CombatSummonMob, COMBAT_SUMMON_MOB);
	INIT_STATIC_PROTOPACKET(CombatMultiMobSpeak, COMBAT_MULTI_MOB_SPEAK);
    INIT_STATIC_PROTOPACKET(TransferCls, TRANSFER_CLS);
	INIT_STATIC_PROTOPACKET(DramaGatherStart, DRAMA_GATHER_START);

	//105
	INIT_STATIC_PROTOPACKET(DramaGatherStop, DRAMA_GATHER_STOP);
	INIT_STATIC_PROTOPACKET(ObjectBWList, OBJECT_BW_LIST);
	INIT_STATIC_PROTOPACKET(ObjectBWListChange, OBJECT_BW_LIST_CHANGE);
	INIT_STATIC_PROTOPACKET(ElsePlayerEquipCRC, ELSE_PLAYER_EQUIPCRC);
	INIT_STATIC_PROTOPACKET(ElsePlayerEquipment, ELSE_PLAYER_EQUIPMENT);

	// 110
	INIT_STATIC_PROTOPACKET(ChangeName_Re, CHANGE_NAME_RE);
	INIT_STATIC_PROTOPACKET(BroadcastChangeName, BROADCAST_CHANGE_NAME);
	INIT_STATIC_PROTOPACKET(GetStaticRoleInfo_Re, GET_STATIC_ROLE_INFO_RE);
	INIT_STATIC_PROTOPACKET(SelfStaticRoleInfo, SELF_STATIC_ROLE_INFO);
	INIT_STATIC_PROTOPACKET(MapKickoutCountdown, MAP_KICKOUT_COUNTDOWN);

	// 115
	INIT_STATIC_PROTOPACKET(GetInstanceData_Re, GET_INSTANCE_DATA_RE);
	INIT_STATIC_PROTOPACKET(ChatMsg, CHAT_MSG);
	INIT_STATIC_PROTOPACKET(MoveCombatPet_Re, MOVE_COMBAT_PET_RE);
	INIT_STATIC_PROTOPACKET(CombatUnitTurnFront, COMBAT_UNIT_TURN_FRONT);
	INIT_STATIC_PROTOPACKET(QueryNpcZoneInfo_Re, QUERY_NPC_ZONE_INFO_RE);

	// 120
	INIT_STATIC_PROTOPACKET(MallData, MALL_DATA);
	INIT_STATIC_PROTOPACKET(MallGoodsDetail, MALL_GOODS_DETAIL);
	INIT_STATIC_PROTOPACKET(MallShopping_Re, MALL_SHOPPING_RE);
	INIT_STATIC_PROTOPACKET(GetOwnCash, GET_OWN_CASH);
	INIT_STATIC_PROTOPACKET(NpcServiceContent, NPC_SERVICE_CONTENT);

	// 125
	INIT_STATIC_PROTOPACKET(ServerTime, SERVER_TIME);
	INIT_STATIC_PROTOPACKET(ShopShopping_Re, SHOP_SHOPPING_RE);
	INIT_STATIC_PROTOPACKET(QueryItemInfo_Re, QUERY_ITEM_INFO_RE);
	INIT_STATIC_PROTOPACKET(AnnounceNewMail, ANNOUNCE_NEW_MAIL);
	INIT_STATIC_PROTOPACKET(GetMailList_Re, GET_MAIL_LIST_RE);

	// 130
	INIT_STATIC_PROTOPACKET(GetMailAttach_Re, GET_MAIL_ATTACH_RE);
	INIT_STATIC_PROTOPACKET(DeleteMail_Re, DELETE_MAIL_RE);
	INIT_STATIC_PROTOPACKET(TakeoffMailAttach_Re, TAKEOFF_MAIL_ATTACH_RE);
	INIT_STATIC_PROTOPACKET(SendMail_Re, SEND_MAIL_RE);
	INIT_STATIC_PROTOPACKET(TeammemberLocalInsReply, TEAMMEMBER_LOCAL_INS_REPLY);
	
	// 135
	INIT_STATIC_PROTOPACKET(TeammemberQuitLocalIns, TEAMMEMBER_QUIT_LOCAL_INS);
	INIT_STATIC_PROTOPACKET(TeammemberAgreeLocalIns, TEAMMEMBER_AGREE_LOCAL_INS);
	INIT_STATIC_PROTOPACKET(GainScore, GAIN_SCORE);
	INIT_STATIC_PROTOPACKET(SpendScore, SPEND_SCORE);
	INIT_STATIC_PROTOPACKET(TeamLocalInsInvite, TEAM_LOCAL_INS_INVITE);

	// 140
	INIT_STATIC_PROTOPACKET(MapTeamInfo, MAP_TEAM_INFO);
	INIT_STATIC_PROTOPACKET(MapTeamQueryMemberRe, MAP_TEAM_QUERY_MEMBER_RE);
	INIT_STATIC_PROTOPACKET(MapTeamJoin, MAP_TEAM_JOIN);
	INIT_STATIC_PROTOPACKET(MapTeamLeave, MAP_TEAM_LEAVE);
	INIT_STATIC_PROTOPACKET(MapTeamStatusChange, MAP_TEAM_STATUS_CHANGE);

	// 145
	INIT_STATIC_PROTOPACKET(MapTeamChangeLeader_Re, MAP_TEAM_CHANGE_LEADER_RE);
	INIT_STATIC_PROTOPACKET(MapTeamChangePos_Re, MAP_TEAM_CHANGE_POS_RE);
    INIT_STATIC_PROTOPACKET(TransferGender, TRANSFER_GENDER);
	INIT_STATIC_PROTOPACKET(CombatLearnSkill, COMBAT_LEARN_SKILL);
	INIT_STATIC_PROTOPACKET(PlayerEnterInsMap, PLAYER_ENTER_INS_MAP);

	// 150
	INIT_STATIC_PROTOPACKET(InstanceEnd, INSTANCE_END);
	INIT_STATIC_PROTOPACKET(PlayerScore, PLAYER_SCORE);
	INIT_STATIC_PROTOPACKET(SelfItemDetailListAll, SELF_ITEM_DETAIL_LIST_ALL);
	INIT_STATIC_PROTOPACKET(PetPower, PET_POWER);
	INIT_STATIC_PROTOPACKET(PetData, PET_DATA);

	// 155
	INIT_STATIC_PROTOPACKET(CombatPetAttack_Re, COMBAT_PET_ATTACK_RE);
	INIT_STATIC_PROTOPACKET(TalentData, TALENT_DATA);
	INIT_STATIC_PROTOPACKET(TalentDetail, TALENT_DETAIL);
	INIT_STATIC_PROTOPACKET(LevelUpTalent_Re, LEVELUP_TALENT_RE);
	INIT_STATIC_PROTOPACKET(OpenTalentGroup, OPEN_TALENT_GROUP);
	
	// 160
	INIT_STATIC_PROTOPACKET(CombatPetPower, COMBAT_PET_POWER);
	INIT_STATIC_PROTOPACKET(FullPetPower_Re, FULL_PET_POWER_RE);
	INIT_STATIC_PROTOPACKET(LevelUpCard_Re, LEVELUP_CARD_RE);
    INIT_STATIC_PROTOPACKET(AuctionItem_Re, AUCTION_ITEM_RE);
    INIT_STATIC_PROTOPACKET(AuctionCancel_Re, AUCTION_CANCEL_RE);

    // 165
    INIT_STATIC_PROTOPACKET(AuctionBuyout_Re, AUCTION_BUYOUT_RE);
    INIT_STATIC_PROTOPACKET(AuctionBid_Re, AUCTION_BID_RE);
    INIT_STATIC_PROTOPACKET(AuctionRevoke, AUCTION_REVOKE);
    INIT_STATIC_PROTOPACKET(PlayerLeaveInsMap, PLAYER_LEAVE_INS_MAP);
    INIT_STATIC_PROTOPACKET(CombatPlayerSelectSkill, COMBAT_PLAYER_SELECT_SKILL);

    // 170
    INIT_STATIC_PROTOPACKET(FriendData, FRIEND_DATA);
    INIT_STATIC_PROTOPACKET(AddFriendRe, ADD_FRIEND_RE);
    INIT_STATIC_PROTOPACKET(DeleteFriendRe, DELETE_FRIEND_RE);
    INIT_STATIC_PROTOPACKET(TeamLeaderConvene, TEAM_LEADER_CONVENE);
    INIT_STATIC_PROTOPACKET(CombatMobTransform, COMBAT_MOB_TRANSFORM);

    // 175
    INIT_STATIC_PROTOPACKET(TeamCrossInsInvite, TEAM_CROSS_INS_INVITE);
    INIT_STATIC_PROTOPACKET(TeamCrossInsInviteReply, TEAM_CROSS_INS_INVITE_REPLY);
    INIT_STATIC_PROTOPACKET(TeammemberAgreeCrossIns, TEAMMEMBER_AGREE_CROSS_INS);
    INIT_STATIC_PROTOPACKET(TeamCrossInsRequestComplete, TEAM_CROSS_INS_REQUEST_COMPLETE);
	INIT_STATIC_PROTOPACKET(CombatPVPEnd, COMBAT_PVP_END);

    // 180
	INIT_STATIC_PROTOPACKET(TitleData, TITLE_DATA);
	INIT_STATIC_PROTOPACKET(GainTitle, GAIN_TITLE);
	INIT_STATIC_PROTOPACKET(SwitchTitle_Re, SWITCH_TITLE_RE);
	INIT_STATIC_PROTOPACKET(BroadcastPlayerTitle, BROADCAST_PLAYER_TITLE);
    INIT_STATIC_PROTOPACKET(DuelRequest, DUEL_REQUEST);

    // 185
    INIT_STATIC_PROTOPACKET(DuelRequest_Re, DUEL_REQUEST_RE);
    INIT_STATIC_PROTOPACKET(TeammateDuelRequest, TEAMMATE_DUEL_REQUEST);
    INIT_STATIC_PROTOPACKET(TeammateDuelRequest_Re, TEAMMATE_DUEL_REQUEST_RE);
    INIT_STATIC_PROTOPACKET(DuelPrepare, DUEL_PREPARE);
    INIT_STATIC_PROTOPACKET(ReputationData, REPUTATION_DATA);

    // 190
    INIT_STATIC_PROTOPACKET(OpenReputation, OPEN_REPUTATION);
    INIT_STATIC_PROTOPACKET(ModifyReputation, MODIFY_REPUTATION);
    INIT_STATIC_PROTOPACKET(UIData, UI_DATA);
    INIT_STATIC_PROTOPACKET(PlayerPrimaryPropRF, PLAYER_PRIMARY_PROP_RF);
    INIT_STATIC_PROTOPACKET(QueryPrimaryPropRF_Re, QUERY_PRIMARY_PROP_RF_RE);

    // 195
    INIT_STATIC_PROTOPACKET(PrimaryPropReinforce_Re, PRIMARY_PROP_REINFORCE_RE);
    INIT_STATIC_PROTOPACKET(BuyPrimaryPropRFEnergy_Re, BUY_PRIMARY_PROP_RF_ENERGY_RE);
    INIT_STATIC_PROTOPACKET(AchieveData, ACHIEVE_DATA);
    INIT_STATIC_PROTOPACKET(AchieveNotifyClient, ACHIEVE_NOTIFY_CLIENT);
    INIT_STATIC_PROTOPACKET(LoginData, LOGIN_DATA);

    // 200
    INIT_STATIC_PROTOPACKET(EnhanceData, ENHANCE_DATA);
    INIT_STATIC_PROTOPACKET(OpenEnhanceSlotRe, OPEN_ENHANCE_SLOT_RE);
    INIT_STATIC_PROTOPACKET(ProtectEnhanceSlotRe, PROTECT_ENHANCE_SLOT_RE);
    INIT_STATIC_PROTOPACKET(UnProtectEnhanceSlotRe, UNPROTECT_ENHANCE_SLOT_RE);
    INIT_STATIC_PROTOPACKET(EnhanceRe, ENHANCE_RE);

    // 205
    INIT_STATIC_PROTOPACKET(StarData, STAR_DATA);
    INIT_STATIC_PROTOPACKET(OpenStarRe, OPEN_STAR_RE);
    INIT_STATIC_PROTOPACKET(ActivateSparkRe, ACTIVATE_SPARK_RE);
    INIT_STATIC_PROTOPACKET(UseItemError, USE_ITEM_ERROR);
    INIT_STATIC_PROTOPACKET(PlayerEnterBGMap, PLAYER_ENTER_BG_MAP);

    // 210
    INIT_STATIC_PROTOPACKET(PlayerLeaveBGMap, PLAYER_LEAVE_BG_MAP);
    INIT_STATIC_PROTOPACKET(GetBattleGroundData_Re, GET_BATTLEGROUND_DATA_RE);
    INIT_STATIC_PROTOPACKET(EquipCard_Re, EQUIP_CARD_RE);
    INIT_STATIC_PROTOPACKET(GainCard, GAIN_CARD);
    INIT_STATIC_PROTOPACKET(LostCard, LOST_CARD);

    // 215
    INIT_STATIC_PROTOPACKET(MapTeamJoinTeamRequest, MAP_TEAM_JOIN_TEAM_REQUEST);
    INIT_STATIC_PROTOPACKET(InsPromptMessage, INS_PROMPT_MESSAGE);
    INIT_STATIC_PROTOPACKET(BGPromptMessage, BG_PROMPT_MESSAGE);
    INIT_STATIC_PROTOPACKET(GlobalCounterChange, GLOBAL_COUNTER_CHANGE);
    INIT_STATIC_PROTOPACKET(MapCounterChange, MAP_COUNTER_CHANGE);

    // 220
    INIT_STATIC_PROTOPACKET(CombatBuffSync, COMBAT_BUFF_SYNC);
    INIT_STATIC_PROTOPACKET(UpdateTaskStorage_Re, UPDATE_TASK_STORAGE_RE);
    INIT_STATIC_PROTOPACKET(CombatMobEscape, COMBAT_MOB_ESCAPE);
    INIT_STATIC_PROTOPACKET(PlayerCounterChange, PLAYER_COUNTER_CHANGE);
    INIT_STATIC_PROTOPACKET(PlayerCounterList, PLAYER_COUNTER_LIST);

    // 225
    INIT_STATIC_PROTOPACKET(NotifyObjectPos, NOTIFY_OBJECT_POS);
    INIT_STATIC_PROTOPACKET(NPCFriendStatusChange, NPC_FRIEND_STATUS_CHANGE);
    INIT_STATIC_PROTOPACKET(ShowMapCountDown, SHOW_MAP_COUNTDOWN);
    INIT_STATIC_PROTOPACKET(CombatSceneShake, COMBAT_SCENE_SHAKE);
    INIT_STATIC_PROTOPACKET(CombatUnitCurGolemProp, COMBAT_UNIT_CUR_GOLEM_PROP);
    
    // 230
    INIT_STATIC_PROTOPACKET(CombatUnitOtherGolemProp, COMBAT_UNIT_OTHER_GOLEM_PROP);
    INIT_STATIC_PROTOPACKET(PunchCardData, PUNCH_CARD_DATA);
    INIT_STATIC_PROTOPACKET(PunchCardResult, PUNCH_CARD_RESULT);
    INIT_STATIC_PROTOPACKET(QueryInstanceRecord_Re, QUERY_INSTANCE_RECORD_RE);
    INIT_STATIC_PROTOPACKET(QueryPunchCardData_Re, QUERY_PUNCH_CARD_DATA_RE);

    // 235
    INIT_STATIC_PROTOPACKET(GainPunchCardAward_Re, GAIN_PUNCH_CARD_AWARD_RE);
    INIT_STATIC_PROTOPACKET(GeventData, GEVENT_DATA);
    INIT_STATIC_PROTOPACKET(JoinGevent_Re, JOIN_GEVENT_RE);
    INIT_STATIC_PROTOPACKET(ParticipationData, PARTICIPATION_DATA);
    INIT_STATIC_PROTOPACKET(ParticipationChange, PARTICIPATION_CHANGE);

    // 240
    INIT_STATIC_PROTOPACKET(ObjectHateYou, OBJECT_HATE_YOU);
    INIT_STATIC_PROTOPACKET(RePunchCardHelp, RE_PUNCH_CARD_HELP);
    INIT_STATIC_PROTOPACKET(MountData, MOUNT_DATA);
    INIT_STATIC_PROTOPACKET(MountMount_Re, MOUNT_MOUNT_RE);
    INIT_STATIC_PROTOPACKET(MountExchange_Re, MOUNT_EXCHANGE_RE);

    // 245
    INIT_STATIC_PROTOPACKET(MountEquipLevelUp_Re, MOUNT_EQUIP_LEVELUP_RE);
    INIT_STATIC_PROTOPACKET(GetParticipationAward_Re, GET_PARTICIPATION_AWARD_RE);
    INIT_STATIC_PROTOPACKET(OpenMountEquip, OPEN_MOUNT_EQUIP);
    INIT_STATIC_PROTOPACKET(GainMount, GAIN_MOUNT);
    INIT_STATIC_PROTOPACKET(LostMount, LOST_MOUNT);

    // 250
    INIT_STATIC_PROTOPACKET(CombatSkillSync, COMBAT_SKILL_SYNC);
    INIT_STATIC_PROTOPACKET(GeventDataChange, GEVENT_DATA_CHANGE);
    INIT_STATIC_PROTOPACKET(ItemDisassemble_Re, ITEM_DISASSEMBLE_RE);
    INIT_STATIC_PROTOPACKET(GameServerVersion, GAME_SERVER_VERSION);
    INIT_STATIC_PROTOPACKET(RePunchCardHelp_Re, RE_PUNCH_CARD_HELP_RE);

    // 255
    INIT_STATIC_PROTOPACKET(BossChallengeData, BOSS_CHALLENGE_DATA);
    INIT_STATIC_PROTOPACKET(GetBossChallengeAward_Re, GET_BOSS_CHALLENGE_AWARD_RE);
    INIT_STATIC_PROTOPACKET(GetClearChallengeAward_Re, GET_CLEAR_CHALLENGE_AWARD_RE);
    INIT_STATIC_PROTOPACKET(MapTeamTidyPos, MAP_TEAM_TIDY_POS);
    INIT_STATIC_PROTOPACKET(BossChallenge_Re, BOSS_CHALLENGE_RE);

    // 260
    INIT_STATIC_PROTOPACKET(QueryWorldBossRecord_Re, QUERY_WORLD_BOSS_RECORD_RE);
    INIT_STATIC_PROTOPACKET(WBCombatEndRecord, WB_COMBAT_END_RECORD);
    INIT_STATIC_PROTOPACKET(CombatWaitSelectSkill, COMBAT_WAIT_SELECT_SKILL);
    INIT_STATIC_PROTOPACKET(CombatWaitSelectPet, COMBAT_WAIT_SELECT_PET);
    INIT_STATIC_PROTOPACKET(GainCash, GAIN_CASH);

    // 265
    INIT_STATIC_PROTOPACKET(SpendCash, SPEND_CASH);
    INIT_STATIC_PROTOPACKET(ArenaODBuyTicketRe, ARENA_OD_BUY_TICKET_RE);

} // namespace G2C
