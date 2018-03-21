#ifndef __GAME_MODULE_COMBAT_MOB_WRAPPER_H__
#define __GAME_MODULE_COMBAT_MOB_WRAPPER_H__

#include <stdint.h>
#include "combat_npc.h"

namespace combat
{

class CombatNpc;
class MobWrapper
{
public:
	CombatNpc* npc_;

public:
	MobWrapper(CombatNpc* npc):
		npc_(npc)
	{
	}
	
	~MobWrapper()
	{
	}
	
	int32_t GetNpcID()
	{
		return npc_->GetNpcID();
	}

	int32_t GetLevel()
	{
		return npc_->GetLevel();
	}
	
	int32_t GetHP()
	{
		return npc_->GetHP();
	}
	
	int32_t GetMP()
	{
		return npc_->GetMP();
	}
	
	int32_t GetEP()
	{
		return npc_->GetEP();
	}
	
	int32_t GetTeammatesAlive()
	{
		return npc_->GetTeammatesAlive();
	}
	
	int32_t GetEnemiesAlive()
	{
		return npc_->GetEnemiesAlive();
	}
	
	int32_t GetProperty(int prop_index)
	{
		return npc_->GetProp(prop_index);
	}

	int GetRoundCounter()
	{
		return npc_->GetRoundCounter();
	}

	void SetSkill(int32_t skill_id)
	{
		npc_->SetSkill(skill_id);
	}

	void Speak(int id_talk, int msec)
	{
		npc_->Speak(id_talk, msec);
	}

	void CastInstantSkill(int skillid)
	{
		npc_->CastInstantSkill(skillid);
	}

    void Transform(TemplID mob_tid)
    {
        npc_->TriggerTransform(mob_tid);
    }

    void Escape(int32_t result)
    {
        npc_->TriggerEscape(result);
    }
};

};

#endif //__GAME_MODULE_COMBAT_MOB_WRAPPER_H__
