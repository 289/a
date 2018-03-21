///
/// inline func
///
inline UnitID Combat::object_id() const
{
	return xid_.id;
}

inline CombatSceneAI* Combat::scene_ai() const
{
    return scene_ai_;
}

inline int32_t Combat::GetMapID() const
{
	return map_id_;
}

inline WObjID Combat::GetWorldObjectID() const
{
	return world_id_;
}

inline UnitID Combat::GetCombatID() const
{
	return xid_.id;
}

inline int32_t Combat::GetCombatSceneID() const
{
	return scene_id_;
}

inline int32_t Combat::GetTickCounter() const
{
	return tick_counter_;
}

inline int32_t Combat::GetMobCount(bool mob) const
{
	int32_t count = 0;
    const CombatUnitVec& unit_list = mob ? defender_list_ : attacker_list_;
    for (size_t i = 0; i < unit_list.size(); ++i)
    {
        if (unit_list[i] != NULL)
        {
            ++count;
        }
    }
	//for (size_t i = 0; i < defender_list_.size(); ++ i)
		//if (defender_list_[i])
			//++count;
	return count;
}

inline int32_t Combat::GetRoundCount() const
{
	return round_counter_;
}

inline int32_t Combat::GetChallengeID() const
{
	return challengeid_;
}

inline void Combat::SetCreator(int64_t roleid)
{
	creator_roleid_ = roleid;
}

inline void Combat::SetTaskID(int32_t taskid)
{
	taskid_ = taskid;
}

inline void Combat::SetChallengeID(int32_t challengeid)
{
	challengeid_ = challengeid;
}

inline void Combat::AttachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher, UnitID target)
{
	combat_buffer_.AttachBuffer(buff_seq, buff_id, attacher, target);
}

inline void Combat::DetachBuffer(uint32_t buff_seq, int32_t buff_id, UnitID attacher, UnitID target)
{
	combat_buffer_.DetachBuffer(buff_seq, buff_id, attacher, target);
}

inline void Combat::SetAttackerSneaked()
{
	attacker_sneaked_ = true;
}

inline void Combat::SetDefenderSneaked()
{
	defender_sneaked_ = true;
}

inline void Combat::ClrSneakedState()
{
	attacker_sneaked_ = false;
	defender_sneaked_ = false;
}

inline void Combat::AddCombatPet(CombatNpc* pet)
{
	assert(pet_set_.insert(pet).second);
}

inline void Combat::RmvCombatPet(CombatNpc* pet)
{
	assert(pet_set_.erase(pet) == 1);
}

inline void Combat::AddGolem(CombatNpc* golem)
{
	assert(golem_set_.insert(golem).second);
}

inline void Combat::RmvGolem(CombatNpc* golem)
{
	assert(golem_set_.erase(golem) == 1);
}

inline void Combat::SetWorldBossCombat(bool is_boss_combat)
{
	is_boss_combat_ = is_boss_combat;
}

inline bool Combat::IsWorldBossCombat() const
{
	return is_boss_combat_;
}

inline bool Combat::CanRegisterATB() const
{
    return combat_state_.OptionPolicy(XOPT_REGISTER_ATB);
}

inline bool Combat::CanUnRegisterATB() const
{
    return combat_state_.OptionPolicy(XOPT_UNREGISTER_ATB);
}

inline bool Combat::CanPlayerOperate() const
{
    return combat_state_.OptionPolicy(XOPT_PLAYER_OPERATE);
}

inline bool Combat::CanScriptOperate() const
{
    return combat_state_.OptionPolicy(XOPT_SCRIPT_OPERATE);
}

inline bool Combat::CanStopCombat() const
{
    return combat_state_.OptionPolicy(XOPT_STOP);
}

inline bool Combat::CanSuspendCombat() const
{
    return combat_state_.OptionPolicy(XOPT_SUSPEND);
}

inline bool Combat::CanWaitSelectSkill() const
{
    return combat_state_.OptionPolicy(XOPT_WAIT_SELECT_SKILL);
}

inline bool Combat::CanWaitSelectPet() const
{
    return combat_state_.OptionPolicy(XOPT_WAIT_SELECT_PET);
}

inline void CombatPVE::SetInitPlayerCount(int32_t player_count)
{
	init_player_count_ = player_count;
}

inline void CombatPVE::SetMobGroupId(TemplID mob_group_id)
{
	mob_group_id_ = mob_group_id;
}

inline void CombatPVE::SetWorldBossID(int32_t world_boss_id)
{
	world_boss_id_ = world_boss_id;
}

inline void CombatPVE::SetWorldMonsterID(int32_t world_monster_object_id)
{
	world_monster_id_ = world_monster_object_id;
}

inline int32_t CombatPVE::GetWorldBossID() const
{
	return world_boss_id_;
}

inline void CombatPVP::SetCombatFlag(int flag)
{
    combat_flag_ = flag;
}
