#ifndef GAMED_GS_PLAYER_PLAYER_INL_H_
#define GAMED_GS_PLAYER_PLAYER_INL_H_

///
/// inline function 
///
inline PlayerController* Player::commander()
{
	return commander_;
}

inline PlayerMoveControl* Player::move_ctrl()
{
	return move_ctrl_;
}

inline PlayerSender* Player::sender()
{
	return sender_;
}

inline SubSysIf* Player::subsys_if()
{
	return subsys_if_;
}

inline PlayerState& Player::state()
{
	return state_;
}

inline PlayerVisibleState& Player::visible_state()
{
	return visible_state_;
}

inline PlayerValueProp& Player::value_prop()
{
    return value_prop_;
}

inline RoleID Player::role_id() const
{
	return object_id();
}

inline void Player::set_link_id(int32_t id)
{
	link_id_ = id;
}

inline void Player::set_sid_in_link(int32_t sid)
{
	sid_in_link_ = sid;
}

inline void Player::set_master_id(int32_t id)
{
	master_id_ = id;
}

inline void Player::set_user_id(int64_t userid)
{
    ASSERT(userid > 0);
    user_id_ = userid;
}

inline int64_t Player::user_id() const
{
    return user_id_;
}

inline bool Player::refresh_state() const
{
	return refresh_state_;
}

inline int32_t Player::link_id() const
{
	return link_id_;
}
	
inline int32_t Player::sid_in_link() const
{
	return sid_in_link_;
}

inline int32_t Player::master_id() const
{
	return master_id_;
}

inline PlayerState::MaskType Player::GetState() const
{
	return state_.state();
}

inline float Player::GetSpeedByMode(int mode) const
{
	return cur_speed();
}

inline int32_t Player::level() const
{
	return base_info_.level;
}

inline uint8_t Player::role_class() const
{
	return base_info_.cls;
}

inline ClassMask Player::class_mask() const
{
	ClassMask cls_mask;
	cls_mask.set(base_info_.cls, 1);
	return cls_mask;
}

inline std::string Player::rolename() const
{
	return role_info_.first_name + role_info_.mid_name + role_info_.last_name;
}

inline std::string Player::first_name() const
{
	return role_info_.first_name;
}

inline std::string Player::middle_name() const
{
	return role_info_.mid_name;
}

inline std::string Player::last_name() const
{
	return role_info_.last_name;
}

inline int32_t Player::create_time() const
{
	return role_info_.created_time;
}

inline int32_t Player::last_login_time() const
{
    return role_info_.last_login_time;
}

inline uint32_t Player::equip_crc()
{
	return equip_crc_;
}

inline uint8_t Player::gender() const
{
	return base_info_.gender;
}

inline float Player::cur_speed() const
{
	//return kPlayerDefaultMoveSpeed;
	return (float)cur_prop_[PROP_INDEX_MOVE_SPEED] / 1000.f;
}

inline int32_t Player::faction() const
{
	return faction_;
}

inline int64_t Player::GetMoney() const
{
	return base_info_.money;
}

inline bool Player::CheckMoney(int32_t money) const
{
	return base_info_.money >= money;
}

inline int64_t Player::GetScore() const
{
	return spec_money_.score_total - spec_money_.score_used;
}

inline int64_t Player::GetScoreTotal() const
{
    return spec_money_.score_total;
}

inline int64_t Player::GetScoreUsed() const
{
    return spec_money_.score_used;
}

inline bool Player::CheckScore(int32_t score) const
{
	return GetScore() >= score;
}

inline void Player::SetRefreshState()
{
	refresh_state_ = true;
}

inline void Player::ClrRefreshState()
{
	refresh_state_ = false;
}

inline bool Player::GetRefreshState() const
{
	return refresh_state_;
}

inline void Player::SetRefreshExtProp(size_t index)
{
	refresh_extprop_ |= 1 << index;
}

inline void Player::ClrRefreshExtProp()
{
	refresh_extprop_ = 0;
}

inline int32_t Player::GetRefreshExtProp() const
{
	return refresh_extprop_;
}

inline int32_t Player::GetHP() const
{
	return base_info_.hp;
}

inline int32_t Player::GetExp() const
{
	return base_info_.exp;
}

inline int32_t Player::GetMP() const
{
	return base_info_.mp;
}

inline int32_t Player::GetEP() const
{
	return base_info_.ep;
}

inline int32_t Player::GetMaxHP() const
{
	return cur_prop_[PROP_INDEX_MAX_HP];
}

inline int32_t Player::GetMaxMP() const
{
	return cur_prop_[PROP_INDEX_MAX_MP];
}

inline int32_t Player::GetMaxEP() const
{
	return cur_prop_[PROP_INDEX_MAX_EP];
}

inline int32_t Player::GetLevel() const
{
	return base_info_.level;
}

inline int32_t Player::GetCatVisionExp() const
{
	return base_info_.cat_exp;
}

inline int32_t Player::GetCatVisionLevel() const
{
	return base_info_.cat_level;
}

inline void Player::SetLevel(int32_t level)
{
	base_info_.level = 1;
}

inline int32_t Player::GetMaxProp(size_t index) const
{
	if (index < 0 || index >= PROP_INDEX_HIGHEST)
	{
		return 0;
	}
	return cur_prop_[index];
}

inline int32_t Player::GetBaseProp(size_t index) const
{
    if (index < 0 || index >= PROP_INDEX_HIGHEST)
    {
        return 0;
    }
    return base_prop_[index];
}

inline bool Player::InCombat() const
{
	return state_.IsCombat();
}

inline bool Player::CanCombat() const
{
	// 被动标记不进入战斗
	if (passivity_flag_.is_passive())
		return false;

	return state_.CanCombat();
}

inline bool Player::CanSwitch() const
{
	// 被动标记不能传送
	if (passivity_flag_.is_passive())
		return false;

	return state_.CanSwitch();
}

inline bool Player::can_see_each_other() const
{
	// 剧情地图玩家相互看不到
	if (IS_DRAMA_MAP(world_id()))
		return false;

	return true;
}

inline int32_t Player::cat_vision_level() const
{
	return base_info_.cat_level;
}

inline void Player::set_equip_crc(uint32_t crc)
{
	equip_crc_ = crc;
}

#define DEFINE_EXT_STATE_FUNC(name, NAME) \
	inline void Player::SetImmune##name() \
	{ \
		extend_state_ |= ES_IMMUNE_##NAME; \
	} \
	inline void Player::ClrImmune##name() \
	{ \
		extend_state_ &= ~ES_IMMUNE_##NAME; \
	} \
	inline bool Player::IsImmune##name() const \
	{ \
		return extend_state_ & ES_IMMUNE_##NAME; \
	}

DEFINE_EXT_STATE_FUNC(ActiveMonster, ACTIVE_MONSTER)
DEFINE_EXT_STATE_FUNC(Landmine, LANDMINE)
DEFINE_EXT_STATE_FUNC(TeamCombat, TEAM_COMBAT)
#undef DEFINE_EXT_STATE_FUNC

inline void Player::IncLandmineAccumulate(int32_t count)
{
	landmine_accumulate_.count += count;
}

inline void Player::DecLandmineAccumulate(int32_t count)
{
	landmine_accumulate_.count -= count;
}

inline void Player::ClrLandmineAccumulate()
{
	landmine_accumulate_.count = 0;
}

inline void Player::SetLandmineAccumulate(int32_t count)
{
	landmine_accumulate_.count = count;
}

inline void Player::RejectPassivityFlag()
{
	passivity_flag_.set_reject();
}

inline void Player::ClrRejectPassivityFlag()
{
	passivity_flag_.clr_reject();
}

inline void Player::SetRuningDramaScript(bool is_runing)
{
	if (is_runing)
	{
		// set passivity flag
		passivity_flag_.set_passive(0xFFFF);

		// save drama pos
		drama_save_pos_.world_id = world_id();
		drama_save_pos_.pos      = pos();
	}
	else
	{
		// clear passivity flag
		passivity_flag_.clear();

		// clear drama pos
		drama_save_pos_.clear();
	}
}

#endif // GAMED_GS_PLAYER_PLAYER_INL_H_
