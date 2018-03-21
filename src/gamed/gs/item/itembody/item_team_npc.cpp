#include "item_team_npc.h"
#include "gs/player/player.h"
#include "gs/template/data_templ/hidden_item_templ.h"

namespace gamed
{

void ItemTeamNpc::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	ASSERT(tpl->GetType() == dataTempl::TEMPL_TYPE_HIDDEN_ITEM);
	const dataTempl::HiddenItemTempl* pTpl = dynamic_cast<const dataTempl::HiddenItemTempl*>(tpl);
	ASSERT(pTpl->type == dataTempl::HiddenItemTempl::HIT_TEAM_NPC);

	int64_t monster_id = 0;
	dataTempl::HiddenItemTempl::PopParamGuard guard(*pTpl);
	if (guard.PopParam(monster_id))
	{
		data_.npc_id = monster_id;
	}
	else
	{
		ASSERT(false);
	}
}

void ItemTeamNpc::OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//玩家获得组队NPC
	Player* player = dynamic_cast<Player*>(obj);
	player->ObtainBuddy(data_.npc_id);
}

void ItemTeamNpc::OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//玩家失去组队NPC
	Player* player = dynamic_cast<Player*>(obj);
	player->TakeoutBuddy(data_.npc_id);
}

};
