#ifndef __GAMED_GS_ITEM_BODY_ITEM_TEAM_NPC_H__
#define __GAMED_GS_ITEM_BODY_ITEM_TEAM_NPC_H__

#include "gs/item/item.h"


namespace gamed {

/**
 * @brief 组队NPC
 */
class ItemTeamNpc : public ItemBody
{
    struct team_npc_data
    {
        int32_t npc_id;
    };

public:
	explicit ItemTeamNpc(int id)
        : ItemBody(id)
	{ }

	virtual ~ItemTeamNpc()
	{ }

	virtual void Initialize(const dataTempl::ItemDataTempl* tpl);
	virtual void OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
	virtual void OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;

private:
	team_npc_data data_;
};

} // namespace gamed

#endif // __GAMED_GS_ITEM_BODY_ITEM_TEAM_NPC_H__
