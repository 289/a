inline bool CombatUnit::CanAction() const
{
	//眩晕和睡眠状态禁止行动;
	return unit_state_.OptionPolicy(OPT_ACTION);
}

inline bool CombatUnit::CanAttacked() const
{
	return unit_state_.OptionPolicy(OPT_ATTACKED);
}

inline void CombatUnit::UpdateState(Event event, int timeout)
{
	unit_state_.Update(event, timeout, this);
}

inline void CombatUnit::SetStateTimeout(int timeout)
{
	unit_state_.SetTimeOut(timeout);
}

inline int CombatUnit::GetStateTimeout() const
{
	return unit_state_.GetTimeOut();
}

inline bool CombatUnit::IsDead() const
{
	return unit_state_.IsDead();
}

inline bool CombatUnit::IsDying() const
{
	return unit_state_.IsDying();
}

inline bool CombatUnit::IsZombie() const
{
	return unit_state_.IsZombie();
}

inline bool CombatUnit::IsZombieDying() const
{
	return unit_state_.IsZombieDying();
}

inline bool CombatUnit::IsNormal() const
{
	return unit_state_.IsNormal();
}

inline bool CombatUnit::IsAction() const
{
	return unit_state_.IsAction();
}

inline bool CombatUnit::IsAttacked() const
{
	return unit_state_.IsAttacked();
}

inline bool CombatUnit::IsSneaked() const
{
	return unit_state_.IsSneaked();
}

inline bool CombatUnit::IsTransformWait() const
{
    return unit_state_.IsTransformWait();
}

inline bool CombatUnit::IsTransforming() const
{
    return unit_state_.IsTransforming();
}

inline bool CombatUnit::IsPlayerAction() const
{
    return unit_state_.IsPlayerAction();
}

inline bool CombatUnit::IsGolemAction() const
{
    return unit_state_.IsGolemAction();
}

inline bool CombatUnit::IsWaitGolem() const
{
    return unit_state_.IsWaitGolem();
}

inline bool CombatUnit::IsATBOwner() const
{
	return is_atb_owner_;
}

inline bool CombatUnit::IsAlive() const
{
	return !IsDying() && !IsDead() && !IsZombie() && !TestEscaping();
}

inline void CombatUnit::SetModel(const std::string& model)
{
    model_ = model;
}

inline std::string CombatUnit::GetModel() const
{
    return model_;
}

inline void CombatUnit::SetATBOwner()
{
	is_atb_owner_ = true;
}

inline int CombatUnit::GetPos() const
{
	return pos_;
}

inline void CombatUnit::SetPos(int pos)
{
	//TODO
	//BOSS战斗可能要调整
	pos_ = pos;
}

inline void CombatUnit::SetLevel(int16_t level)
{
	level_ = level;
}

inline int CombatUnit::GetLevel() const
{
	return level_;
}

inline void CombatUnit::SetParty(int party)
{
    party_ = party;
}

inline int CombatUnit::GetParty() const
{
    return party_;
}

inline int CombatUnit::GetRoundCounter() const
{
	return round_counter_;
}

inline void CombatUnit::SetCombat(Combat* combat)
{
	assert(combat);
	combat_ = combat;
}

inline Combat* CombatUnit::GetCombat() const
{
	return combat_;
}

inline int CombatUnit::GetHP() const
{
	return basic_prop_.hp;
}

inline int CombatUnit::GetMP() const
{
	return basic_prop_.mp;
}

inline int CombatUnit::GetEP() const
{
	return basic_prop_.ep;
}

inline int CombatUnit::GetMaxHP() const
{
	return cur_prop_[PROP_INDEX_MAX_HP];
}

inline int CombatUnit::GetMaxMP() const
{
	return cur_prop_[PROP_INDEX_MAX_MP];
}

inline int CombatUnit::GetMaxEP() const
{
	return cur_prop_[PROP_INDEX_MAX_EP];
}

inline int CombatUnit::GetATBTime() const
{
	return cur_prop_[PROP_INDEX_ATB_TIME];
}

inline int CombatUnit::GetAttackPriority() const
{
	return cur_prop_[PROP_INDEX_ATTACK_PRIORITY];
}

inline void CombatUnit::SetATBTime(int atb_time)
{
	assert(atb_time);
	cur_prop_[PROP_INDEX_ATB_TIME] = atb_time;
}

inline void CombatUnit::SetMP(int mp)
{
	int tmp = mp;
	if (tmp > cur_prop_[PROP_INDEX_MAX_MP])
		tmp = cur_prop_[PROP_INDEX_MAX_MP];
	basic_prop_.mp = tmp;
}

inline void CombatUnit::SetEP(int ep)
{
	int tmp = ep;
	if (tmp > cur_prop_[PROP_INDEX_MAX_EP])
		tmp = cur_prop_[PROP_INDEX_MAX_EP];
	basic_prop_.ep = tmp;
}

inline int32_t CombatUnit::GetRefreshExtPropMask() const
{
	return refresh_extprop_mask_;
}

inline void CombatUnit::SetRefreshVolatileProp()
{
	refresh_volatile_prop_ = true;
}

inline void CombatUnit::ClrRefreshVolatileProp()
{
	refresh_volatile_prop_ = false;
}

inline bool CombatUnit::TestRefreshVolatileProp() const
{
	return refresh_volatile_prop_;
}

inline void CombatUnit::SetRefreshBasicProp()
{
	refresh_basic_prop_ = true;
}

inline void CombatUnit::ClrRefreshBasicProp()
{
	refresh_basic_prop_ = false;
}

inline bool CombatUnit::TestRefreshBasicProp() const
{
	return refresh_basic_prop_;
}

inline void CombatUnit::SetRefreshExtPropMask(size_t index)
{
	refresh_extprop_mask_ |= 1 << index;
}

inline void CombatUnit::ClrRefreshExtPropMask()
{
	refresh_extprop_mask_ = 0;
}

inline void CombatUnit::SetRefreshCurGolem()
{
	refresh_cur_golem_ = true;
}

inline void CombatUnit::ClrRefreshCurGolem()
{
	refresh_cur_golem_ = false;
}

inline bool CombatUnit::TestRefreshCurGolem() const
{
	return refresh_cur_golem_;
}

inline void CombatUnit::SetRefreshOtherGolem()
{
	refresh_other_golem_ = true;
}

inline void CombatUnit::ClrRefreshOtherGolem()
{
	refresh_other_golem_ = false;
}

inline bool CombatUnit::TestRefreshOtherGolem() const
{
	return refresh_other_golem_;
}

inline UnitID CombatUnit::GetKiller() const
{
	return killer_;
}

inline void CombatUnit::SetKiller(UnitID killer)
{
	killer_ = killer;
}

inline skill::SkillWrapper* CombatUnit::GetSkillWrapper()
{
	return skill_;
}

inline skill::BuffWrapper* CombatUnit::GetBuffWrapper()
{
	return buff_man_;
}

inline skill::CooldownWrapper* CombatUnit::GetCDWrapper()
{
	return skill_cd_;
}

inline void CombatUnit::AddSkill(SkillID skill)
{
	skill_->AddSkill(skill);
}

inline void CombatUnit::SetDefaultSkill(SkillID skill)
{
	default_skill_ = skill;
	skill_->AddSkill(skill);
	skill_->set_default_skillid(skill);
}

inline void CombatUnit::SetRecastSkill(SkillID skill)
{
	recast_skill_ = skill;
}

inline void CombatUnit::ClrRecastSkill()
{
	recast_skill_ = 0;
}

inline SkillID CombatUnit::GetDefaultSkill() const
{
	return default_skill_;
}

inline SkillID CombatUnit::GetRecastSkill() const
{
	return recast_skill_;
}

inline bool CombatUnit::IsDizzy() const
{
	return state_ & STATE_DIZZY;
}

inline bool CombatUnit::IsSleep() const
{
	return state_ & STATE_SLEEP;
}

inline bool CombatUnit::IsSilent() const
{
	return state_ & STATE_SILENT;
}

inline bool CombatUnit::IsConfuse() const
{
	return state_ & STATE_CONFUSE;
}

inline bool CombatUnit::IsCharging() const
{
	return state_ & STATE_CHARGING;
}

inline bool CombatUnit::IsCharged() const
{
	return state_ & STATE_CHARGED;
}

inline void CombatUnit::SetDizzy(bool bdizzy)
{
	//回合开始可能被设置
	if (bdizzy)
		state_ |= STATE_DIZZY;
	else
		state_ &= ~STATE_DIZZY;
}

inline void CombatUnit::SetSleep(bool bsleep)
{
	//任何时候都可能被设置
	if (bsleep)
		state_ |= STATE_SLEEP;
	else
		state_ &= ~STATE_SLEEP;
}

inline void CombatUnit::SetSilent(bool bsilent)
{
	if (bsilent)
		state_ |= STATE_SILENT;
	else
		state_ &= ~STATE_SILENT;
}

inline void CombatUnit::SetConfuse(bool bconfuse)
{
	if (bconfuse)
		state_ |= STATE_CONFUSE;
	else
		state_ &= ~STATE_CONFUSE;
}

inline void CombatUnit::SetCharging(bool bcharging)
{
	if (bcharging)
		state_ |= STATE_CHARGING;
	else
		state_ &= ~STATE_CHARGING;
}

inline void CombatUnit::SetCharged(bool bcharged)
{
	if (bcharged)
		state_ |= STATE_CHARGED;
	else
		state_ &= ~STATE_CHARGED;
}

inline void CombatUnit::SetAttackDone()
{
	attack_done_ = true;
}

inline void CombatUnit::ClrAttackDone()
{
	attack_done_ = false;
}

inline bool CombatUnit::TestAttackDone() const
{
	return attack_done_;
}

inline void CombatUnit::SetSneaked()
{
	UpdateState(EVENT_SNEAKED);
}

inline const std::vector<skill::SkillDamage>& CombatUnit::GetSkillDamages() const
{
	return skill_damages_;
}

inline void CombatUnit::SetTransforming()
{
    is_transforming_ = true;
}

inline void CombatUnit::ClrTransforming()
{
    is_transforming_ = false;
}

inline bool CombatUnit::TestTransforming() const
{
    return is_transforming_;
}

inline void CombatUnit::SetEscaping()
{
    is_escaping_ = true;
}

inline bool CombatUnit::TestEscaping() const
{
    return is_escaping_;
}

