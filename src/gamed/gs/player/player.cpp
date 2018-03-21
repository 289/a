#include "player.h"

#include "shared/base/rwlock.h"
#include "shared/lua/lua_engine_lite.h"

#include "game_module/task/include/task.h"
#include "game_module/skill/include/world_buff.h"
#include "gs/global/glogger.h"
#include "gs/global/timer.h"
#include "gs/global/gmatrix.h"
#include "gs/global/randomgen.h"
#include "gs/global/game_types.h"
#include "gs/global/game_util.h"
#include "gs/global/game_lua_ent.h"
#include "gs/global/achieve_config.h"
#include "gs/movement/player_move_ctrl.h"
#include "gs/scene/world.h"
#include "gs/netmsg/send_to_master.h"
#include "gs/obj/faction.h"
#include "gs/player/filter/invincible_filter.h"
#include "gs/player/filter/property_filter.h"
#include "gs/player/filter/transform_filter.h"
#include "gs/player/subsys/player_inventory.h"
#include "gs/player/subsys/player_combat.h"
#include "gs/player/subsys/player_skill.h"
#include "gs/player/subsys/player_service.h"
#include "gs/player/subsys/player_team.h"
#include "gs/player/subsys/player_pet.h"
#include "gs/player/subsys/player_transfer.h"
#include "gs/player/subsys/player_task.h"
#include "gs/player/subsys/player_instance.h"
#include "gs/player/subsys/player_buddy.h"
#include "gs/player/subsys/player_buff.h"
#include "gs/player/subsys/player_mall.h"
#include "gs/player/subsys/player_mail.h"
#include "gs/player/subsys/player_map_team.h"
#include "gs/player/subsys/player_chat.h"
#include "gs/player/subsys/player_talent.h"
#include "gs/player/subsys/player_auction.h"
#include "gs/player/subsys/player_friend.h"
#include "gs/player/subsys/player_cooldown.h"
#include "gs/player/subsys/player_reputation.h"
#include "gs/player/subsys/player_achieve.h"
#include "gs/player/subsys/player_duel.h"
#include "gs/player/subsys/player_enhance.h"
#include "gs/player/subsys/player_prop_reinforce.h"
#include "gs/player/subsys/player_title.h"
#include "gs/player/subsys/player_star.h"
#include "gs/player/subsys/player_battleground.h"
#include "gs/player/subsys/player_landmine.h"
#include "gs/player/subsys/player_card.h"
#include "gs/player/subsys/player_bw_list.h"
#include "gs/player/subsys/player_counter.h"
#include "gs/player/subsys/player_punch_card.h"
#include "gs/player/subsys/player_gevent.h"
#include "gs/player/subsys/player_mount.h"
#include "gs/player/subsys/player_participation.h"
#include "gs/player/subsys/player_boss_challenge.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/player_class_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/global_counter.h"
#include "gs/template/data_templ/monster_templ.h"

#include "cmdprepare.h"
#include "player_ctrl.h"
#include "player_sender.h"
#include "inter_msg_filter.h"
#include "subsys_if.h"
#include "player_subsys.h"
#include "master_rpc.h"
#include "psession.h"
#include "task_if.h"
#include "fixed_item_def.h"


namespace gamed {

using namespace common;
using namespace playerdef;
using namespace dataTempl;

#define GET_SUBSYS_PTR(subclass, subtype) \
    GetSubSysPtr<subclass>(PlayerSubSystem::SUB_SYS_TYPE_##subtype);


CmdPacketDispatcher Player::s_cmd_dispatcher_;


namespace {

    static const size_t kMaxPlayerLuaEntCount = 16;
	static const char* kPlayerCombatValueFile = "player_combat_value.lua";
    static const char* kCombatValueFuncPrefix = "getPlayerCombatValue_";
	static size_t lua_entity_rr_key           = 0;
	static shared::net::FixedArray<GameLuaEntity, kMaxPlayerLuaEntCount> g_player_lua_ent;

	GameLuaEntity* GetLuaEntityRR()
	{
		return &g_player_lua_ent[++lua_entity_rr_key % kMaxPlayerLuaEntCount];
	}
		
} // Anonymous


///
/// static function
///
void Player::InitPlayerScriptSys(const char* script_dir)
{
    std::string path = std::string(script_dir) + kPlayerCombatValueFile;
    for (size_t i = 0; i < kMaxPlayerLuaEntCount; ++i)
    {
        if (!g_player_lua_ent[i].LoadFile(path.c_str()))
        {
            LOG_ERROR << "Player::InitPlayerScriptSys() error! script_file:" << path;
            break;
        }
    }
}


///
/// Player
///
Player::Player()
	: commander_(NULL),
	  move_ctrl_(NULL),
	  sender_(NULL),
	  subsys_if_(NULL),
	  rpc_master_(NULL),
	  extend_state_(0),
	  equip_crc_(0),
	  refresh_state_(false),
	  refresh_extprop_(0),
	  link_id_(-1),
	  sid_in_link_(-1),
	  master_id_(-1),
      user_id_(-1),
	  faction_(0),
	  write_timer_(0),
	  hp_gen_counter_(0),
	  ep_gen_counter_(0),
	  vip_level_(0),
	  cat_vision_state_(false),
	  pimp_(NULL)
{
	memset(base_prop_, 0, sizeof(base_prop_));
	memset(cur_prop_, 0, sizeof(cur_prop_));
	memset(enh_point_, 0, sizeof(enh_point_));
	memset(enh_scale_, 0, sizeof(enh_scale_));
	memset(equip_point_, 0, sizeof(equip_point_));
	memset(equip_scale_, 0, sizeof(equip_scale_));
}

Player::~Player()
{
}

void Player::Release()
{
	link_id_             = -1;
	sid_in_link_         = -1;
	master_id_           = -1;
	faction_             = FACTION_PLAYER;
	write_timer_         = 0;
	hp_gen_counter_      = 0;
	ep_gen_counter_      = 0;
	vip_level_           = 0;
	cat_vision_state_    = false;
	equip_crc_           = 0;
    extend_state_        = 0;
    refresh_state_       = false;
    refresh_extprop_     = 0;

    value_prop_.Reset();
	state_.Release();
	visible_state_.Release();
	else_players_set_.clear();
	spec_save_pos_.clear();
	rule_areas_vec_.clear();
	area_rules_info_.Reset();

	ClrRefreshState();
	ClrRefreshExtProp();

	subsys_if_->Release();

	memset(base_prop_, 0, sizeof(base_prop_));
	memset(cur_prop_, 0, sizeof(cur_prop_));
	memset(enh_point_, 0, sizeof(enh_point_));
	memset(enh_scale_, 0, sizeof(enh_scale_));
	memset(equip_point_, 0, sizeof(equip_point_));
	memset(equip_scale_, 0, sizeof(equip_scale_));

	// 小心删除顺序，析构里不要调用别的模块
	SAFE_DELETE(pimp_);
	SAFE_DELETE(subsys_if_);
	SAFE_DELETE(move_ctrl_);
	SAFE_DELETE(commander_);
	SAFE_DELETE(rpc_master_);
	SAFE_DELETE(sender_);

	Unit::Release();
}

int Player::InitPlayerData(RoleID roleid, const char* pbuf, int32_t len)
{
	PlayerData playerdata;
	if (playerdata.Deserialize(pbuf, len))
	{
		LOG_ERROR << "playerdata.Deserialize() PlayerData deserialize error";
		return -1;
	}

	// check first login, and fill PlayerData
	if (!HandleFirstLogin(&playerdata))
	{
		LOG_ERROR << "HandleFirstLogin() error!";
		return -2;
	}

	// load DB-data
	if (!LoadFromDatabaseFormat(playerdata))
	{
		LOG_ERROR << "LoadAllFromDB() error!";
		return -3;
	}

	// always the last!
	if (!InitPlayerRuntimeData())
	{
		LOG_ERROR << "InitPlayerRuntimeData() error!";
		return -4;
	}

	//刷新玩家属性数据
	//这里需要手动刷新，否则可能导致玩家上线取到的属性数据不正确。
	PropertyPolicy::UpdatePlayerProp(this);

    // 上线计算一次战斗力
    combat_value_ = CalcCombatValue();
	return 0;
}

bool Player::InitPlayerRuntimeData()
{
    if (!value_prop_.Init())
        return false;

	//计算玩家基本属性
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(base_info_.cls);
	const dataTempl::PlayerClassTempl* pTpl = cls_default.templ_ptr;
	for (size_t i = 0; i < pTpl->properties.size(); ++i)
	{
		SetRefreshExtProp(i);
		base_prop_[i]  = pTpl->properties[i].value;
		base_prop_[i] += pTpl->properties[i].levelup_inc * (base_info_.level); 
	}
    
	//初始化玩家死亡状态
	if (base_info_.hp <= 0)
	{
		state_.SetDead();
		visible_state_.SetDeadFlag();
	}

	faction_     = FACTION_PLAYER;
	write_timer_ = mrand::Rand(60, 173);
	vip_level_   = 1;
	return true;
}

bool Player::InitRuntimeModule(RoleID roleid)
{
	link_id_     = -1;
	sid_in_link_ = -1;
	master_id_   = -1;
	set_xid(roleid, XID::TYPE_PLAYER);

	commander_ = new PlayerController(*this);
	if (NULL == commander_) 
	{
		LOG_ERROR << "new PlayerController() error!";
		return false;
	}

	move_ctrl_ = new PlayerMoveControl(*this);
	if (NULL == move_ctrl_) 
	{
		LOG_ERROR << "new PlayerMoveControl() error!";
		return false;
	}

	sender_ = new PlayerSender(*this);
	if (NULL == sender_)
	{
		LOG_ERROR << "new PlayerSender() error!";
		return false;
	}

	subsys_if_ = new SubSysIf(*this);
	if (NULL == subsys_if_ || !subsys_if_->InitSubSys())
	{
		LOG_ERROR << "new SubSysIf() error!";
		return false;
	}

	rpc_master_ = new MasterRpcProxy(roleid, *this);
	if (NULL == rpc_master_)
	{
		LOG_ERROR << "new MasterRpcProxy() error!";
		return false;
	}

	pimp_ = new PlayerImp(*this);
	if (NULL == pimp_)
	{
		LOG_ERROR << "new PlayerImp() error!";
		return false;
	}

	return true;
}

void Player::FillRumtimeData(const PlayerData& data)
{
	// Fill player runtime data
	ASSERT(object_id() == role_info_.roleid);
	set_world_id(data.location.world_id, data.location.world_tag);

	A2DVECTOR tmp_pos(data.location.x, data.location.y);
	set_pos(tmp_pos);
	set_dir(data.location.dir);
}

void Player::TransferCls(int32_t dest_cls)
{
	// 角色转职
	if (dest_cls < 0 || dest_cls >= playerdef::CLS_MAXCLS_LABEL)
		return;

	if (dest_cls == base_info_.cls)
	{
        sender_->ErrorMessage(G2C::ERR_SAME_CLS);
		return;
	}

	// 获取新职业模板
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(dest_cls);
	const dataTempl::PlayerClassTempl* pTpl = cls_default.templ_ptr;
	if (!pTpl)
	{
		__PRINTF("玩家 %ld 转职到职业 %d 失败", role_id(), dest_cls);
		return;
	}

	// 更新职业
    int32_t old_cls = base_info_.cls;
	base_info_.cls  = dest_cls;

	// 更新玩家基础属性
	for (size_t i = 0; i < pTpl->properties.size(); ++i)
	{
		SetRefreshExtProp(i);
		base_prop_[i]  = pTpl->properties[i].value;
		base_prop_[i] += pTpl->properties[i].levelup_inc * (base_info_.level); 
	}

	// 转职成功调用子系统
	FOREACH_SUBSYS_0(subsys_if_, OnTransferCls);

	// 刷新玩家属性和速度
    UpdatePropertyAndSpeed();

	// 通知客户端
    //sender_->PlayerTransferCls();
    std::vector<int32_t> visible_list;
    visible_list.push_back(dest_cls);
    sender_->PlayerVisibleInfoChange(G2C::PlayerVisibleInfoChange::VM_ROLE_CLS, visible_list);

	// 同步技能数据给客户端
	PlayerGetSkillData();

    GLog::log("玩家 %ld 从职业:%d 转职到:%d", role_id(), old_cls, dest_cls);
}

void Player::TransferGender(uint8_t gender)
{
	if (gender < 0 || gender > 1)
	{
		return;
	}

	if (base_info_.gender != gender)
	{
		base_info_.gender = gender;
	}

	//通知客户端
	//sender_->PlayerTransferGender();
    std::vector<int32_t> visible_list;
    visible_list.push_back(gender);
    sender_->PlayerVisibleInfoChange(G2C::PlayerVisibleInfoChange::VM_ROLE_GENDER, visible_list);

    GLog::log("玩家 %ld 变性到%d", role_id(), gender);
}

bool Player::LoadFromDatabaseFormat(const PlayerData& data)
{
	role_info_    = data.role_info;
	base_info_    = data.base_info;
	preload_data_ = data.preload_data;
	spec_money_	  = data.spec_money;

    if ((int32_t)base_info_.ui_config.length() != MAX_UI_NUM)
    {
        ASSERT((int32_t)base_info_.ui_config.length() < MAX_UI_NUM);
        base_info_.ui_config.resize(MAX_UI_NUM);
    }

	FillRumtimeData(data);

	// always last!
	if (!subsys_if_->LoadFromDB(data))
	{
		LOG_ERROR << "subsys_if_->LoadFromDB(data) error!";
		return false;
	}

    return true;
}

void Player::CopyRuntimeData(PlayerData* pdata)
{
	// copy runtime data from WorldObject
	
	///
	/// location data: 注意优先级，靠前的优先级高
	///
	int32_t spec_world_id;
	A2DVECTOR spec_pos;
	int32_t drama_world_id;
	A2DVECTOR drama_pos;
	if (GetSpecSavePos(spec_world_id, spec_pos))
	{
		pdata->location.world_id  = spec_world_id;
		pdata->location.world_tag = 0;
		pdata->location.x         = spec_pos.x;
		pdata->location.y         = spec_pos.y;
	}
	else if (GetDramaSavePos(drama_world_id, drama_pos))
	{
		pdata->location.world_id  = drama_world_id;
		pdata->location.world_tag = 0;
		pdata->location.x         = drama_pos.x;
		pdata->location.y         = drama_pos.y;
	}
	else
	{
		pdata->location.world_id  = world_id();
		pdata->location.world_tag = world_tag();
		pdata->location.x         = pos().x;
		pdata->location.y         = pos().y;
	}
	pdata->location.dir = dir();
    ASSERT(IS_NORMAL_MAP(pdata->location.world_id));
	ASSERT(pdata->location.world_tag == 0);
}

bool Player::SaveToDatabaseFormat(PlayerData* pdata)
{
	pdata->role_info    = role_info_;
    pdata->role_info.last_logout_wid = world_id();
	pdata->base_info    = base_info_;
    pdata->base_info.combat_value = CalcCombatValue(); // 要在base_info_赋值之后，因为战斗力要现算
	pdata->preload_data = preload_data_;
	pdata->spec_money   = spec_money_;

	CopyRuntimeData(pdata);

	// always the last!
	if (!subsys_if_->SaveToDB(pdata))
	{
		LOG_ERROR << "subsys_if_->SaveToDB(pdata) error!";
		return false;
	}

	return true;
}

bool Player::GetPlayerDataContent(shared::net::Buffer* buf)
{
	return GetPlayerDataOctetStr(buf, false);
}

bool Player::GetPlayerDataOctetStr(shared::net::Buffer* buf, bool allow_logout)
{
	PlayerData tmpdata;
	if (!SaveToDatabaseFormat(&tmpdata))
	{
		GLog::log("Player:%ld get playerdata error, in Player::GetPlayerDataOctetStr()", role_id());
		if (allow_logout) {
			PlayerLogout(LT_FATAL_ERROR);
		}
		return false;
	}

	if (tmpdata.Serialize(buf) != 0)
	{
		GLog::log("Player:%ld pdata_->Serialize() PlayerData serialize error, in Player::GetPlayerDataOctetStr()", role_id());
		if (allow_logout) {
			PlayerLogout(LT_FATAL_ERROR);
		}
		return false;
	}

	return true;
}

void Player::PlayerDataSyncToMaster(SaveDataOpType op_type)
{
	shared::net::Buffer tmpdata;
	if (!GetPlayerDataOctetStr(&tmpdata))
	{
		LOG_ERROR << "get playerdata error, in Player::PlayerDataSyncToMaster() error";
		return;
	}

	if (op_type == SDOP_LOGOUT)
	{
		rpc_master_->LogoutSaveData(tmpdata);
	}
	else if (op_type == SDOP_HEARTBEAT)
	{
		rpc_master_->SavePlayerData(tmpdata);
	}
	else
	{
		ASSERT(false);
	}
}

int Player::DispatchCommand(int cmd_type_no, ProtoPacket* packet)
{
	return s_cmd_dispatcher_.DispatchCommand(this, cmd_type_no, packet);
}

void Player::EnterWorld()
{
	__PRINTF("玩家%ld登陆世界EnterWorld", role_id());

	EnterMap();

	FOREACH_SUBSYS_0(subsys_if_, OnEnterWorld);

	pimp_->OnEnterWorld();
}

void Player::LeaveWorld()
{
	__PRINTF("玩家%ld离开世界LeaveWorld", role_id());

    // 先离开地图
	LeaveMap();

    // 在子系统之前调用，因为子系统里一些存盘数据可能在这里改
	pimp_->OnLeaveWorld();

    // subsys
	FOREACH_SUBSYS_0(subsys_if_, OnLeaveWorld);
}

void Player::EnterMap()
{
	A2DVECTOR init_pos(pos().x, pos().y);
	int64_t param = pack_linkid_sid(link_id(), sid_in_link());
	world_plane()->InsertObjToAOI(object_xid(), init_pos, param);
	
	// 在刷新自己的area信息
	RefreshAreaRulesInfo(0, false);

	//添加无敌Buff
	AddSessionAndStart(new PLogonInvincibleSession(this, INVINCIBLE_BUFF_TIME));

	__PRINTF("玩家%ld进入地图:%d", role_id(), world_id());
}
	
void Player::LeaveMap()
{
	world_plane()->RemoveObjFromAOI(object_xid());

	// 清除自己的area信息
	ResetAreaRulesInfo();

	// 关闭瞄类视觉
	DeActivateCatVision();

	// 清除视野内的else player
	ClearElsePlayerInView();

	// 被动标记不带过地图
	passivity_flag_.clear();

    // 清除身上的暗雷数据
    ClearLandmineTrigger();

    // 清除地图计数器的订阅列表
    ClearMapCounterList();

	__PRINTF("玩家%ld离开地图:%d", role_id(), world_id());
}

void Player::ElsePlayerEnterView(RoleID playerid, int32_t linkid, int32_t sid)
{
	if (!else_players_set_.insert(playerid).second)
	{
		//ASSERT(false);
		LOG_WARN << playerid << " enter view error in player:" << role_id();
		return;
	}

	sender_->ElsePlayerEnterView(playerid, linkid, sid);
}

void Player::ElsePlayerLeaveView(RoleID id)
{
	size_t n = else_players_set_.erase(id);
	//(void)n; assert(n == 1);
	if (n != 1)
	{
		LOG_WARN << id << " leave view error in player:" << role_id();
		return;
	}

	sender_->ElsePlayerLeaveView(id);
}

void Player::ClearElsePlayerInView()
{
	else_players_set_.clear();
}

void Player::MasterRpcMessageProc(const shared::net::RpcMessage& msg)
{
	rpc_master_->RpcMessageProc(msg);
}

int Player::OnDispatchMessage(const MSG& msg)
{
	return InterMsgFilter::DispatchMessage(this, msg);
}

void Player::SyncDispatchMsg(const MSG& msg)
{
	OnDispatchMessage(msg);
}

void Player::OnGetAllData()
{
    // 子系统不要在这里做getalldata，尽量显示的调用
	// imp里需要处理刚上线的情况，多半是副本等特殊地图
	pimp_->OnGetAllData();
}

void Player::ChangeGsComplete()
{
	// 重新发Npc伙伴信息
	PlayerGetBuddyInfo();

	// 发副本组队信息
	PlayerGetInsTeamInfo();

    // 开始发地图任务
    StartRecvWorldTask();

	// imp里需要处理换gs的情况，多半是副本等特殊地图
	pimp_->OnChangeGsComplete();
}

void Player::PlayerLogout(LogoutType type)
{
	// set offline status
	state_.SetOffline();

	// leave world before save data
	LeaveWorld();

	// logout type
	switch (type)
	{
		case LT_DB_SAVE_ERROR:
			{
				sender_->DBSaveErrorLogout();
				rpc_master_->LogoutWithoutSaveData();
			}
			break;

		case LT_FATAL_ERROR:
			{
				sender_->PlayerFatalError();
				rpc_master_->LogoutWithoutSaveData();
			}
			break;

		case LT_MASTER_DISCONNECT:
			{
				rpc_master_->LogoutWithoutSaveData();
			}
			break;

		case LT_CHANGE_MAP:
			{
				rpc_master_->LogoutWithoutSaveData();
			}
			break;

		default:
			{
				// sync to master
				PlayerDataSyncToMaster(SDOP_LOGOUT);
			}
			break;
	}
}

bool Player::HandleFirstLogin(PlayerData* pdata)
{
	if (pdata->role_info.created_time == 0)
	{
		// 从模板载入数据
		if (!LoadPlayerDataFromTempl(pdata))
			return false;

		// 初始化包裹栏大小
		pdata->inventory.inventory_cap = INIT_INVENTORY_SIZE;
	}

	// 不是首次登陆或者首次登陆Load数据正确
	return true;
}

bool Player::LoadPlayerDataFromTempl(PlayerData* pdata)
{
	int cls = pdata->base_info.cls;
	if (cls < playerdef::CLS_NEWBIE || cls >= playerdef::CLS_MAXCLS_LABEL)
	{
		ASSERT(false && "获取的职业id不在有效范围内");
	}

	// 首次创建需要给PlayerData填入职业模板数据
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(pdata->base_info.cls);

	/// fill PlayerData by default template
	pdata->base_info.hp  = cls_default.templ_ptr->properties[PROP_INDEX_MAX_HP].value + cls_default.templ_ptr->properties[PROP_INDEX_MAX_HP].levelup_inc;//待定
	pdata->base_info.mp  = cls_default.templ_ptr->properties[PROP_INDEX_MAX_MP].value;
	pdata->base_info.ep  = cls_default.templ_ptr->properties[PROP_INDEX_MAX_EP].value;
	pdata->base_info.exp = 0;
	pdata->base_info.level = 1;
	pdata->base_info.money = 0;
	pdata->base_info.cat_exp = 0;
	pdata->base_info.cat_level = 1;

	// 位置
	pdata->location.dir       = cls_default.dir;
	pdata->location.x         = pdata->location.x;
	pdata->location.y         = pdata->location.y;
	pdata->location.world_id  = pdata->location.world_id;
	pdata->location.world_tag = pdata->location.world_tag;

	const dataTempl::PlayerClassTempl* cls_templ_ptr = cls_default.templ_ptr;
	if (!cls_templ_ptr)
	{
		return false;
	}

	// 初始化职业初始技能
	for (size_t i = 0; i < cls_templ_ptr->initial_skill_trees.size(); ++ i)
	{
		int32_t sk_tree_id = cls_templ_ptr->initial_skill_trees[i];
		for (size_t j = 0; j < cls_templ_ptr->skill_trees_base.size(); ++ j) {
			if (cls_templ_ptr->skill_trees_base[j] == sk_tree_id) {
				common::PlayerSkillData::SkillTree entry;
				entry.id = sk_tree_id;
				entry.level = 1;
				entry.active = true;
				pdata->skill_data.sk_tree_list.push_back(entry);

				break;
			}
		}
	}

    const dataTempl::GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
    ASSERT(pTpl);

    //初始化宠物能量上限值
    pdata->pet_data.pet_power_cap = pTpl->pet_lvlup_power_config.init_pet_power_up_limit;

	// 修改创建时间
	pdata->role_info.created_time = g_timer->GetSysTime();
	return true;
}

void Player::OnHeartbeat()
{
	time_t cur_time = g_timer->GetSysTime();

	// rpc timeout check
	CheckRpcTimeout(cur_time);

	// check time and save to DB
	CheckTimeToSaveData();

	// 刷新运行时易变数据
	DriveRuntimeVolatileData();

	// 暗雷累积值
	LandmineAccumulate();

	// 检查被动标记超时
	CheckPassivityFlag();
	
	// 对子系统做heartbeat
	FOREACH_SUBSYS_1(subsys_if_, OnHeartbeat, cur_time);

	// imp heartbeat always at last
	pimp_->OnHeartbeat();
}

void Player::DriveRuntimeVolatileData()
{
	// 每2秒恢复一次血量
	if (GetHP() < GetMaxHP() && state_.IsNormal())
	{
		if (++ hp_gen_counter_ == HP_GEN_INTERVAL)
		{
			int32_t hp_gen = PropertyPolicy::GetHPGen(this);
			GenHP(hp_gen);
			hp_gen_counter_ = 0;
		}
	}

	// 处理瞄类视觉
	if (!InCombat())
	{
		if (cat_vision_state_)
		{
			//消耗瞄类视觉精力
			int32_t ep_use = Gmatrix::GetCatVisionEPSpeedUse();
			DecEP(ep_use);
			if (GetEP() <= 0)
			{
				//关闭瞄类视觉
				DeActivateCatVision();
			}
		}
		else if (GetEP() < GetMaxEP())
		{
			//恢复瞄类视觉精力
			if (++ ep_gen_counter_ == value_prop_.GetCatVisionGenInterval())
			{
				int32_t ep_gen = value_prop_.GetCatVisionGenSpeed();
				GenEP(ep_gen);
				ep_gen_counter_ = 0;
			}
		}
	}

	// 同步扩展属性给客户端
	if (GetRefreshExtProp())
	{
	    // 刷新玩家属性和速度
        UpdatePropertyAndSpeed();
		PlayerGetExtendProp();
		ClrRefreshExtProp();
	}

	// 同步易变信息给客户端
	if (GetRefreshState())
	{
		PlayerGetVolatileInfo();
		ClrRefreshState();
	}
}

void Player::CheckRpcTimeout(time_t cur_time)
{
	rpc_master_->TimeoutHeartbeat(cur_time);
}

PlayerSubSystem* Player::QuerySubSys(int type)
{
	return subsys_if_->QuerySubSys(type);
}

const PlayerSubSystem* Player::QuerySubSys(int type) const
{
	return subsys_if_->QuerySubSys(type);
}

void Player::PlayerGetCoolDownData()
{
    PlayerCoolDown* pCooldown = GET_SUBSYS_PTR(PlayerCoolDown, COOLDOWN);
    pCooldown->PlayerGetCoolDownData();
}

void Player::SetCoolDown(int id, int msec)
{
    PlayerCoolDown* pCooldown = GET_SUBSYS_PTR(PlayerCoolDown, COOLDOWN);
    pCooldown->SetCoolDown(id, msec);
}

void Player::ClrCoolDown(int id)
{
    PlayerCoolDown* pCooldown = GET_SUBSYS_PTR(PlayerCoolDown, COOLDOWN);
    pCooldown->ClrCoolDown(id);
}

bool Player::TestCoolDown(int index)
{
    PlayerCoolDown* pCooldown = GET_SUBSYS_PTR(PlayerCoolDown, COOLDOWN);
    return pCooldown->TestCoolDown(index);
}

bool Player::TestItemCoolDown(int cd_group_id, int item_id)
{
    PlayerCoolDown* pCooldown = GET_SUBSYS_PTR(PlayerCoolDown, COOLDOWN);
    return pCooldown->TestItemCoolDown(cd_group_id, item_id);
}

void Player::PlayerGetCombatData()
{
    PlayerCombat* pCombat = GET_SUBSYS_PTR(PlayerCombat, COMBAT);
	pCombat->PlayerGetCombatData();
}

void Player::PlayerGetBuddyInfo()
{
    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	pbuddy->StartRecvBuddyInfo();
}

void Player::PlayerGetInsTeamInfo()
{
	PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL)
	{
		pmap_team->StartRecvMapTeamInfo();
	}
}

void Player::PlayerGetStaticRoleInfo()
{
	sender_->SelfStaticRoleInfo();
}

void Player::PlayerGetSkillData()
{
    PlayerSkill* pSkill = GET_SUBSYS_PTR(PlayerSkill, SKILL);
	pSkill->PlayerGetSkillData();
}

void Player::PlayerGetTaskData()
{
    PlayerTask* pTask = GET_SUBSYS_PTR(PlayerTask, TASK);
	pTask->PlayerGetTaskData();
    // flush the waiting task
    pTask->StartRecvWorldTask();
}

void Player::PlayerGetPetData()
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->PlayerGetPetData();
}

void Player::PlayerGetTalentData()
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->PlayerGetTalentData();
}

void Player::PlayerGetFriendData()
{
    PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
    pFriend->PlayerGetFriendData();
}

void Player::PlayerGetTitleData()
{
    PlayerTitle* pTitle = GET_SUBSYS_PTR(PlayerTitle, TITLE);
    pTitle->PlayerGetTitleData();
}

void Player::PlayerGetReputationData()
{
    PlayerReputation* pReputation = GET_SUBSYS_PTR(PlayerReputation, REPUTATION);
    pReputation->PlayerGetReputationData();
}

void Player::LevelUpSkill()
{
    PlayerSkill* pSkill = GET_SUBSYS_PTR(PlayerSkill, SKILL);
	pSkill->LevelUpAuto();
}

void Player::LevelUpSkill(int32_t sk_tree_id, int8_t lvl, int8_t lvlup_mode)
{
    PlayerSkill* pSkill = GET_SUBSYS_PTR(PlayerSkill, SKILL);
	pSkill->LevelUpTrigger(sk_tree_id, lvl, lvlup_mode);
}

void Player::LevelUpAllSkill(int8_t lvl, int8_t lvlup_mode)
{
    PlayerSkill* pSkill = GET_SUBSYS_PTR(PlayerSkill, SKILL);
	pSkill->LevelUpAllSkill(lvl, lvlup_mode);
}

void Player::QueryAvailableSkillTree(std::vector<playerdef::SkillTreeInfo>& skill_tree_info_list) const
{
    const PlayerSkill* pSkill = GET_SUBSYS_PTR(PlayerSkill, SKILL);
	pSkill->QueryAvailableSkillTree(skill_tree_info_list);
}

void Player::CastSkill(int32_t skill_id)
{
    //TODO
}

typedef void (Player::*QuerySkillFunc)(std::set<int32_t>& skills) const;
static QuerySkillFunc GetQuerySkillFunc(int32_t type)
{
    switch (type)
    {
    case 0:
        return &Player::QueryTalentSkill;
    case 1:
        return &Player::QueryTitleSkill;
    case 2:
        return &Player::QueryEnhanceSkill;
    case 3:
        return &Player::QueryCardSkill;
    default:
        return NULL;
    }
}

int32_t Player::IsSkillExist(int32_t skill_id) const
{
    // 查找基础技能系统是否存在
	std::vector<playerdef::SkillTreeInfo> skill_info_vec;
	QueryAvailableSkillTree(skill_info_vec);
	for (size_t i = 0; i < skill_info_vec.size(); ++ i)
	{
        if (skill_info_vec[i].skill_id == skill_id)
        {
            return 1;
        }
	}
    for (int32_t i = 0; i < 4; ++i)
    {
        std::set<int32_t> skill_set;
        QuerySkillFunc func = GetQuerySkillFunc(i);
        (this->*(func))(skill_set);
        if (skill_set.find(skill_id) != skill_set.end())
        {
            return 1;
        }
    }
    /*// 查找天赋技能是否存在
    for (size_t i = 0; i < skill_vec.size(); ++i)
    {
        if (skill_vec[i] == skill_id)
        {
            return 1;
        }
    }
    // 查找称号技能是否存在
    skill_vec.clear();
    QueryTitleSkill(skill_vec);
    for (size_t i = 0; i < skill_vec.size(); ++i)
    {
        if (skill_vec[i] == skill_id)
        {
            return 1;
        }
    }*/
    return 0;
}

int32_t Player::GetWeaponID() const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->GetWeaponID();
}

bool Player::IsInvFull(int where) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->IsInvFull(where);
}

bool Player::GainItem(int item_type, int item_count)
{
    return GainItem(item_type, item_count, 0, GIM_NORMAL);
}

bool Player::GainItem(int32_t item_type, int32_t item_count, int valid_time, GainItemMode mode)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	if (!pInv->GainItem(item_type, mode, valid_time, item_count))
	{
		return false;
	}

	LevelUpSkill();
	return true;
}

bool Player::GainItem(int where, itemdata& item, GainItemMode mode)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	if (!pInv->GainItem(where, mode, item))
	{
		return false;
	}

	LevelUpSkill();
	return true;
}

int32_t Player::CountItem(int item_type) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->CountItem(item_type);
}

int32_t Player::CountEquipItem(int item_type) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->CountEquipItem(item_type);
}

bool Player::CheckItem(int item_type, int item_count) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->CheckItem(item_type, item_count);
}

bool Player::CheckItem(int item_idx, int item_type, int item_count) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->CheckItem(item_idx, item_type, item_count);
}

bool Player::HasSlot(int where, int empty_slot_count) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->HasSlot(where, empty_slot_count);
}

bool Player::TakeOutItem(int item_type, int item_count)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->TakeOutItem(item_type, item_count);
}

bool Player::TakeOutItem(int item_idx, int item_type, int item_count)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->TakeOutItem(item_idx, item_type, item_count);
}

bool Player::TakeOutAllItem(int item_type, int item_count)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->TakeOutAllItem(item_type, item_count);
}

bool Player::UpdatePetItem(int item_idx)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->UpdatePetItem(item_idx);
}

bool Player::UpdateCardItem(int item_idx)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->UpdateCardItem(item_idx);
}

bool Player::CanTrade(int where, int item_idx) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->CanTrade(where, item_idx);
}

int32_t Player::GetEquipNum(int32_t refine_level, int32_t rank) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->GetEquipNum(refine_level, rank);
}

int32_t Player::GetCardNum(int32_t level, int32_t rank) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->GetCardNum(level, rank);
}

bool Player::GetItemListDetail(int where, std::vector<itemdata>& list) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
    return pInv->GetItemListDetail(where, list);
}

void Player::QueryItemlist(int where, int& inv_cap, std::vector<itemdata>& itemlist) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	pInv->QueryItemlist(where, inv_cap, itemlist);
}

bool Player::QueryItem(int where, int item_idx, itemdata& item) const
{
    const PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	return pInv->QueryItem(where, item_idx, item);
}

void Player::PlayerGetInventoryAll()
{
	sender_->GetAllInventory();
}

void Player::PlayerGetInventory(int where)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	pInv->PlayerGetInventory(where);
}

void Player::PlayerGetInventoryDetail(int where)
{
    PlayerInventory* pInv = GET_SUBSYS_PTR(PlayerInventory, INVENTORY);
	pInv->SelfGetInventoryDetail(where);
}

void Player::GainPetExp(int32_t exp)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->GainExp(exp);
}

void Player::GainPetPower(int32_t offset)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->GainPower(offset);
}

void Player::SetPetPower(int32_t power)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->SetPower(power);
}

void Player::IncPetPowerGenSpeed(int32_t value)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->IncPowerGenSpeed(value);
}

void Player::DecPetPowerGenSpeed(int32_t value)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->DecPowerGenSpeed(value);
}

void Player::IncPetAttackCDTime(int32_t time)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->IncAttackCDTime(time);
}

void Player::DecPetAttackCDTime(int32_t time)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->DecAttackCDTime(time);
}

bool Player::GainPet(int32_t pet_item_id, int pet_item_idx)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->GainPet(pet_item_id, pet_item_idx);
}

bool Player::UnRegisterPet(int pet_item_idx)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->UnRegisterPet(pet_item_idx);
}

bool Player::RegisterPet(const playerdef::PetEntry& pet)
{
    PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->RegisterPet(pet);
}

bool Player::QueryPetInfo(int pet_item_idx, playerdef::PetEntry& pet) const
{
    const PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->QueryPetInfo(pet_item_idx, pet);
}

void Player::QueryCombatPetInfo(std::vector<playerdef::PetInfo>& list) const
{
    const PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->QueryCombatPetInfo(list);
}

void Player::QueryPetPowerInfo(int32_t& power, int32_t& power_cap, int32_t& power_gen_speed) const
{
    const PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	pPet->QueryPetPowerInfo(power, power_cap, power_gen_speed);
}

int32_t Player::GetCombatPetNum(int32_t level, int32_t blevel, int32_t rank) const
{
    const PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->GetCombatPetNum(level, blevel, rank);
}

int32_t Player::QueryPetAttackCDTime() const
{
    const PlayerPet* pPet = GET_SUBSYS_PTR(PlayerPet, PET);
	return pPet->QueryPetAttackCDTime();
}

void Player::GainMoney(int32_t money)
{
	if (money <= 0)
		return;

	int64_t __money = 0;
	if (MAX_PLAYER_MONEY - GetMoney() < money)
	{
		__money = MAX_PLAYER_MONEY;
	}
	else
	{
		__money = GetMoney() + money;
	}
	ASSERT(__money >= 0 && __money <= MAX_PLAYER_MONEY);

	base_info_.money = __money;
	PlayerGainMoney(money);
	LevelUpSkill();

    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	pachieve->GainMoney(money);
}

void Player::SpendMoney(int32_t money)
{
	if (money <= 0)
		return;

	if (money > GetMoney())
		return;

	base_info_.money -= money;
	ASSERT(base_info_.money >= 0);
	PlayerSpendMoney(money);

    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	pachieve->SpendMoney(money);
}

void Player::UseCash(int32_t offset)
{
    if (offset <= 0)
        return; 

    PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	pMall->UseCash(offset);
}

void Player::AddCash(int32_t cash_add)
{
    if (cash_add <= 0)
        return;

    if (MAX_PLAYER_CASH - GetCash() < cash_add)
    {
        LOG_ERROR << "游戏获得元宝超过上限!! cur_cash:" << GetCash()
            << " add_cash:" << cash_add;
        return;
    }

    PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	pMall->AddCash(cash_add);

    GLog::log("玩家 %ld 游戏内AddCash扩充当前可用元宝, cash_add=%d cur_cash=%d", role_id(), cash_add, GetCash());
}

void Player::SetCashTotal(int32_t cash_total)
{
    PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	pMall->SetCashTotal(cash_total);

    GLog::log("玩家 %ld 游戏内SetCashTotal扩充当前可用元宝, cash_total=%d cur_cash=%d", role_id(), cash_total, GetCash());
}

int32_t Player::GetCash() const
{
    const PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	return pMall->GetCash();
}

bool Player::CheckCash(int32_t cash) const
{
    const PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	return pMall->CheckCash(cash);
}

void Player::AddCashByGame(int32_t cash_add)
{
    if (cash_add <= 0)
    {
        LOG_ERROR << "游戏过程中获得的元宝小于等于0!!!! roleid:" << role_id();
        return;
    }

    if (MAX_PLAYER_CASH - GetCash() < cash_add)
    {
        LOG_ERROR << "游戏工程中获得元宝超过上限!! cur_cash:" << GetCash()
            << " add_cash:" << cash_add;
        return;
    }

    if (cash_add > 0)
    {
        rpc_master_->AddCashByGame(cash_add);
    }

    GLog::log("玩家 %ld 游戏内AddCashByGame扩充当前可用元宝, cash_add=%d cur_cash=%d", role_id(), cash_add, GetCash());
}

void Player::PlayerGetCash()
{
    PlayerMall* pMall = GET_SUBSYS_PTR(PlayerMall, MALL);
	pMall->PlayerGetCash();
}

void Player::PlayerGetOwnScore()
{
	sender_->GetOwnScore();
}

void Player::PlayerGetOwnMoney()
{
	sender_->GetOwnMoney();
}

void Player::PlayerGainMoney(int32_t amount)
{
	sender_->GainMoney(amount);
}

void Player::PlayerSpendMoney(int32_t cost)
{
	sender_->SpendMoney(cost);
}

void Player::PlayerGetBaseInfo()
{
	sender_->PlayerBaseInfo();
}

void Player::PlayerGetVolatileInfo()
{
	sender_->PlayerVolatileInfo();
}

void Player::PlayerGetExtendProp()
{
	sender_->PlayerExtendProp();
}

void Player::GenHP(int32_t hp)
{
    IncHP(hp);
}

void Player::GenMP(int32_t mp)
{
    IncMP(mp);
}

void Player::GenEP(int32_t ep)
{
    IncEP(ep);
}

void Player::IncHP(int32_t hp)
{
    if (state().IsDead())
        return;

	if (hp <= 0)
		return;

	int32_t tmp = GetHP() + hp;
	if (tmp > GetMaxHP())
		tmp = GetMaxHP();

	if (tmp != GetHP())
	{
		base_info_.hp = tmp;
		SetRefreshState();
	}
}

void Player::DecHP(int32_t hp)
{
	DoDamage(hp);
	if (GetHP() <= 0)
	{
		Die();
	}
}

void Player::IncMP(int32_t mp)
{
	if (mp <= 0)
		return;

	int32_t tmp = GetMP() + mp;
	if (tmp > GetMaxMP())
		tmp = GetMaxMP();

	if (tmp != GetMP())
	{
		base_info_.mp = tmp;
		SetRefreshState();
	}
}

void Player::DecMP(int32_t mp)
{
	if (mp <= 0)
		return;

	int32_t tmp = GetMP() - mp;
	if (tmp < 0) tmp = 0;
	if (tmp != GetMP())
	{
		base_info_.mp = tmp;
		SetRefreshState();
	}
}

void Player::IncEP(int32_t ep)
{
	if (ep <= 0)
		return;

	int32_t tmp = GetEP() + ep;
	if (tmp > GetMaxEP())
		tmp = GetMaxEP();

	if (tmp != GetEP())
	{
		base_info_.ep = tmp;
		SetRefreshState();
	}
}

void Player::DecEP(int32_t ep)
{
	if (ep <= 0)
		return;

	int32_t tmp = GetEP() - ep;
	if (tmp < 0) tmp = 0;
	if (tmp != GetEP())
	{
		base_info_.ep = tmp;
		SetRefreshState();
	}
}

void Player::SetHP(int32_t hp)
{
    if (state().IsDead())
        return;

	if (hp < 0)
		return;

	int32_t tmp = GetHP();
	if (hp > GetMaxHP())
		base_info_.hp = GetMaxHP();
	else
		base_info_.hp = hp;

	if (tmp != GetHP())
		SetRefreshState();
}

void Player::FullHP()
{
    if (state().IsDead())
        return;

	int32_t tmp = GetHP();
	if (tmp < GetMaxHP())
	{
		base_info_.hp = GetMaxHP();
		SetRefreshState();
	}
}

void Player::SetMP(int32_t mp)
{
	if (mp <= 0)
		return;

	int32_t tmp = GetMP();
	if (mp > GetMaxMP())
		base_info_.mp = GetMaxMP();
	else
		base_info_.mp = mp;

	if (tmp != GetMP())
		SetRefreshState();
}

void Player::SetEP(int32_t ep)
{
	if (ep <= 0)
		return;

	int32_t tmp = GetEP();
	if (ep > GetMaxEP())
		base_info_.ep = GetMaxEP();
	else
		base_info_.ep = ep;

	if (tmp != GetEP())
		SetRefreshState();
}

void Player::IncExp(int32_t exp)
{
	if (exp <= 0) return;
	int32_t cur_level = level();
	if (cur_level >= Gmatrix::GetPlayerMaxLevel(role_class()))
	{
		//已满级
		return;
	}

	int32_t __exp = base_info_.exp;
	if (INT_MAX - __exp <= exp)
	{
		base_info_.exp = 0x7FFFFFFF;
	}
	else
	{
		base_info_.exp += exp;
	}

	LevelUp();
	sender_->PlayerGainExp(exp);
}

void Player::LevelUp()
{
	bool is_level_up = false;
	int8_t cls = role_class();

	for (;;)
	{
		int32_t need_exp = Gmatrix::GetPlayerLevelUpExp(GetLevel());
		if (need_exp <= 0)
		{
			break;
		}

		if (need_exp > GetExp())
		{
			break;
		}

		is_level_up = true;
		base_info_.exp -= need_exp;
		base_info_.level += 1;

		//升级提升属性
		gmatrixdef::PlayerClassDefaultTempl cls_tpl = Gmatrix::GetPlayerClsTemplCfg(base_info_.cls);
		const dataTempl::PlayerClassTempl* pTpl = cls_tpl.templ_ptr;//获取玩家职业模板
		for (size_t i = 0; i < pTpl->properties.size(); ++ i)
		{
			SetRefreshExtProp(i);
			base_prop_[i] += pTpl->properties[i].levelup_inc;
		}

        GLog::log("玩家 %ld 升级到%d级, 消耗经验%d, 剩余经验值%d.", role_id(), GetLevel(), need_exp, GetExp());

		//技能升级
		LevelUpSkill();

        // 每15级开启一个附魔位
        if (base_info_.level % 15 == 0)
        {
            OpenEnhanceSlot(ENHANCE_OPEN_LEVEL);
        }

		if (level() >= Gmatrix::GetPlayerMaxLevel(cls))
		{
			base_info_.exp = 0;
			break;
		}
	}

	if (is_level_up)
	{
		FullHP(); //升级后满血
		sender_->PlayerLevelUp();
	}
}

#define ASSERT_PROP_INDEX(index) ASSERT(index >= 0 && index < PROP_INDEX_HIGHEST)
#define ASSERT_PROP_VALUE(value) {if (value <= 0) return;}

void Player::IncPropPoint(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	enh_point_[index] += value;
	SetRefreshExtProp(index);
}

void Player::DecPropPoint(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	enh_point_[index] -= value;
	SetRefreshExtProp(index);
}

void Player::IncPropScale(size_t index ,int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	enh_scale_[index] += value;
	SetRefreshExtProp(index);
}

void Player::DecPropScale(size_t index ,int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	enh_scale_[index] -= value;
	SetRefreshExtProp(index);
}

void Player::IncEquipPoint(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	equip_point_[index] += value;
	SetRefreshExtProp(index);
}

void Player::DecEquipPoint(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	equip_point_[index] -= value;
	SetRefreshExtProp(index);
}

void Player::IncEquipScale(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	equip_scale_[index] += value;
	SetRefreshExtProp(index);
}

void Player::DecEquipScale(size_t index, int32_t value)
{
	ASSERT_PROP_INDEX(index);
	ASSERT_PROP_VALUE(value);
	equip_scale_[index] -= value;
	SetRefreshExtProp(index);
}

#undef  ASSERT_PROP_INDEX
#undef  ASSERT_PROP_VALUE

void Player::Die()
{
	SetHP(0);
	state_.SetDead();
	visible_state_.SetDeadFlag();
	sender_->PlayerDead();
}

void Player::ForceResurrect()
{
    // 不在死亡状态
    if (!state_.IsDead())
        return;

    DoResurrect();
}

void Player::Resurrect(int choice)
{
    // 不在死亡状态
    if (!state_.IsDead())
        return;

    // 玩家的选择
    if (choice == 0)
    {
        //原地复活
        if (!TakeOutItem(FIXED_ITEM_TOOL_RESURRECT, 1))
        {
            __PRINTF("玩家%ld原地复活失败，无复活道具！！！", role_id());
            return;
        }
        
        //添加无敌Buff
	    AddSessionAndStart(new PLogonInvincibleSession(this, INVINCIBLE_BUFF_TIME));
    }
    else if (choice == 1)
    {
        //回复活点
        msg_player_region_transport param;
        param.is_resurrect    = true;
        param.source_world_id = world_id();
        param.target_world_id = world_id();
        param.target_pos.x    = pos().x;
        param.target_pos.y    = pos().y;

        // 从区域取复活点坐标
        if (area_rules_info_.resurrect_coord.map_id > 0)
        {
            //地图配置有复活点
            param.target_world_id = area_rules_info_.resurrect_coord.map_id;
            param.target_pos.x    = area_rules_info_.resurrect_coord.coord.x;
            param.target_pos.y    = area_rules_info_.resurrect_coord.coord.y;
        }
        else
        {
            LOG_WARN << "地图没有配置复活点，玩家 " << role_id() << " 传回原位置！world_id: " << world_id();
        }

        // 传送信息,复活传送
        SendMsg(GS_MSG_PLAYER_REGION_TRANSPORT, object_xid(), &param, sizeof(param));
    }
    else
    {
        LOG_ERROR << "玩家 " << role_id() << " 选择的复活类型非法, choice: " << choice;
    }

    // 执行复活
    DoResurrect();
}

// 需要判断玩家是否死亡
void Player::DoResurrect()
{
    // 先恢复正常状态才能设置血量
    state_.SetNormal();
    visible_state_.ClrDeadFlag();

    if (vip_level_ >= 2)
    {
        //满血
        FullHP();
    }
    else
    {
        //回复最大血量的10%
        int32_t hp = 0.1f * GetMaxHP();
        if (hp <= 0)
            SetHP(1);
        else
            SetHP(hp);
    }

    // 通知客户端
    sender_->PlayerResurrect();
}

void Player::RecoverHP(int32_t hp)
{
	if (vip_level_ >= 2)
	{
		FullHP();
	}
	else
	{
		//TODO
		//补充血量,具体待定
		SetHP(hp);
	}
}

int Player::DoDamage(int32_t damage)
{
    if (state().IsDead())
        return 0;

	if (damage < 0) 
    {
        damage = 1;
    }

	if (damage > GetHP()) 
    {
        damage = GetHP();
    }

	base_info_.hp -= damage;
	SetRefreshState();
	return damage;
}

int32_t Player::GetCombatSceneID() const
{
	const std::vector<int32_t>& scene_list = area_rules_info_.battle_scene_list;
	if (scene_list.size() <= 0)
		return 0;

	if (scene_list.size() == 1)
		return scene_list[0];

	size_t rand_num = mrand::Rand(0, scene_list.size()-1);
	ASSERT(rand_num >= 0 && rand_num < scene_list.size());
	return scene_list[rand_num];
}

int32_t Player::GetCombatValue() const
{
    return combat_value_;
}

int32_t Player::CalcCombatValue() const
{
    GameLuaEntity* lua_ent = GetLuaEntityRR();

	// lock
	GameLuaEntityLockGuard guard(*lua_ent);
	gmatrixdef::PlayerClassDefaultTempl cls_info = Gmatrix::GetPlayerClsTemplCfg(base_info_.cls);
    std::string tmp_func = kCombatValueFuncPrefix + itos(cls_info.cls_templ_id);
    if(!guard.lua_engine()->Call(tmp_func.c_str(), cur_prop_, PROP_INDEX_HIGHEST))
	{
		__PRINTF("LuaEngine::Call Error %s()", tmp_func.c_str());
		return 0;
	}

	int32_t combat_value = 0;
	guard.lua_engine()->PopValue(combat_value);
    combat_value_ = combat_value;
	return combat_value;
}

bool Player::CheckJoinCombat(RoleID someone)
{
    if (!CanCombat())
        return false;

	// 自己不能加入自己的战斗
	if (someone == role_id())
		return false;

	if (!IsTeammate(someone))
		return false;

	return true;
}

void Player::BroadCastEnterCombat(int32_t combat_id, int32_t world_boss_id)
{
	// 发给组队队友
	std::vector<RoleID> members_vec;
	if (!GetTeammates(members_vec))
		return;

	for (size_t i = 0; i < members_vec.size(); ++i)
	{
		world::player_base_info info;
		if (world_plane()->QueryPlayer(members_vec[i], info))
		{
			// 检查距离
			if (info.can_combat && info.pos.squared_distance(pos()) < kBCEnterCombatRangeSquare)
			{
                msg_companion_enter_combat param;
                param.combat_id     = combat_id;
                param.world_boss_id = world_boss_id;
				SendMsg(GS_MSG_COMPANION_ENTER_COMBAT, info.xid, &param, sizeof(param));
			}
		}
	}
}

int Player::BroadCastQueryTeamCombat(int64_t monster_obj_id)
{
	// 发给组队队友
	std::vector<RoleID> members_vec;
	if (!GetTeammates(members_vec))
		return 0;

	int count = 0;
	for (size_t i = 0; i < members_vec.size(); ++i)
	{
		world::player_base_info info;
		if (world_plane()->QueryPlayer(members_vec[i], info))
		{
			// 检查距离，同时队友要在战斗中
			if (!info.can_combat && info.pos.squared_distance(pos()) < kBCEnterCombatRangeSquare)
			{
				++count;
				SendMsg(GS_MSG_QUERY_TEAM_COMBAT, info.xid, monster_obj_id);
			}
		}
	}

	return count;
}

int32_t Player::GetCombatPosition()
{
	// 默认4号位
	int pos = 3;

    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	if (pbuddy->HasBuddyTeam())
	{
		// 从伙伴组队中获取位置
		int index = pbuddy->GetSelfPos();
		if (index >= 0)
		{
			pos = index;
		}
	}
	else // 和伙伴队伍互斥
	{
        // 从组队中获取位置
        int index = GetPosInTeam();
        if (index >= 0)
        {
            pos = index;
        }
	}

	return pos;
}

int32_t Player::QueryTeammateCountEnterCombat()
{
	int rst = 0;
	std::vector<RoleID> members_vec;
	if (!GetTeammates(members_vec))
		return rst;

	for (size_t i = 0; i < members_vec.size(); ++i)
	{
		world::player_base_info info;
		if (world_plane()->QueryPlayer(members_vec[i], info))
		{
			// 检查距离
			if (info.pos.squared_distance(pos()) < kBCEnterCombatRangeSquare)
			{
				// 是否可以战斗
				if (info.can_combat)
					++ rst;
			}
		}
	}
	return rst;
}

bool Player::TerminateCombat()
{
    PlayerCombat* pCombat = GET_SUBSYS_PTR(PlayerCombat, COMBAT);
	return pCombat->TerminateCombat();
}

bool Player::StartPVPCombat(int32_t combat_scene_id, playerdef::CombatPVPType type, playerdef::StartPVPResult& result)
{
    PlayerCombat* pCombat = GET_SUBSYS_PTR(PlayerCombat, COMBAT);
	return pCombat->StartPVPCombat(combat_scene_id, type, result);
}

bool Player::FirstJoinPVPCombat(int32_t combat_id)
{
    PlayerCombat* pCombat = GET_SUBSYS_PTR(PlayerCombat, COMBAT);
	return pCombat->FirstJoinPVPCombat(combat_id);
}

void Player::HandleCombatPVPEnd(const playerdef::PVPEndInfo& info, playerdef::PVPEndExtraData& data)
{
    switch (info.pvp_type)
    {
        case PVP_TYPE_DUEL:
            {
                PlayerDuel* pDuel = GET_SUBSYS_PTR(PlayerDuel, DUEL);
                pDuel->DuelCombatEnd(info, data);
            }
            break;

        default:
            LOG_WARN << "玩家pvp战斗HandleCombatPVPEnd的pvp_type未知类型" << info.pvp_type;
            return;
    }
}

void Player::AddFilter(Filter* filter)
{
    PlayerBuff* pBuff = GET_SUBSYS_PTR(PlayerBuff, BUFF);
	pBuff->AddFilter(filter);
}

void Player::DelFilter(int32_t filter_type)
{
    PlayerBuff* pBuff = GET_SUBSYS_PTR(PlayerBuff, BUFF);
	pBuff->DelFilter(filter_type);
}

void Player::DelFilterByID(int32_t effectid)
{
    PlayerBuff* pBuff = GET_SUBSYS_PTR(PlayerBuff, BUFF);
	pBuff->DelFilterByID(effectid);
}

void Player::DelSpecFilter(int32_t filter_mask)
{
    PlayerBuff* pBuff = GET_SUBSYS_PTR(PlayerBuff, BUFF);
	return pBuff->DelSpecFilter(filter_mask);
}

void Player::ModifyFilterByID(int32_t effectid, bool is_add, bool reject_save)
{
    if (is_add)
    {
        int8_t type = skill::GetWorldBuffType(effectid);
        Filter* pfilter = NULL;
        switch (type)
        {
            case skill::WORLD_BUFF_INVINCIBLE:
                {
                    pfilter = new InvincibleFilter(this, effectid);
                }
                break;

            case skill::WORLD_BUFF_TRANSFORM:
                {
                    pfilter = new TransformFilter(this, effectid);
                }
                break;

            case skill::WORLD_BUFF_PROP:
                {
                    pfilter = new PropertyFilter(this, effectid);
                }
                break;

            default:
                LOG_ERROR << "Error task add world-buff effectid:" << effectid;
                return;
        }

        // 是否拒绝存盘
        if (reject_save)
        {
            pfilter->SetRejectSave();
        }

        AddFilter(pfilter);
    }
    else
    {
        DelFilterByID(effectid);
    }
}

bool Player::IsFilterExistByID(int32_t effectid)
{
    PlayerBuff* pBuff = GET_SUBSYS_PTR(PlayerBuff, BUFF);
	return pBuff->IsFilterExistByID(effectid);
}

void Player::ProcessTeamMsg(const MSG& msg)
{
	PlayerSubSystem* pSubSys = QuerySubSys(PlayerSubSystem::SUB_SYS_TYPE_TEAM);
	if (pSubSys == NULL)
	{
		if (msg.message == GS_MSG_TEAM_INFO)
		{
			if (!subsys_if_->AddSubSys(PlayerSubSystem::SUB_SYS_TYPE_TEAM))
			{
				LOG_ERROR << "ProcessTeamMsg() SUB_SYS_TYPE_TEAM 创建失败！";
				return;
			}
		}
		else
		{
			LOG_WARN << "team子系统没有创建，收到teamMsg， message=" << msg.message;
			sender_->QueryTeamInfo();
			return;
		}
	}

	// 同步执行，不进消息队列
	SyncDispatchMsg(msg);
}

int Player::GetTeamId() const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->GetTeamId();
	}
	
	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return -1;

	return pteam->GetTeamId();
}

bool Player::IsInTeam(int32_t teamid) const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->IsInTeam(teamid);
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return false;

	return pteam->IsInTeam(teamid);
}

bool Player::IsInTeam() const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->IsInTeam();
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return false;

	return pteam->IsInTeam();
}

bool Player::IsTeamLeader() const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->IsTeamLeader();
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return false;

	return pteam->IsTeamLeader();
}

bool Player::IsTeammate(RoleID roleid) const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->IsTeammate(roleid);
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return false;

	return pteam->IsTeammate(roleid);
}

bool Player::GetTeammates(std::vector<RoleID>& members_vec)
{
	members_vec.clear();

	// 副本组队优先
	PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->GetTeammates(members_vec);
	}
	
	// 普通组队
	PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return false;

	return pteam->GetTeammates(members_vec);
}

RoleID Player::GetLeaderId() const
{
	// 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->GetLeaderId();
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return 0;

	return pteam->GetLeaderId();
}

int Player::GetPosInTeam()
{
	// 副本组队优先
	PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->GetSelfPos();
	}

	// 普通组队
	PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return -1;

	return pteam->GetSelfPos();
}

int Player::GetTeammateCount() const
{
    // 副本组队优先
	const PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team != NULL && world_plane()->HasMapTeam())
	{
		return pmap_team->GetTeammateCount();
	}

	// 普通组队
	const PlayerTeam* pteam = GetTeamPtr();
	if (pteam == NULL)
		return 0;

	return pteam->GetTeammateCount();
}

void Player::JoinTeamTransmit(RoleID other_roleid)
{
    // 在非普通地图不能发起普通组队
    if (!IS_NORMAL_MAP(world_id()))
    {
        sender_->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	if (!pbuddy->CanJoinPlayerTeam())
	{
		sender_->ErrorMessage(G2C::ERR_HAS_TEAM_WITH_NPC);
	}
	else
	{
		sender_->JoinTeam(other_roleid);
	}
}

void Player::JoinTeamResTransmit(bool invite, bool accept, RoleID requester)
{
    // 在非普通地图不能回应普通组队
    if (!IS_NORMAL_MAP(world_id()))
    {
        sender_->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
        return;
    }

    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	if (!pbuddy->CanJoinPlayerTeam())
	{
		sender_->ErrorMessage(G2C::ERR_HAS_TEAM_WITH_NPC);
	}
	else
	{
		sender_->JoinTeamRes(invite, accept, requester);
	}
}

bool Player::CanJoinPlayerTeam() 
{
    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	return pbuddy->CanJoinPlayerTeam();
}

void Player::LeaveTeamBySelf()
{
	PlayerTeam* pteam = GetTeamPtr();
	if (pteam != NULL)
	{
		pteam->LeaveTeamBySelf();
	}
}

void Player::SendJoinTeamReq(RoleID requester, const std::string& first_name, const std::string& mid_name, const std::string& last_name, int32_t invite)
{
	sender_->JoinTeamReq(requester, first_name, mid_name, last_name, invite);
}

void Player::TeamLeaderConvene(RoleID leader, int32_t world_id, const A2DVECTOR& pos)
{
    PlayerTeam* pteam = GetTeamPtr();
    if (pteam == NULL)
        return;

    pteam->TeamLeaderConvene(leader, world_id, pos);
}

PlayerTeam* Player::GetTeamPtr()
{
	PlayerSubSystem* pSubSys = QuerySubSys(PlayerSubSystem::SUB_SYS_TYPE_TEAM);
	if (pSubSys == NULL)
		return NULL;

	PlayerTeam* pteam = dynamic_cast<PlayerTeam*>(pSubSys);
	return pteam;
}

const PlayerTeam* Player::GetTeamPtr() const
{
	const PlayerSubSystem* pSubSys = QuerySubSys(PlayerSubSystem::SUB_SYS_TYPE_TEAM);
	if (pSubSys == NULL)
		return NULL;

	const PlayerTeam* pteam = dynamic_cast<const PlayerTeam*>(pSubSys);
	return pteam;
}

PlayerMapTeam* Player::GetMapTeamPtr()
{
	PlayerSubSystem* pSubSys = QuerySubSys(PlayerSubSystem::SUB_SYS_TYPE_MAP_TEAM);
	if (pSubSys == NULL)
		return NULL;

	PlayerMapTeam* pmap_team = dynamic_cast<PlayerMapTeam*>(pSubSys);
	return pmap_team;
}

const PlayerMapTeam* Player::GetMapTeamPtr() const
{
	const PlayerSubSystem* pSubSys = QuerySubSys(PlayerSubSystem::SUB_SYS_TYPE_MAP_TEAM);
	if (pSubSys == NULL)
		return NULL;

	const PlayerMapTeam* pmap_team = dynamic_cast<const PlayerMapTeam*>(pSubSys);
	return pmap_team;
}

void Player::SyncMapTeamInfo(const MSG& msg)
{
	PlayerMapTeam* pmap_team = GetMapTeamPtr();
	if (pmap_team == NULL)
	{
		if (msg.message == GS_MSG_MAP_TEAM_INFO)
		{
			if (!subsys_if_->AddSubSys(PlayerSubSystem::SUB_SYS_TYPE_MAP_TEAM))
			{
				LOG_ERROR << "SyncInsTeamInfo() SUB_SYS_TYPE_INS_TEAM 创建失败！";
				return;
			}
		}
		else
		{
			LOG_WARN << "ins-team子系统没有创建，message=" << msg.message;
			return;
		}
	}

	// 同步执行，不进消息队列
	SyncDispatchMsg(msg);
}

void Player::ObtainBuddy(int32_t buddy_tid)
{
    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	pbuddy->ObtainBuddy(buddy_tid);
}

void Player::TakeoutBuddy(int32_t buddy_tid)
{
    PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	pbuddy->TakeoutBuddy(buddy_tid);
}

bool Player::GetBuddyMembers(std::vector<playerdef::BuddyMemberInfo>& buddy_vec) const
{
    const PlayerBuddy* pbuddy = GET_SUBSYS_PTR(PlayerBuddy, BUDDY);
	return pbuddy->GetBuddyMembers(buddy_vec);
}

void Player::ResetAreaRulesInfo()
{
	area_rules_info_.Reset();
	rule_areas_vec_.clear();
}

void Player::RefreshAreaRulesInfo(int32_t elem_id, bool is_added)
{
	bool need_refresh = false;

	// 普通的规则区域
	if (elem_id > 0)
	{
		RuleAreaVec::iterator it_find = std::find(rule_areas_vec_.begin(), rule_areas_vec_.end(), elem_id);
		if (is_added)
		{
			// 先加入area vector
			//ASSERT(it_find == rule_areas_vec_.end()); // 不会同时进入同一个区域两次
			if (it_find == rule_areas_vec_.end())
			{
				rule_areas_vec_.push_back(elem_id);
				need_refresh = true;
			}
		}
		else
		{
			// 从vector中删除
			//ASSERT(it_find != rule_areas_vec_.end()); // 离开的区域肯定是先前已经进入的
			if (it_find != rule_areas_vec_.end())
			{
				rule_areas_vec_.erase(it_find);
				need_refresh = true;
			}
		}
	}
	else if (elem_id == 0) // 默认规则区域刷一下
	{
		need_refresh = true;
	}

	// 在刷新自己的area信息
	if (need_refresh)
	{
		area_rules_info_.Reset();
		world_plane()->QueryAreaRules(rule_areas_vec_, area_rules_info_);
	}
}

void Player::SayHelloToNpc(const XID& target)
{
    PlayerService* pservice = GET_SUBSYS_PTR(PlayerService, SERVICE);
	pservice->SayHelloToNpc(target);
}

bool Player::RegionTransport(int32_t world_id, const A2DVECTOR& pos)
{
	// 做状态检查
	if (!CanSwitch())
	{
		sender_->ErrorMessage(G2C::ERR_CUR_STATE_CANNOT_TRANSPORT);
		return false;
	}
		
	///
	/// 地图跳转，包含不再本gs的地图
	///
	if (!LongJump(world_id, pos))
	{
		sender_->ErrorMessage(G2C::ERR_LONGJUMP_FAILURE);
		return false;
	}

	return true;
}

void Player::NotifySelfPos()
{
	sender_->NotifySelfPos();
}

void Player::CheckTimeToSaveData()
{
	if (--write_timer_ <= 0)
	{
		if (state_.CanSave())
		{
			PlayerDataSyncToMaster(SDOP_HEARTBEAT);
			// 正常存盘时间
			// 2014-09-12: 从500改成200秒
			write_timer_ = mrand::Rand(200, 213);
		}
		else
		{
			// 当前状态不能存盘则调快下一个存盘时间点
			write_timer_ = mrand::Rand(30, 94);
		}
	}
}

void Player::ResetWriteTime()
{
	write_timer_ = 0;
}

bool Player::LongJump(int32_t target_world_id, const A2DVECTOR& pos)
{
	// 做状态检查
	if (!CanSwitch())
	{
		return false;
	}
	
	///
	/// 本地图跳转
	///
	if (world_id() == target_world_id)
	{
		return LocalJump(pos);
	}

	///
	/// 进入特殊地图
	///
	if (!IS_NORMAL_MAP(target_world_id))
	{
		if (CheckSpecMapLongJumpRule(target_world_id, world_id()))
		{
			// 发送数据到master，然后做下线处理
			PlayerChangeMap(target_world_id, pos);
			return true;
		}
		else
		{
			__PRINTF("不满足特殊地图的LongJump规则，不能直接跳转！");
			return false;
		}
	}

	///
	/// @brief 从特殊地图离开 
    ///  非普通地图包括 -- 副本、竞技场、战场
	/// （1）非普通地图做ChangeMap相当于一次下线再上线
	/// （2）非普通地图只能切换到普通地图
	///
	if (!IS_NORMAL_MAP(world_id()))
	{
		if (CheckUnnormalMapLeaveRule(target_world_id, world_id()))
		{
			PlayerChangeMap(target_world_id, pos);
			return true;
		}
		else
		{
			__PRINTF("不满足从非普通地图进行ChangeMap的规则，不能直接跳转！");
			return false;
		}
	}

    // 现在非本地图传送都要过master
    PlayerChangeMap(target_world_id, pos);
    return true;

    /*
	///
	/// 目标地图不在本gs
	///
	if (!Gmatrix::IsWorldInCharge(target_world_id))
	{
		// 发送数据到master，然后做下线处理
		PlayerChangeMap(target_world_id, pos);
		return true;
	}
	
	///
	/// 跳转到本gs的其他地图
	///
	if (!world_plane()->PlaneSwitch(this, target_world_id, pos))
	{
		return false;
	}

	return true;
    */
}

bool Player::LocalJump(const A2DVECTOR& pos)
{
	// 本地图跳跃，调用前需要做player的状态检查
	// walkable
	if (!world_plane()->IsWalkablePos(pos))
		return false;
	
	// check pos
	if (!world_plane()->PosInWorld(pos))
		return false;

	// step to new pos
	if (!move_ctrl_->StepTo(pos))
		return false;

	// notify client player pos
	NotifySelfPos();
	// new position broadcast to all else players
	sender_->BroadcastPlayerPullBack(pos);
	return true;
}

void Player::PlayerChangeMap(int32_t target_world_id, const A2DVECTOR& pos)
{
	// player logout会调用LeaveWorld，所以必须在GetPlayerData之前
	PlayerLogout(LT_CHANGE_MAP);

	// get player data
	shared::net::Buffer tmpbuf;	
	if (!GetPlayerDataOctetStr(&tmpbuf))
	{
		LOG_ERROR << "get playerdata error, in Player::PlayerChangeMap()";
		return;
	}
	
	// 发送数据到master，然后做下线处理
	if (!sender_->PlayerChangeMap(target_world_id, pos, tmpbuf))
    {
        // sender发送可能失败，这时直接把玩家踢下线
		sender_->PlayerFatalError();
        LOG_ERROR << "玩家传送数据有误，直接踢下线！role_id: " << role_id();
        return;
    }
}

void Player::ChangeMapError(bool is_change_gs_err)
{
    PlayerTransfer* ptransfer = GET_SUBSYS_PTR(PlayerTransfer, TRANSFER);
	ptransfer->ChangeMapError(is_change_gs_err);
}

bool Player::IsChangeMapFillMapTeam(int32_t target_world_id) const
{
    if (IS_BG_MAP(target_world_id))
    {
        const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
        return pbattle->IsChangeMapFillMapTeam(target_world_id);
    }
    return false;
}

int32_t Player::GetClsTemplId() const
{
	gmatrixdef::PlayerClassDefaultTempl cls_tpl = Gmatrix::GetPlayerClsTemplCfg(base_info_.cls);
	const dataTempl::PlayerClassTempl* pTpl = cls_tpl.templ_ptr;//获取玩家职业模板
	return pTpl->templ_id;
}

task::ActiveTask* Player::GetActiveTask()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->GetActiveTask();
}

task::FinishTask* Player::GetFinishTask()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->GetFinishTask();
}

task::FinishTimeTask* Player::GetFinishTimeTask()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->GetFinishTimeTask();
}

task::TaskDiary* Player::GetTaskDiary()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->GetTaskDiary();
}

task::TaskStorage* Player::GetTaskStorage()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->GetTaskStorage();
}

bool Player::HasActiveTask(int32_t task_id)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->HasActiveTask(task_id);
}

bool Player::HasFinishTask(int32_t task_id)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->HasFinishTask(task_id);
}

int Player::CanDeliverTask(int32_t taskid) const
{
    const PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->CanDeliverTask(taskid);
}

bool Player::TaskMiniGameEnd(int32_t game_tid)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->MiniGameEnd(game_tid);
}

void Player::StartRecvWorldTask()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    ptask->StartRecvWorldTask();
}

void Player::SubscribeGlobalCounter(int32_t index, bool is_subscribe)
{
    const dataTempl::GlobalCounter* ptempl = s_pDataTempl->QueryDataTempl<dataTempl::GlobalCounter>(index);
	if (ptempl == NULL)
	{
		LOG_ERROR << "Player::SubscribeGlobalCounter() 没有这个全局计数器! id:" << index;
		return;
	}

    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    ptask->SubscribeGlobalCounter(index, is_subscribe);
}

bool Player::GetGlobalCounter(int32_t index, int32_t& value) const
{
    const dataTempl::GlobalCounter* ptempl = s_pDataTempl->QueryDataTempl<dataTempl::GlobalCounter>(index);
	if (ptempl == NULL)
	{
		LOG_ERROR << "Player::GetGlobalCounter() 没有这个全局计数器! id:" << index;
		return false;
	}

    const PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    return ptask->GetGlobalCounter(index, value);
}

void Player::ModifyGlobalCounter(int32_t index, int8_t op, int32_t delta)
{
    const dataTempl::GlobalCounter* ptempl = s_pDataTempl->QueryDataTempl<dataTempl::GlobalCounter>(index);
	if (ptempl == NULL)
	{
		LOG_ERROR << "Player::ModifyGlobalCounter() 没有这个全局计数器! id:" << index << " op:" << op;
		return;
	}

    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    return ptask->ModifyGlobalCounter(index, op, delta);
}

void Player::SubscribeMapCounter(int32_t world_id, int32_t index, bool is_subscribe)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    ptask->SubscribeMapCounter(world_id, index, is_subscribe);
}

void Player::ClearMapCounterList()
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    ptask->ClearMapCounterList();
}

void Player::UpdatePropertyAndSpeed()
{
    float old_speed = cur_speed();
	PropertyPolicy::UpdatePlayerProp(this);
    float new_speed = cur_speed();
    float diff      = new_speed - old_speed;
    if (diff > STD_EPSINON || diff < -STD_EPSINON)
    {
	    sender()->NotifyPlayerMoveProp(new_speed);
    }
}

bool Player::CombatFail(int32_t task_id)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    return ptask->CombatFail(task_id);
}

bool Player::IsTaskFinish(int32_t task_id)
{
    const PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
    return ptask->IsTaskFinish(task_id);
}

void Player::KillMonster(int32_t monster_tid, int32_t monster_num, std::vector<playerdef::ItemEntry>& items)
{
	// task
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	ptask->KillMonster(monster_tid, monster_num, items);

    // achieve
    if (s_pAchieveCfg->IsAchieveMonster(monster_tid))
    {
        PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
        pachieve->KillMonster(monster_tid, monster_num);
    }

	// imp
	pimp_->OnKillMonster(monster_tid, monster_num);

    // change
    HandleKillMonster(monster_tid, monster_num);
}

void Player::KillMonsterLastTime(int32_t monster_tid, int32_t monster_num, std::vector<playerdef::ItemEntry>& items)
{
	// task
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	ptask->KillMonster(monster_tid, monster_num, items);
}

void Player::HandleKillMonster(int32_t monster_tid, int32_t monster_num)
{
    const MonsterTempl* pmonster = s_pDataTempl->QueryDataTempl<MonsterTempl>(monster_tid);
    if (pmonster == NULL)
    {
        LOG_ERROR << "没有找到怪物模板！tid:" << monster_tid;
        return;
    }

    // 增加喵类视觉
    if (pmonster->cat_vision_exp > 0)
    {
        CatVisionGainExp(pmonster->cat_vision_exp * monster_num);
    }

    // 修改计数器
    int32_t delta = 0, index = 0, optype = 0;
    if (pmonster->global_counter.tid > 0)
    {
        index  = pmonster->global_counter.tid;
        optype = pmonster->global_counter.optype;
        delta  = pmonster->global_counter.value * monster_num;
        MonsterChangeCounter(index, optype, delta, false);
    }
    if (pmonster->player_counter.tid > 0)
    {
        index  = pmonster->player_counter.tid;
        optype = pmonster->player_counter.optype;
        delta  = pmonster->player_counter.value * monster_num;
        MonsterChangeCounter(index, optype, delta, true);
    }
}

void Player::MonsterChangeCounter(int32_t index, int32_t type, int32_t delta, bool is_player)
{
    int optype = 0;
    switch (type)
    {
        case MonsterTempl::CounterInfo::OT_DEC:
            optype = task::COUNTER_OP_DEC;
            break;

        case MonsterTempl::CounterInfo::OT_INC:
            optype = task::COUNTER_OP_INC;
            break;

        case MonsterTempl::CounterInfo::OT_ASS:
            optype = task::COUNTER_OP_ASSIGN;
            break;

        default:
            LOG_ERROR << "MonsterChangeCounter() error type:" << type;
            return;
    }

    if (is_player)
    {
        ModifyPCounter(index, optype, delta);
    }
    else // global counter
    {
        ModifyGlobalCounter(index, optype, delta);
    }
}

int Player::DeliverTask(int32_t taskid)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	return ptask->DeliverTask(taskid);
}

void Player::DeliverAward(int32_t task_id, int32_t choice)
{
    PlayerTask* ptask = GET_SUBSYS_PTR(PlayerTask, TASK);
	ptask->DeliverAward(task_id, choice);
}

bool Player::CheckSpecMapLongJumpRule(MapID target_world_id, MapID src_world_id) const
{
    if (IS_NORMAL_MAP(target_world_id))
        return false;

    // 特殊地图内只能跳转到本地图的其他位置，不能直接跳到别的特殊
    if (!IS_NORMAL_MAP(src_world_id) && target_world_id != src_world_id)
        return false;

	return true;
}

bool Player::CheckUnnormalMapLeaveRule(MapID target_world_id, MapID src_world_id) const
{
	// 非常规地图只能ChangeMap到常规地图
	if (IS_NORMAL_MAP(src_world_id) || !IS_NORMAL_MAP(target_world_id))
		return false;

	return true;
}

void Player::OpenCatVision()
{
	if (cat_vision_state_ || GetEP() <= 0)
	{
		sender_->CatVisionOpen_Re(0);
		return;
	}

	cat_vision_state_ = true;
	sender_->CatVisionOpen_Re(1);
}

void Player::CloseCatVision()
{
	if (!cat_vision_state_)
	{
		sender_->CatVisionClose_Re(0);
		return;
	}

	cat_vision_state_ = false;
	sender_->CatVisionClose_Re(1);
}

void Player::DeActivateCatVision()
{
	if (!cat_vision_state_)
		return;

	cat_vision_state_ = false;
	sender_->CatVisionClose();
}

void Player::CatVisionGainExp(int32_t exp)
{
	if (exp <= 0)
		return;

	int32_t cur_level = GetLevel();
	if (cur_level >= Gmatrix::GetCatVisionMaxLevel())
	{
		//已满级
		return;
	}

	base_info_.cat_exp += exp;
	CatVisionLevelUp();
	sender_->CatVisionGainExp(exp);
}

void Player::CatVisionLevelUp()
{
	bool is_level_up = false;

	for (;;)
	{
		int16_t level  = GetCatVisionLevel();
		int32_t need_exp = Gmatrix::GetCatVisionLevelUpExp(level);
		if (need_exp <= 0)
		{
			break;
		}

		if (need_exp > GetCatVisionExp())
		{
			break;
		}

		is_level_up = true;
		base_info_.cat_level += 1;
		sender_->CatVisionLevelUp();

        GLog::log("玩家 %ld 瞄类视觉升级到 %d 级, 瞄类视觉经验值 %d.", role_id(), GetCatVisionLevel(), GetCatVisionExp());
	}
}

void Player::GainCatVisionPower(int32_t power)
{
	GenEP(power);
}

void Player::GetElsePlayerExtProp(int64_t target)
{
	msg_query_extprop param;
	param.requester = role_id();
	param.link_id = link_id();
	param.sid_in_link = sid_in_link();
	SendMsg(GS_MSG_QUERY_EXTPROP, XID(target, XID::TYPE_PLAYER), &param, sizeof(param));
}

void Player::GetElsePlayerEquipCRC(int64_t target)
{
	msg_query_extprop param;
	param.requester = role_id();
	param.link_id = link_id();
	param.sid_in_link = sid_in_link();
	SendMsg(GS_MSG_QUERY_EQUIPCRC, XID(target, XID::TYPE_PLAYER), &param, sizeof(param));
}

void Player::GetElsePlayerEquipment(int64_t target)
{
	msg_query_extprop param;
	param.requester = role_id();
	param.link_id = link_id();
	param.sid_in_link = sid_in_link();
	SendMsg(GS_MSG_QUERY_EQUIPMENT, XID(target, XID::TYPE_PLAYER), &param, sizeof(param));
}

void Player::QueryExtProp(std::vector<int32_t>& props) const
{
	for (int i = 0; i < PROP_INDEX_HIGHEST; ++ i)
		props.push_back(cur_prop_[i]);
}

void Player::IncSpeed(int32_t value)
{
    IncPropPoint(PROP_INDEX_MOVE_SPEED, value);
    //UpdatePropertyAndSpeed();
}

void Player::DecSpeed(int32_t value)
{
    DecPropPoint(PROP_INDEX_MOVE_SPEED, value);
    //UpdatePropertyAndSpeed();
}

bool Player::ObjectCanInteract(int32_t tid)
{
    PlayerBWList* pbw_list = GET_SUBSYS_PTR(PlayerBWList, BW_LIST);
	if (pbw_list->IsInWhiteList(tid))
	{
		return true;
	}

	if (pbw_list->IsInBlackList(tid))
	{
		return false;
	}

	return true;
}

void Player::ModifyNPCBWList(int32_t templ_id, bool is_black, bool is_add)
{
    PlayerBWList* pbw_list = GET_SUBSYS_PTR(PlayerBWList, BW_LIST);
    pbw_list->ModifyNPCBWList(templ_id, is_black, is_add);
}

void Player::ClearNPCBWList()
{
    PlayerBWList* pbw_list = GET_SUBSYS_PTR(PlayerBWList, BW_LIST);
    pbw_list->ClearNPCBWList();
}

bool Player::GetRuntimeInsInfo(MapID target_world_id, common::protocol::global::InstanceInfo& insinfo) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->GetRuntimeInsInfo(target_world_id, insinfo);
}

void Player::SetInstanceInfo(const world::instance_info& info)
{
    PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->SetInstanceInfo(info);
}

void Player::ResetInstanceInfo()
{
    PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->ResetInstanceInfo();
}

bool Player::CheckInstanceCond(const playerdef::InsInfoCond& info) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->CheckInstanceCond(info);
}

bool Player::ConsumeInstanceCond(int32_t ins_tid, bool is_create_ins)
{
    PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->ConsumeInstanceCond(ins_tid, is_create_ins);
}

bool Player::GetInstancePos(A2DVECTOR& pos) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->GetInstancePos(pos);
}

bool Player::QueryInsEntrancePos(A2DVECTOR& pos) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	return pinstance->QueryInsEntrancePos(pos);
}

void Player::TaskTransferSoloIns(int32_t ins_tid) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	pinstance->TaskTransferSoloIns(ins_tid);
}

void Player::GeventTransferSoloIns(int32_t ins_tid) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	pinstance->GeventTransferSoloIns(ins_tid);
}

void Player::CalcInstanceRecord(int32_t ins_tid, int32_t clear_time, bool is_svr_record, int32_t last_record)
{
    PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	pinstance->CalcInstanceRecord(ins_tid, clear_time, is_svr_record, last_record);
}

void Player::FinishInstance(int32_t ins_id, bool succ) const
{
    const PlayerInstance* pinstance = GET_SUBSYS_PTR(PlayerInstance, INSTANCE);
	pinstance->FinishInstance(ins_id, succ);
}

bool Player::GetRuntimeBGInfo(MapID target_world_id, common::protocol::global::BattleGroundInfo& bginfo) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    return pbattle->GetRuntimeBGInfo(target_world_id, bginfo);
}

bool Player::CheckBattleGroundCond(const playerdef::BGInfoCond& info) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    return pbattle->CheckBattleGroundCond(info);
}

void Player::ResetBattleGroundInfo()
{
    PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    pbattle->ResetBattleGroundInfo();
}

void Player::SetBattleGroundInfo(const world::battleground_info& info)
{
    PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    pbattle->SetBattleGroundInfo(info);
}

bool Player::GetBattleGroundPos(A2DVECTOR& pos) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    return pbattle->GetBattleGroundPos(pos);
}

bool Player::QueryBGEntrancePos(A2DVECTOR& pos) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    return pbattle->QueryBGEntrancePos(pos);
}

void Player::EnterPveBattleGround(int32_t bg_tid) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    pbattle->EnterPveBattleGround(bg_tid);
}

void Player::QuitBattleGround(int32_t bg_tid) const
{
    const PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    pbattle->QuitBattleGround(bg_tid);
}

bool Player::ConsumeBattleGroundCond(int32_t bg_tid)
{
    PlayerBattleGround* pbattle = GET_SUBSYS_PTR(PlayerBattleGround, BATTLEGROUND);
    return pbattle->ConsumeBattleGroundCond(bg_tid);
}

void Player::ChangeName(playerdef::NameType name_type, const std::string& name)
{
	rpc_master_->ChangeName(name_type, name);
}

void Player::TaskChangeName(int32_t taskid, playerdef::NameType name_type, const std::string& name)
{
	rpc_master_->TaskChangeName(taskid, name_type, name);
}

///
/// 改名成功，通知客户端还需要调用别的接口,见master_rpc
///
void Player::ChangeNameSuccess(int8_t name_type, const std::string& name, int32_t task_id)
{
	std::string old_name;
	if (name_type == playerdef::NT_FIRST_NAME)
	{
		old_name              = role_info_.first_name;
		role_info_.first_name = name;
	}
	else if (name_type == playerdef::NT_MIDDLE_NAME)
	{
		old_name            = role_info_.mid_name;
		role_info_.mid_name = name;
	}
	else if (name_type == playerdef::NT_LAST_NAME)
	{
		old_name             = role_info_.last_name;
		role_info_.last_name = name;
	}
	else
	{
		ASSERT(false);
	}

	if (task_id > 0)
	{
		PlayerTaskIf task_if(this);
		task::ChangeNameSucc(&task_if, task_id, name_type);
		GLog::log("玩家 %ld 完成改名任务%d，修改name_type:%d，old_name:%s -> new_name:%s", 
				role_id(), task_id, name_type, old_name.c_str(), name.c_str());
	}
	else
	{
		GLog::log("玩家 %ld 修改名字，修改name_type:%d，old_name:%s -> new_name:%s", 
				role_id(), name_type, old_name.c_str(), name.c_str());
	}

    // 同步给地图
    ModifyPlayerExtraInfo();
}

void Player::ModifyPlayerExtraInfo()
{
    world_plane()->SetPlayerExtraInfo(this);
}

void Player::CtrlMapElement(int32_t elem_id, bool open)
{
	if (open)
	{
		SendPlaneMsg(GS_PLANE_MSG_ENABLE_MAP_ELEM, elem_id);
	}
	else
	{
		SendPlaneMsg(GS_PLANE_MSG_DISABLE_MAP_ELEM, elem_id);
	}
}

void Player::SetSpecSavePos(int32_t worldid, const A2DVECTOR& pos)
{
	ASSERT(IS_NORMAL_MAP(worldid));
	spec_save_pos_.world_id = worldid;
	spec_save_pos_.pos      = pos;
}

bool Player::GetSpecSavePos(int32_t& worldid, A2DVECTOR& pos) const
{
	if (spec_save_pos_.has_spec_pos())
	{
		worldid = spec_save_pos_.world_id;
		pos     = spec_save_pos_.pos;
		return true;
	}
	return false;
}

bool Player::GetDramaSavePos(int32_t& worldid, A2DVECTOR& pos) const
{
	if (drama_save_pos_.has_drama_pos() &&
		drama_save_pos_.world_id == world_id() &&
		IS_NORMAL_MAP(drama_save_pos_.world_id))
	{
		worldid = drama_save_pos_.world_id;
		pos     = drama_save_pos_.pos;
		return true;
	}
	return false;
}

void Player::PullBackToValidPos(int seq_from_cli)
{
	move_ctrl_->PullBackToValidPos(seq_from_cli, pos());
}

void Player::HandleWorldClose(const MSG& msg)
{
	pimp_->OnWorldClosing(msg);
}

void Player::CheckPassivityFlag()
{
	if (passivity_flag_.is_passive())
	{
		if (--passivity_flag_.timeout < 0)
		{
			passivity_flag_.clear();
		}
	}
}

void Player::PlayerAnnounceNewMail()
{
    PlayerMail* pMail = GET_SUBSYS_PTR(PlayerMail, MAIL);
	pMail->AnnounceNewMail();
}

void Player::SendSysMail(shared::net::ByteBuffer& data)
{
    PlayerMail* pMail = GET_SUBSYS_PTR(PlayerMail, MAIL);
	pMail->SendSysMail(data);
}

void Player::SendSysMail(int32_t gold, int32_t item_id)
{
    PlayerMail* pMail = GET_SUBSYS_PTR(PlayerMail, MAIL);
	pMail->SendSysMail(gold, item_id);
}

void Player::SendSysMail(int32_t master_id, RoleID receiver, shared::net::ByteBuffer& data)
{
    PlayerMail::SendSysMail(master_id, receiver, data);
}

void Player::SendSysMail(int32_t master_id, RoleID receiver, const playerdef::SysMail& sysmail)
{
    PlayerMail::SendSysMail(master_id, receiver, sysmail);
}

void Player::SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content)
{
    PlayerChat* pChat = GET_SUBSYS_PTR(PlayerChat, CHAT);
	pChat->SendSysChat(channel, sender_name, content);
}

void Player::UpdateTaskStorage(int32_t sid)
{
	PlayerTaskIf task_if(this);
	task::UpdateTaskStorage(&task_if, sid);
}

void Player::GainScore(int32_t score)
{
    if (MAX_PLAYER_SCORE - spec_money_.score_total < score)
    {
        score = MAX_PLAYER_SCORE - spec_money_.score_total;
    }
    if (score > 0)
    {
	    spec_money_.score_total += score;
	    sender_->GainScore(score);
    }
}

void Player::SpendScore(int32_t score)
{
	int32_t cur_score = spec_money_.score_total - spec_money_.score_used;
	if (score <= 0 || score > cur_score)
	{
		return;
	}
	spec_money_.score_used += score;
	sender_->SpendScore(score);
}

bool Player::HasTalent(int32_t talent_group_id, int32_t talent_id) const
{
    const PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	return pTalent->IsTalentExist(talent_group_id, talent_id);
}

bool Player::HasTalentGroup(int32_t talent_group_id) const
{
    const PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	return pTalent->IsTalentGroupExist(talent_group_id);
}

void Player::OpenTalentGroup(int32_t talent_group_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->OpenTalentGroup(talent_group_id);
}

void Player::OpenTalent(int32_t talent_group_id, int32_t talent_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->OpenTalent(talent_group_id, talent_id);
}

void Player::CloseTalentGroup(int32_t talent_group_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->CloseTalentGroup(talent_group_id);
}

void Player::LevelUpTalent(int32_t talent_group_id, int32_t talent_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->SystemLevelUp(talent_group_id, talent_id);
}

void Player::IncTalentTempLevel(int32_t talent_group_id, int32_t talent_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->IncTempLevel(talent_group_id, talent_id);
}

void Player::DecTalentTempLevel(int32_t talent_group_id, int32_t talent_id)
{
    PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->DecTempLevel(talent_group_id, talent_id);
}

void Player::QueryTalentSkill(std::set<int32_t>& skills) const
{
    const PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	pTalent->QueryTalentSkill(skills);
}

int32_t Player::GetTalentLevel(int32_t talent_group_id, int32_t talent_id) const
{
    const PlayerTalent* pTalent = GET_SUBSYS_PTR(PlayerTalent, TALENT);
	return pTalent->GetTalentLevel(talent_group_id, talent_id);
}

#define ABS_GREATER(value, num) (value > num || value < -num)
void Player::LandmineAccumulate()
{
	A2DVECTOR new_pos = pos();
	// 移动了
    float offset_x = new_pos.x - landmine_accumulate_.old_pos.x;
    float offset_y = new_pos.y - landmine_accumulate_.old_pos.y;
    if (ABS_GREATER(offset_x, 0.001) || ABS_GREATER(offset_y, 0.001))
	{
		landmine_accumulate_.old_pos = new_pos;
		IncLandmineAccumulate(1);
	}

	if (landmine_accumulate_.reject_flag > 0)
	{
		landmine_accumulate_.reject_flag--;
	}
}
#undef ABS_GREATER

bool Player::CheckLandmineAccumulate(int landmine_interval)
{
	if (landmine_accumulate_.reject_flag > 0)
	{
		landmine_accumulate_.reject_flag--;
		return false;
	}

	if (landmine_accumulate_.count >= landmine_interval)
	{
		ClrLandmineAccumulate();
		//ASSERT(landmine_accumulate_.count >= 0);
		return true;
	}

	return false;
}

void Player::RejectLandmineOneTime()
{
	// 保证至少1秒后才能触发暗雷，避免在心跳被过早扣掉
	landmine_accumulate_.reject_flag += 2;
}

void Player::AuctionItem(const playerdef::AuctionItemData& data)
{
    rpc_master_->AuctionItem(data);
}

void Player::AuctionItemResult(int err, int64_t auction_id, const void* request)
{
    PlayerAuction* pAuction = GET_SUBSYS_PTR(PlayerAuction, AUCTION);
    ASSERT(request);
	pAuction->AuctionItemResult(err, auction_id, request);
}

void Player::AuctionCancel(int64_t auction_id)
{
    rpc_master_->AuctionCancel(auction_id);
}

void Player::AuctionCancelResult(int err_code, int64_t auction_id, const void* response) const
{
    const PlayerAuction* pAuction = GET_SUBSYS_PTR(PlayerAuction, AUCTION);
    pAuction->AuctionCancelResult(err_code, auction_id, response);
}

void Player::AuctionBuyout(int64_t auction_id, int32_t currency_type, int32_t price)
{
    rpc_master_->AuctionBuyout(auction_id, currency_type, price);
}

void Player::AuctionBuyoutResult(int err_code, const void* request)
{
    PlayerAuction* pAuction = GET_SUBSYS_PTR(PlayerAuction, AUCTION);
    pAuction->AuctionBuyoutResult(err_code, request);
}

void Player::AuctionBid(int64_t auction_id, int32_t currency_type, int32_t price)
{
    rpc_master_->AuctionBid(auction_id, currency_type, price);
}

void Player::AuctionBidResult(int err_code, int32_t cur_price, int64_t cur_bidder, const void* request)
{
    PlayerAuction* pAuction = GET_SUBSYS_PTR(PlayerAuction, AUCTION);
    pAuction->AuctionBidResult(err_code, cur_price, cur_bidder, request);
}

void Player::AuctionInvalidQuery(const std::vector<int64_t>& auctionid_list)
{
    PlayerAuction* pAuction = GET_SUBSYS_PTR(PlayerAuction, AUCTION);
    pAuction->AuctionInvalidQuery(auctionid_list);
}

bool Player::IsFriend(int64_t roleid) const
{
    const PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->IsFriend(roleid);
}

int32_t Player::GetFriendNum() const
{
    const PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->GetFriendNum();
}

int32_t Player::GetEnemyNum() const
{
    const PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->GetEnemyNum();
}

void Player::AddNPCFriend(int32_t id)
{
    PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->AddNPCFriend(id);
}

void Player::DelNPCFriend(int32_t id)
{
    PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->DelNPCFriend(id);
}

void Player::OnlineNPCFriend(int32_t id)
{
    PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->OnlineNPCFriend(id);
}

void Player::OfflineNPCFriend(int32_t id)
{
    PlayerFriend* pFriend = GET_SUBSYS_PTR(PlayerFriend, FRIEND);
	return pFriend->OfflineNPCFriend(id);
}

void Player::QueryTitleSkill(std::set<int32_t>& skills) const
{
    const PlayerTitle* pTitle = GET_SUBSYS_PTR(PlayerTitle, TITLE);
    return pTitle->QueryTitleSkill(skills);
}

bool Player::GainTitle(int32_t title_id)
{
    PlayerTitle* pTitle = GET_SUBSYS_PTR(PlayerTitle, TITLE);
    return pTitle->GainTitle(title_id);
}

void Player::OpenReputation(int32_t reputation_id)
{
    PlayerReputation* pReputation = GET_SUBSYS_PTR(PlayerReputation, REPUTATION);
    pReputation->OpenReputation(reputation_id);
}

int32_t Player::GetReputation(int32_t reputation_id)
{
    PlayerReputation* pReputation = GET_SUBSYS_PTR(PlayerReputation, REPUTATION);
    return pReputation->GetReputation(reputation_id);
}

void Player::ModifyReputation(int32_t reputation_id, int32_t delta)
{
    PlayerReputation* pReputation = GET_SUBSYS_PTR(PlayerReputation, REPUTATION);
    pReputation->ModifyReputation(reputation_id, delta);
}

void Player::PlayerGetUIConf()
{
    G2C::UIData packet;
    packet.config = base_info_.ui_config;
    sender()->SendCmd(packet);
}

void Player::ModifyUIConf(int32_t ui_id, bool show)
{
    int32_t group_size = 8;
    if (ui_id >= MAX_UI_NUM * group_size)
    {
        return;
    }
    int32_t group_index = ui_id / group_size;
    ASSERT(group_index < MAX_UI_NUM);
    char& value = base_info_.ui_config[group_index];
    char flag = 0x01 << (ui_id % group_size);
    if (show)
    {
        value |= flag;
    }
    else
    {
        value &= ~flag;
    }
}

void Player::PlayerGetAchieveData()
{
    PlayerAchieve* pAchieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
    pAchieve->PlayerGetAchieveData();
}

void Player::PlayerGetLoginData()
{
    G2C::LoginData packet;
    packet.last_login_time = role_info_.last_login_time;
    sender()->SendCmd(packet);

    role_info_.last_login_time = g_timer->GetSysTime();
}

void Player::PlayerGetStarData()
{
    PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    pStar->PlayerGetStarData();
}

void Player::PlayerGetEnhanceData()
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->PlayerGetEnhanceData();
}

void Player::PlayerGetPCounterList() const
{
    const PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    pCounter->SendCounterList();
}

void Player::PlayerGetPunchCardData() const
{
    const PlayerPunchCard* pPunch = GET_SUBSYS_PTR(PlayerPunchCard, PUNCH_CARD);
    pPunch->GetPunchCardData();
}

void Player::PlayerGetMoveSpeed()
{
    sender()->NotifyPlayerMoveProp(cur_speed());
}

void Player::PlayerGetBossChallengeData() const
{
    const PlayerBossChallenge* pChallenge = GET_SUBSYS_PTR(PlayerBossChallenge, BOSS_CHALLENGE);
    pChallenge->PlayerGetBossChallengeData();
}

void Player::PlayerGetGameVersion()
{
    G2C::GameServerVersion packet;
    packet.resource_version = Gmatrix::GetGameResVersion();
    sender()->SendCmd(packet);
}

void Player::RefineAchieve(int32_t level)
{
    PlayerAchieve* pAchieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
    pAchieve->RefineAchieve(level);
}

void Player::OpenEnhanceSlot(int8_t mode)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->OpenEnhanceSlot(mode);
}

void Player::ProtectEnhanceSlot(int8_t index)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->ProtectEnhanceSlot(index);
}

void Player::UnProtectEnhanceSlot(int8_t index)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->UnProtectEnhanceSlot(index);
}

int32_t Player::GetUnProtectSlotNum()
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    return pEnhance->GetUnProtectSlotNum();
}

int32_t Player::GetProtectSlotNum()
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    return pEnhance->GetProtectSlotNum();
}

int32_t Player::RandSelectEnhanceId(int32_t enhance_gid)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    return pEnhance->RandSelectEnhanceId(enhance_gid);
}

int32_t Player::GetEnhanceNum(int32_t level, int32_t rank)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    return pEnhance->GetEnhanceNum(level, rank);
}

void Player::DoEnhance(int32_t enhance_gid, int32_t count)
{
    PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->DoEnhance(enhance_gid, count);
}

void Player::QueryEnhanceSkill(std::set<int32_t>& skills) const
{
    const PlayerEnhance* pEnhance = GET_SUBSYS_PTR(PlayerEnhance, ENHANCE);
    pEnhance->QueryEnhanceSkill(skills);
}

achieve::ActiveAchieve* Player::GetActiveAchieve()
{
    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->GetActiveAchieve();
}

achieve::FinishAchieve* Player::GetFinishAchieve()
{
    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->GetFinishAchieve();
}

achieve::AchieveData* Player::GetAchieveData()
{
    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->GetAchieveData();
}

void Player::StorageTaskComplete(int32_t taskid, int8_t quality)
{
    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->StorageTaskComplete(taskid, quality);
}

void Player::FinishInstance(int32_t ins_id)
{
    PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->FinishInstance(ins_id);
}

int32_t Player::GetInsFinishCount(int32_t ins_id) const
{
    const PlayerAchieve* pachieve = GET_SUBSYS_PTR(PlayerAchieve, ACHIEVE);
	return pachieve->GetInsFinishCount(ins_id);
}

void Player::PlayerGetPropReinforce()
{
    PlayerPropReinforce* pPropRF = GET_SUBSYS_PTR(PlayerPropReinforce, PROP_REINFORCE);
    pPropRF->PlayerGetPropReinforce();
}

void Player::ResetPrimaryPropRF()
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerPropReinforce* pPropRF = GET_SUBSYS_PTR(PlayerPropReinforce, PROP_REINFORCE);
    pPropRF->ResetPrimaryPropRF();
}

void Player::AddPrimaryPropRFCurEnergy()
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;
    
    PlayerPropReinforce* pPropRF = GET_SUBSYS_PTR(PlayerPropReinforce, PROP_REINFORCE);
    pPropRF->AddPrimaryPropRFCurEnergy();
}

void Player::OpenStar(int32_t star_id)
{
    PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    pStar->OpenStar(star_id);
}

void Player::CloseStar(int32_t star_id)
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    pStar->CloseStar(star_id);
}

void Player::ActivateSpark(int32_t star_id)
{
    PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    pStar->ActivateSpark(star_id);
}

bool Player::AllSparkActivate(int32_t star_id) const
{
    const PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    return pStar->AllSparkActivate(star_id);
}

int32_t Player::GetFullActivateStarNum() const
{
    const PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    return pStar->GetFullActivateStarNum();
}

int32_t Player::GetSparkNum() const
{
    const PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    return pStar->GetSparkNum();
}

bool Player::IsStarOpen(int32_t star_id) const
{
    const PlayerStar* pStar = GET_SUBSYS_PTR(PlayerStar, STAR);
    return pStar->IsStarOpen(star_id);
}

void Player::ClearLandmineTrigger()
{
    PlayerLandmine* pLandmine = GET_SUBSYS_PTR(PlayerLandmine, LANDMINE);
    pLandmine->ResetTriggerData();
}

bool Player::GainCard(int32_t card_item_id, int32_t card_item_idx)
{
    PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	return pCard->GainCard(card_item_id, card_item_idx);
}

bool Player::RegisterCard(const playerdef::CardEntry& card)
{
    PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	return pCard->RegisterCard(card);
}

bool Player::UnRegisterCard(int32_t card_item_idx)
{
    PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	return pCard->UnRegisterCard(card_item_idx);
}

bool Player::QueryCardInfo(int32_t card_item_idx, playerdef::CardEntry& card) const
{
    const PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	return pCard->QueryCardInfo(card_item_idx, card);
}

void Player::QueryCardSkill(std::set<int32_t>& skills) const
{
    const PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	pCard->QueryCardSkill(skills);
}

int32_t Player::GetEquipCardNum(int32_t rank, int32_t level) const
{
    const PlayerCard* pCard = GET_SUBSYS_PTR(PlayerCard, CARD);
	return pCard->GetEquipCardNum(rank, level);
}

void Player::ErrorMessage(int err_no) const
{
    sender_->ErrorMessage(err_no);
}

void Player::Say(const std::string& content)
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerChat* pChat = GET_SUBSYS_PTR(PlayerChat, CHAT);
    pChat->SendDebugChat(content);
}

bool Player::RegisterPCounter(int32_t tid)
{
    PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    return pCounter->RegisterCounter(tid);
}

bool Player::UnregisterPCounter(int32_t tid)
{
    PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    return pCounter->UnregisterCounter(tid);
}

bool Player::ModifyPCounter(int32_t tid, int32_t delta)
{
    PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    return pCounter->ModifyCounter(tid, delta);
}

void Player::ModifyPCounter(int32_t id, int8_t op, int32_t value)
{
    switch (op)
    {
    case task::COUNTER_OP_INC:
        ModifyPCounter(id, value);
        break;

    case task::COUNTER_OP_DEC:
        ModifyPCounter(id, -value);
        break;

    case task::COUNTER_OP_ASSIGN:
        SetPCounter(id, value);
        break;

    case task::COUNTER_OP_LOAD:
        RegisterPCounter(id);
        break;

    case task::COUNTER_OP_UNLOAD:
        UnregisterPCounter(id);
        break;

    default:
        LOG_ERROR << "OperatePCounter() error! id:" << id << " op:" << op 
            << " value:" << value;
        break;
    }
}

bool Player::GetPCounter(int32_t tid, int32_t& value) const
{
    const PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    return pCounter->GetCounter(tid, value);
}

bool Player::SetPCounter(int32_t tid, int32_t value)
{
    PlayerCounter* pCounter = GET_SUBSYS_PTR(PlayerCounter, PLAYER_COUNTER);
    return pCounter->SetCounter(tid, value);
}

void Player::PlayerGetParticipation() const
{
    const PlayerParticipation* sub = GET_SUBSYS_PTR(PlayerParticipation, PARTICIPATION);
    sub->PlayerGetParticipationData();
}

void Player::ModifyParticipation(int32_t value)
{
    PlayerParticipation* sub = GET_SUBSYS_PTR(PlayerParticipation, PARTICIPATION);
    sub->ModifyParticipation(value);
}

void Player::SetParticipation(int32_t value)
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerParticipation* sub = GET_SUBSYS_PTR(PlayerParticipation, PARTICIPATION);
    sub->SetParticipation(value);
}

void Player::PlayerGetGeventData() const
{
    const PlayerGevent* gevent = GET_SUBSYS_PTR(PlayerGevent, GEVENT);
    gevent->PlayerGetGeventData();
}

void Player::CompleteGevent(int32_t gevent_gid)
{
    PlayerGevent* gevent = GET_SUBSYS_PTR(PlayerGevent, GEVENT);
    gevent->CompleteGevent(gevent_gid);
}

void Player::SetGeventNum(int32_t gevent_gid, int32_t num)
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd || gevent_gid < 0 || num < 0)
        return;

    PlayerGevent* gevent = GET_SUBSYS_PTR(PlayerGevent, GEVENT);
    gevent->SetGeventNum(gevent_gid, num);
}

void Player::ClearParticipationAward()
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerParticipation* parti = GET_SUBSYS_PTR(PlayerParticipation, PARTICIPATION);
    parti->ClearParticipationAward();
}

void Player::PlayerGetMountData() const
{
    const PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    pMount->PlayerGetMountData();
}

void Player::RegisterMount(int32_t item_index, int32_t item_id)
{
    PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    pMount->RegisterMount(item_index, item_id);
}

void Player::UnRegisterMount(int32_t item_index)
{
    PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    pMount->UnRegisterMount(item_index);
}

void Player::OpenMountEquip(int32_t equip_index)
{
    PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    pMount->OpenMountEquip(equip_index);
}

int32_t Player::GetMountCategory() const
{
    const PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    return pMount->GetMountCategory();
}

int32_t Player::GetMountEquipLevel(int32_t index) const
{
    const PlayerMount* pMount = GET_SUBSYS_PTR(PlayerMount, MOUNT);
    return pMount->GetMountEquipLevel(index);
}

void Player::ResetPunchCard()
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerPunchCard* pPunch = GET_SUBSYS_PTR(PlayerPunchCard, PUNCH_CARD);
    pPunch->ResetPunchCard();
}

void Player::WinBossChallenge(int8_t result, int32_t challenge_id, int32_t monster_gid)
{
    PlayerBossChallenge* pChallenge = GET_SUBSYS_PTR(PlayerBossChallenge, BOSS_CHALLENGE);
	pChallenge->WinBossChallenge(result, challenge_id, monster_gid);
}

void Player::ClearBossChallenge(int32_t challenge_id)
{
    if (!Gmatrix::GetServerParam().permit_debug_cmd)
        return;

    PlayerBossChallenge* pChallenge = GET_SUBSYS_PTR(PlayerBossChallenge, BOSS_CHALLENGE);
	pChallenge->ClearBossChallenge(challenge_id);
}

} // namespace gamed
