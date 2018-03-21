#include "C2G_proto.h"

// 类名和协议号的对应关系与G2C_proto.h 否则启动时会报错
// TODO: 下面这段代码应该由lua程序读取G2C_proto.h自动生成
namespace C2G {

	// 0
	INIT_STATIC_PROTOPACKET(PlayerLogout, LOGOUT);
	INIT_STATIC_PROTOPACKET(StartMove, START_MOVE);
	INIT_STATIC_PROTOPACKET(MoveContinue, MOVE_CONTINUE);
	INIT_STATIC_PROTOPACKET(StopMove, STOP_MOVE);
	INIT_STATIC_PROTOPACKET(MoveChange, MOVE_CHANGE);

	// 5
	INIT_STATIC_PROTOPACKET(GetAllData, GET_ALL_DATA);
	INIT_STATIC_PROTOPACKET(GetItemData, GET_ITEM_DATA);
	INIT_STATIC_PROTOPACKET(GetItemListBrief, GET_ITEM_LIST_BRIEF);
	INIT_STATIC_PROTOPACKET(GetItemListDetail, GET_ITEM_LIST_DETAIL);
	INIT_STATIC_PROTOPACKET(DropInventoryItem, DROP_INVENTORY_ITEM);

	// 10
	INIT_STATIC_PROTOPACKET(MoveInventoryItem, MOVE_INVENTORY_ITEM);
	INIT_STATIC_PROTOPACKET(SellInventoryItem, SELL_INVENTORY_ITEM);
	INIT_STATIC_PROTOPACKET(ExchangeInventoryItem, EXCHANGE_INVENTORY_ITEM);
	INIT_STATIC_PROTOPACKET(TidyInventory, TIDY_INVENTORY);
	INIT_STATIC_PROTOPACKET(EquipItem, EQUIP_ITEM);

	// 15
	INIT_STATIC_PROTOPACKET(UndoEquip, UNDO_EQUIP);
	INIT_STATIC_PROTOPACKET(UseItem, USE_ITEM);
	INIT_STATIC_PROTOPACKET(RefineEquip, REFINE_EQUIP);
	INIT_STATIC_PROTOPACKET(CombatPlayerJoin, COMBAT_PLAYER_JOIN);
	INIT_STATIC_PROTOPACKET(CombatPlayerTrigger, COMBAT_PLAYER_TRIGGER);

	//20
	INIT_STATIC_PROTOPACKET(CombatSelectSkill, COMBAT_SELECT_SKILL);
	INIT_STATIC_PROTOPACKET(FullPetPower, FULL_PET_POWER);
	INIT_STATIC_PROTOPACKET(CombatPetAttack, COMBAT_PET_ATTACK);
	INIT_STATIC_PROTOPACKET(ServiceHello, SERVICE_HELLO);
	INIT_STATIC_PROTOPACKET(ServiceServe, SERVICE_SERVE);

	//25
	INIT_STATIC_PROTOPACKET(LevelUpPetBloodline, LEVELUP_PET_BLOODLINE);
	INIT_STATIC_PROTOPACKET(LevelUpPetPowerCap, LEVELUP_PET_POWER_CAP);
	INIT_STATIC_PROTOPACKET(SetCombatPet, SET_COMBAT_PET);
	INIT_STATIC_PROTOPACKET(LearnSkill, LEARN_SKILL);
	INIT_STATIC_PROTOPACKET(LevelUpTalent, LEVELUP_TALENT);

	// 30
	INIT_STATIC_PROTOPACKET(TaskNotify, TASK_NOTIFY);
	INIT_STATIC_PROTOPACKET(TransferPrepareFinish, TRANSFER_PREPARE_FINISH);
	INIT_STATIC_PROTOPACKET(SelectResurrectPos, SELECT_RESURRECT_POS);
	INIT_STATIC_PROTOPACKET(JoinTeam, JOIN_TEAM);
	INIT_STATIC_PROTOPACKET(JoinTeamRes, JOIN_TEAM_RES);

	// 35
	INIT_STATIC_PROTOPACKET(ChangeBuddyTeamPos, CHANGE_BUDDY_TEAM_POS);
	INIT_STATIC_PROTOPACKET(GatherMaterial, GATHER_MATERIAL);
	INIT_STATIC_PROTOPACKET(GatherMiniGameResult, GATHER_MINIGAME_RESULT);
	INIT_STATIC_PROTOPACKET(OpenCatVision, OPEN_CAT_VISION);
	INIT_STATIC_PROTOPACKET(CloseCatVision, CLOSE_CAT_VISION);

	// 40
	INIT_STATIC_PROTOPACKET(DramaTriggerCombat, DRAMA_TRIGGER_COMBAT);
	INIT_STATIC_PROTOPACKET(DramaServiceServe, DRAMA_SERVICE_SERVE);
	INIT_STATIC_PROTOPACKET(DramaGatherMaterial, DRAMA_GATHER_MATERIAL);
	INIT_STATIC_PROTOPACKET(DramaGatherMiniGameResult, DRAMA_GATHER_MINIGAME_RESULT);
	INIT_STATIC_PROTOPACKET(SwitchSkill, SWITCH_SKILL);

	// 45
	INIT_STATIC_PROTOPACKET(GetElsePlayerExtProp, GET_ELSE_PLAYER_EXTPROP);
	INIT_STATIC_PROTOPACKET(GetElsePlayerEquipCRC, GET_ELSE_PLAYER_EQUIPCRC);
	INIT_STATIC_PROTOPACKET(GetElsePlayerEquipment, GET_ELSE_PLAYER_EQUIPMENT);
	INIT_STATIC_PROTOPACKET(ChatMsg, CHAT_MSG);
	INIT_STATIC_PROTOPACKET(TaskRegionTransfer, TASK_REGION_TRANSFER);

	// 50
	INIT_STATIC_PROTOPACKET(TaskChangeName, TASK_CHANGE_NAME);
	INIT_STATIC_PROTOPACKET(GetStaticRoleInfo, GET_STATIC_ROLE_INFO);
	INIT_STATIC_PROTOPACKET(GetInstanceData, GET_INSTANCE_DATA);
	INIT_STATIC_PROTOPACKET(UITaskRequest, UI_TASK_REQUEST);
	INIT_STATIC_PROTOPACKET(QueryNpcZoneInfo, QUERY_NPC_ZONE_INFO);

	// 55
	INIT_STATIC_PROTOPACKET(OpenMall, OPEN_MALL);
	INIT_STATIC_PROTOPACKET(QueryMallGoodsDetail, QUERY_MALL_GOODS_DETAIL);
	INIT_STATIC_PROTOPACKET(MallShopping, MALL_SHOPPING);
	INIT_STATIC_PROTOPACKET(ServiceGetContent, SERVICE_GET_CONTENT);
	INIT_STATIC_PROTOPACKET(QueryServerTime, QUERY_SERVER_TIME);

	// 60
	INIT_STATIC_PROTOPACKET(GetMailList, GET_MAIL_LIST);
	INIT_STATIC_PROTOPACKET(GetMailAttach, GET_MAIL_ATTACH);
	INIT_STATIC_PROTOPACKET(DeleteMail, DELETE_MAIL);
	INIT_STATIC_PROTOPACKET(TakeoffMailAttach, TAKEOFF_MAIL_ATTACH);
	INIT_STATIC_PROTOPACKET(SendMail, SEND_MAIL);

	// 65
	INIT_STATIC_PROTOPACKET(TeamLocalInsInvite, TEAM_LOCAL_INS_INVITE);
	INIT_STATIC_PROTOPACKET(LaunchTeamLocalIns, LAUNCH_TEAM_LOCAL_INS);
	INIT_STATIC_PROTOPACKET(QuitTeamLocalInsRoom, QUIT_TEAM_LOCAL_INS_ROOM);
	INIT_STATIC_PROTOPACKET(TeamCrossInsRequest, TEAM_CROSS_INS_REQUEST);
	INIT_STATIC_PROTOPACKET(QuitTeamCrossInsRoom, QUIT_TEAM_CROSS_INS_ROOM);

	// 70
	INIT_STATIC_PROTOPACKET(LaunchSoloLocalIns, LAUNCH_SOLO_LOCAL_INS);
	INIT_STATIC_PROTOPACKET(TeamLocalInsInvite_Re, TEAM_LOCAL_INS_INVITE_RE);
	INIT_STATIC_PROTOPACKET(TeamCrossInsReady, TEAM_CROSS_INS_READY);
	INIT_STATIC_PROTOPACKET(PlayerQuitInstance, PLAYER_QUIT_INSTANCE);
	INIT_STATIC_PROTOPACKET(MapTeamChangePos, MAP_TEAM_CHANGE_POS);

	// 75
	INIT_STATIC_PROTOPACKET(MapTeamChangeLeader, MAP_TEAM_CHANGE_LEADER);
	INIT_STATIC_PROTOPACKET(MapTeamQueryMember, MAP_TEAM_QUERY_MEMBER);
	INIT_STATIC_PROTOPACKET(TaskTransferGenderCls, TASK_TRANSFER_GENDER_CLS);
	INIT_STATIC_PROTOPACKET(TaskInsTransfer, TASK_INS_TRANSFER);
	INIT_STATIC_PROTOPACKET(MoveCombatPet, MOVE_COMBAT_PET);

    // 80
    INIT_STATIC_PROTOPACKET(AuctionItem, AUCTION_ITEM);
    INIT_STATIC_PROTOPACKET(AuctionCancel, AUCTION_CANCEL);
    INIT_STATIC_PROTOPACKET(AuctionBuyout, AUCTION_BUYOUT);
    INIT_STATIC_PROTOPACKET(AuctionBid, AUCTION_BID);
    INIT_STATIC_PROTOPACKET(AuctionQueryList, AUCTION_QUERY_LIST);

    // 85
    INIT_STATIC_PROTOPACKET(AuctionQueryBidPrice, AUCTION_QUERY_BID_PRICE);
    INIT_STATIC_PROTOPACKET(AuctionQueryDetail, AUCTION_QUERY_DETAIL);
    INIT_STATIC_PROTOPACKET(LevelUpCard, LEVELUP_CARD);
    INIT_STATIC_PROTOPACKET(GetSelfAuctionItem, GET_SELF_AUCTION_ITEM);
    INIT_STATIC_PROTOPACKET(GetSelfBidItem, GET_SELF_BID_ITEM);

    // 90
    INIT_STATIC_PROTOPACKET(AddFriend, ADD_FRIEND);
    INIT_STATIC_PROTOPACKET(DeleteFriend, DELETE_FRIEND);
    INIT_STATIC_PROTOPACKET(QueryRoleInfo, QUERY_ROLEINFO);
    INIT_STATIC_PROTOPACKET(ConveneTeammate, CONVENE_TEAMMATE);
    INIT_STATIC_PROTOPACKET(ConveneResponse, CONVENE_RESPONSE);

    // 95
    INIT_STATIC_PROTOPACKET(TeamCrossInsInvite_Re, TEAM_CROSS_INS_INVITE_RE);
    INIT_STATIC_PROTOPACKET(DuelRequest, DUEL_REQUEST);
    INIT_STATIC_PROTOPACKET(DuelRequest_Re, DUEL_REQUEST_RE);
    INIT_STATIC_PROTOPACKET(TeammateDuelRequest_Re, TEAMMATE_DUEL_REQUEST_RE);
    INIT_STATIC_PROTOPACKET(SwitchTitle, SWITCH_TITLE);

    // 100
    INIT_STATIC_PROTOPACKET(QueryPrimaryPropRF, QUERY_PRIMARY_PROP_RF);
    INIT_STATIC_PROTOPACKET(BuyPrimaryPropRFEnergy, BUY_PRIMARY_PROP_RF_ENERGY);
    INIT_STATIC_PROTOPACKET(PrimaryPropReinforce, PRIMARY_PROP_REINFORCE);
    INIT_STATIC_PROTOPACKET(AchieveNotify, ACHIEVE_NOTIFY);
    INIT_STATIC_PROTOPACKET(OpenEnhanceSlot, OPEN_ENHANCE_SLOT);

    // 105
    INIT_STATIC_PROTOPACKET(ProtectEnhanceSlot, PROTECT_ENHANCE_SLOT);
    INIT_STATIC_PROTOPACKET(UnProtectEnhanceSlot, UNPROTECT_ENHANCE_SLOT);
    INIT_STATIC_PROTOPACKET(ActivateSpark, ACTIVATE_SPARK);
    INIT_STATIC_PROTOPACKET(PlayerQuitBattleGround, PLAYER_QUIT_BATTLEGROUND);
    INIT_STATIC_PROTOPACKET(GetBattleGroundData, GET_BATTLEGROUND_DATA);

    // 110
    INIT_STATIC_PROTOPACKET(EquipCard, EQUIP_CARD);
    INIT_STATIC_PROTOPACKET(MapTeamJoinTeam, MAP_TEAM_JOIN_TEAM);
    INIT_STATIC_PROTOPACKET(MapTeamLeaveTeam, MAP_TEAM_LEAVE_TEAM);
    INIT_STATIC_PROTOPACKET(MapTeamKickoutMember, MAP_TEAM_KICKOUT_MEMBER);
    INIT_STATIC_PROTOPACKET(MapTeamJoinTeamRes, MAP_TEAM_JOIN_TEAM_RES);

    // 115
    INIT_STATIC_PROTOPACKET(UpdateTaskStorage, UPDATE_TASK_STORAGE);
    INIT_STATIC_PROTOPACKET(ArenaODQueryData, ARENA_OD_QUERY_DATA);
    INIT_STATIC_PROTOPACKET(ArenaODInviteAssist, ARENA_OD_INVITE_ASSIST);
    INIT_STATIC_PROTOPACKET(ArenaODAssistSelect, ARENA_OD_ASSIST_SELECT);
    INIT_STATIC_PROTOPACKET(ArenaODCombatStart, ARENA_OD_COMBAT_START);

    // 120
    INIT_STATIC_PROTOPACKET(PlayerPunchCard, PLAYER_PUNCH_CARD);
    INIT_STATIC_PROTOPACKET(GainPunchCardAward, GAIN_PUNCH_CARD_AWARD);
    INIT_STATIC_PROTOPACKET(QueryInstanceRecord, QUERY_INSTANCE_RECORD);
    INIT_STATIC_PROTOPACKET(QueryPunchCardData, QUERY_PUNCH_CARD_DATA);
    INIT_STATIC_PROTOPACKET(JoinGevent, JOIN_GEVENT);

    // 125
    INIT_STATIC_PROTOPACKET(RePunchCardHelp_Re, RE_PUNCH_CARD_HELP_RE);
    INIT_STATIC_PROTOPACKET(ItemDisassemble, ITEM_DISASSEMBLE);
    INIT_STATIC_PROTOPACKET(MountMount, MOUNT_MOUNT);
    INIT_STATIC_PROTOPACKET(MountExchange, MOUNT_EXCHANGE);
    INIT_STATIC_PROTOPACKET(MountEquipLevelUp, MOUNT_EQUIP_LEVELUP);

    // 130
    INIT_STATIC_PROTOPACKET(GetParticipationAward, GET_PARTICIPATION_AWARD);
    INIT_STATIC_PROTOPACKET(BossChallenge, BOSS_CHALLENGE);
    INIT_STATIC_PROTOPACKET(GetBossChallengeAward, GET_BOSS_CHALLENGE_AWARD);
    INIT_STATIC_PROTOPACKET(GetClearChallengeAward, GET_CLEAR_CHALLENGE_AWARD);
    INIT_STATIC_PROTOPACKET(QueryWorldBossRecord, QUERY_WORLD_BOSS_RECORD);

    // 135
    INIT_STATIC_PROTOPACKET(TaskBGTransfer, TASK_BG_TRANSFER);
    INIT_STATIC_PROTOPACKET(ArenaODBuyTicket, ARENA_OD_BUY_TICKET);
	

	//
	// 从这里开始为调试CMD的定义
	//
	// start cmd no: 21700
	// 0
	INIT_STATIC_PROTOPACKET(DebugCommonCmd, DEBUG_COMMON_CMD);
	INIT_STATIC_PROTOPACKET(DebugGainItem, DEBUG_GAIN_ITEM);
	INIT_STATIC_PROTOPACKET(DebugCleanAllItem, DEBUG_CLEAN_ALL_ITEM);
	INIT_STATIC_PROTOPACKET(DebugChangePlayerPos, DEBUG_CHANGE_PLAYER_POS);
    INIT_STATIC_PROTOPACKET(DebugChangeFirstName, DEBUG_CHANGE_FIRST_NAME);

} // namespace C2G
