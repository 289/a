#ifndef TASK_BASIC_COND_INL_H_
#define TASK_BASIC_COND_INL_H_

// CheckDataValidity要注意保证默认值返回true
inline bool LevelCond::CheckDataValidity() const
{
	if (min_level < 0 || max_level < 0)
	{
		return false;
	}
	if (max_level != 0 && min_level > max_level)
	{
		return false;
	}
	return true;
}

inline bool GoldCond::CheckDataValidity() const
{
	return gold >= 0;
}

inline bool CashCond::CheckDataValidity() const
{
	return cash >= 0;
}

inline bool ScoreCond::CheckDataValidity() const
{
	return score >= 0;
}

inline bool ItemCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(item_list)
	return true;
}

inline bool GenderCond::CheckDataValidity() const
{
	return gender >= GENDER_NONE && gender <= GENDER_FEMALE;
}

inline bool RoleClassCond::CheckDataValidity() const
{
	RoleClassVec::const_iterator it = class_list.begin();
	for (; it != class_list.end(); ++it)
	{
		if (*it < CLS_NONE || *it >= CLS_MAXCLS_LABEL)
		{
			return false;
		}
	}
	return true;
}

inline bool ZoneCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(zone_list)
	return true;
}

inline bool TimeCond::CheckDataValidity() const
{
	return time >= 0;
}

inline bool TimeSegmentCond::CheckDataValidity() const
{
	return true;
}

inline bool MonsterCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(monster_list)
	return true;
}

inline bool TaskCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(task_list)
	return true;
}

inline bool InstanceCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(instance_list)
	return true;
}

inline bool CounterCond::CheckDataValidity() const
{
	CHECK_VEC_VALIDITY(counter_list)
	return true;
}

inline bool ReputationCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(reputation_list)
    return true;
}

inline bool TalentCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(talent_list)
    return true;
}

inline bool BuffCond::CheckDataValidity() const
{
    BuffVec::const_iterator bit = buff_list.begin();
    for (; bit != buff_list.end(); ++bit)
    {
        if (*bit <= 0)
        {
            return false;
        }
    }
    return true;
}

inline bool CombatValueCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(value_list)
    return true;
}

inline bool PetPowerCond::CheckDataValidity() const
{
    return value >= 0;
}

inline bool PetCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(pet_list)
    return true;
}

inline bool CardCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(card_list)
    return true;
}

inline bool EnhanceCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(enhance_list)
    return true;
}

inline bool EquipCond::CheckDataValidity() const
{
    CHECK_VEC_VALIDITY(equip_list)
    return true;
}

inline bool StarCond::CheckDataValidity() const
{
    return full_activate_num >= 0;
}

#endif // TASK_BASIC_COND_INL_H_
