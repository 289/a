#include <algorithm>
#include "world_boss.h"
#include "combat_def.h"
#include "combat.h"
#include "combat_unit.h"
#include "combat_npc.h"
#include "combat_man.h"
#include "combat_player.h"


namespace combat {

WorldBoss::WorldBoss(): Object(OBJ_TYPE_WORLD_BOSS),
	world_boss_object_id_(0),
	some_combat_end_(false),
	boss_hp_change_(false),
	is_combating_(false)
{
}

WorldBoss::~WorldBoss()
{
}

void WorldBoss::RegisterCombat(ObjectID combat_id)
{
	CombatNode info;
	info.combat_id = combat_id;
	info.running = true;
	info.result = RESULT_INVALID;
	combat_list_.push_back(info);

	is_combating_ = true;
}

void WorldBoss::RegisterPlayer(CombatID combat_id, UnitID unit_id, RoleID role_id)
{
    DamageMap& damage_map      = player_map_[combat_id];
    DamageMap::iterator it_dam = damage_map.find(unit_id);
    if (it_dam == damage_map.end())
    {
        damage_map[unit_id].roleid = role_id;
        damage_map[unit_id].damage = 0;
    }
}

bool WorldBoss::IsWorldBossDead() const
{
    return TestWorldBossDead();
}

void WorldBoss::AddBoss(TemplID boss_templ_id, int pos, int32_t hp)
{
	BossVec& list = combat_boss_list_;
	BossVec::iterator it = std::find_if(list.begin(), list.end(), boss_finder(pos));
	if (it != list.end())
	{
		assert(false);
		return;
	}

	BossNode boss;
	boss.tid       = boss_templ_id;
	boss.pos       = pos;
	boss.hp        = hp;
	boss.hp_change = false;
	combat_boss_list_.push_back(boss);
}

bool WorldBoss::CheckBoss(int pos) const
{
	const BossVec& list = combat_boss_list_;
	BossVec::const_iterator it = std::find_if(list.begin(), list.end(), boss_finder(pos));
	return it != list.end();
}

bool WorldBoss::CheckBoss(int pos, TemplID boss_templ_id) const
{
	const BossVec& list = combat_boss_list_;
	BossVec::const_iterator it = std::find_if(list.begin(), list.end(), boss_finder(pos));
	if (it == list.end())
	{
		return false;
	}
	return it->tid == boss_templ_id;
}

int32_t WorldBoss::GetBossHP(int pos) const
{
	const BossVec& list = combat_boss_list_;
	BossVec::const_iterator it = std::find_if(list.begin(), list.end(), boss_finder(pos));
	if (it == list.end())
	{
		assert(false);
		return -1;
	}
	return it->hp;
}

void WorldBoss::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case COMBAT_MSG_BOSS_DAMAGED:
			{
				assert(msg.content_len == sizeof(combat_msg_boss_damaged));
				const combat_msg_boss_damaged* pMsg = (const combat_msg_boss_damaged*)msg.content;
				DoDamage(pMsg->combat_id, pMsg->boss_pos, pMsg->damage, pMsg->attacker);
			}
			break;

		case COMBAT_MSG_BOSS_HEALED:
			{
				assert(msg.content_len == sizeof(combat_msg_boss_healed));
				const combat_msg_boss_healed* pMsg = (const combat_msg_boss_healed*)msg.content;
				DoHeal(pMsg->combat_id, pMsg->boss_pos, pMsg->life);
			}
			break;

		case COMBAT_MSG_BOSS_COMBAT_END:
			{
                MarkCombatEnd(msg.source.id, msg.param);
			}
			break;
       
		default:
			{
				__PRINTF("WorldBoss: 非法消息 message:%d", msg.message);
				assert(false);
			}
			break;
	}
}

void WorldBoss::HeartBeat()
{
	if (!is_combating_)
		return;

	//同步世界BOSS血量
	if (boss_hp_change_)
	{
		SyncCombatBossHP();
		boss_hp_change_ = false;
	}

	//测试是否销毁世界BOSS
	if (some_combat_end_ && TestCombatEnd())
	{
		DoCombatEnd();
	}
	else
	{
		some_combat_end_ = false;
	}
}

void WorldBoss::Trace() const
{
}

void WorldBoss::Clear()
{
	Object::Clear();

	world_boss_object_id_ = 0;
	some_combat_end_      = false;
	boss_hp_change_       = false;
	is_combating_         = false;

	player_map_.clear();
	combat_list_.clear();
	combat_boss_list_.clear();
}

/**************************private func*********************************/
void WorldBoss::DoDamage(ObjectID combat_id, int boss_pos, int32_t damage, UnitID attacker)
{
	if (damage <= 0) return;
	if (!CheckBoss(boss_pos))
	{
		assert(false);
		return;
	}

	BossVec& list = combat_boss_list_;
	BossVec::iterator it_boss = std::find_if(list.begin(), list.end(), boss_finder(boss_pos));
	if (it_boss != list.end())
	{
		//更新BOSS血量
		it_boss->hp -= damage;
		it_boss->hp_change = true;
		if (it_boss->hp < 0)
		{
			it_boss->hp = 0;
		}
		boss_hp_change_ = true;
	}
    
    DamageMap::iterator it_player = player_map_[combat_id].find(attacker);
    if (it_player != player_map_[combat_id].end())
    {
        //更新玩家造成的伤害
        it_player->second.damage += damage;
    }
}

void WorldBoss::DoHeal(ObjectID combat_id, int boss_pos, int32_t life)
{
	if (life <= 0) return;
	if (!CheckBoss(boss_pos))
	{
		assert(false);
		return;
	}

	BossVec& list = combat_boss_list_;
	BossVec::iterator it_boss = std::find_if(list.begin(), list.end(), boss_finder(boss_pos));
	if (it_boss != list.end())
	{
		//更新BOSS血量
		it_boss->hp += life;
		it_boss->hp_change = true;
		boss_hp_change_ = true;
	}
}

void WorldBoss::SyncCombatBossHP()
{
	//同步BOSS血量给其它战场
	for (size_t i = 0; i < combat_boss_list_.size(); ++ i)
	{
		BossNode& node = combat_boss_list_[i];
		if (!node.hp_change) continue;

		for (size_t j = 0; j < combat_list_.size(); ++ j)
		{
			const CombatNode& target = combat_list_[j];
			if (target.running)
			{
				Combat* combat = s_pCombatMan->QueryCombat(target.combat_id);
				if (!combat)
				{
					assert(false && "出错了,找不到战斗!!!");
					continue;
				}

				if (!combat->IsActived())
				{
					__PRINTF("战斗提前结束了!!!");
					continue;
				}

				shared::MutexLockGuard keeper(combat->GetMutex());
				if (!combat->IsActived())
				{
					assert(false && "出错了,战斗结束了!!!");
					continue;
				}

				CombatPVE* pve_combat = dynamic_cast<CombatPVE*>(combat);
				CombatUnit* unit = pve_combat->GetBoss(node.pos);
				if (!unit || !unit->IsActived())
				{
					assert(false && "出错了,找不到BOSS!!!");
					continue;
				}

				CombatNpc* boss = dynamic_cast<CombatNpc*>(unit);
				boss->SyncBossHP(node.hp);
			}
		}

		node.hp_change = false;
	}
}

void WorldBoss::MarkCombatEnd(ObjectID combat_id, int result)
{
	for (size_t i = 0; i < combat_list_.size(); ++ i)
	{
		if (combat_list_[i].combat_id == combat_id)
		{
			some_combat_end_        = true;
			combat_list_[i].running = false;
			combat_list_[i].result  = result;
            SyncWorldBossStatus(combat_id);
            return;
		}
	}

	assert(false && "战场不属于该世界BOSS");
}

void WorldBoss::SyncWorldBossStatus(ObjectID combat_id)
{
    WorldBossCombatStatus result;
	result.status = WBST_BOSS_LOSE;

    PlayerMap::iterator it_player = player_map_.find(combat_id);
    if (it_player == player_map_.end())
        return;

    DamageMap::iterator it_dam = it_player->second.begin();
    for (; it_dam != it_player->second.end(); ++it_dam)
    {
        WorldBossCombatStatus::DamageEntry ent;
        ent.roleid = it_dam->second.roleid;
        ent.damage = it_dam->second.damage;
        result.dmg_list.push_back(ent);
    }
    player_map_.erase(it_player);
    
    if (!TestWorldBossDead())
    {
        result.status = WBST_BOSS_WIN;
    }

	//同步BOSS战斗结果给大世界
	Combat::WorldBossStatusCB(world_boss_object_id_, result);
}

bool WorldBoss::TestCombatEnd() const
{
	bool combat_end = true;
	for (size_t i = 0; i < combat_list_.size(); ++ i)
	{
		if (combat_list_[i].running)
		{
			combat_end = false;
			break;
		}
	}
	return combat_end;
}

bool WorldBoss::TestWorldBossDead() const
{
	bool world_boss_dead = true;
	for (size_t i = 0; i < combat_boss_list_.size(); ++ i)
	{
		if (combat_boss_list_[i].hp > 0)
		{
			world_boss_dead = false;
			break;
		}
	}
	return world_boss_dead;
}

void WorldBoss::DoCombatEnd()
{
    WorldBossCombatStatus result;
    int32_t boss_id = world_boss_object_id_;

    if (TestWorldBossDead())
    {
        //世界BOSS死亡，销毁自己
        result.status = WBST_BOSS_DEAD;
        s_pCombatMan->RemoveWorldBossFromMan(this);
        s_pCombatMan->FreeWorldBoss(this);
    }
    else
    {
        //世界BOSS未死亡，战斗暂时结束
        result.status    = WBST_BOSS_ALIVE;
        boss_hp_change_  = false;
        some_combat_end_ = false;
        is_combating_    = false;
        player_map_.clear();
        combat_list_.clear();
    }

    //同步BOSS战斗结果给大世界
    Combat::WorldBossStatusCB(boss_id, result);
}

} // namespace combat

