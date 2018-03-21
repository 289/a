#include "item_talent.h"

#include "gs/template/data_templ/talent_item_templ.h"
#include "gs/player/player.h"
#include "gs/player/player_sender.h"

namespace gamed
{

void ItemTalent::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	ASSERT(tpl);
	const dataTempl::TalentItemTempl* pTpl = dynamic_cast<const dataTempl::TalentItemTempl*>(tpl);

    data.item_func_type = pTpl->item_func_type;
    data.talent_group_id = pTpl->talent_group_id;
    data.talent_id = pTpl->talent_id;
}

bool ItemTalent::TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	Player* player = dynamic_cast<Player*>(obj);

    if (data.item_func_type == dataTempl::TalentItemTempl::IFT_OPEN_TALENT_GROUP)
    {
        return player->HasTalentGroup(data.talent_group_id);
    }
    else if (data.item_func_type == dataTempl::TalentItemTempl::IFT_OPEN_TALENT)
    {
        return player->HasTalent(data.talent_group_id, data.talent_id);
    }

	return true;
}

Item::ITEM_USE ItemTalent::OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	Player* player = dynamic_cast<Player*>(obj);

    if (data.item_func_type == dataTempl::TalentItemTempl::IFT_OPEN_TALENT_GROUP)
    {
        player->OpenTalentGroup(data.talent_group_id);
    }
    else if (data.item_func_type == dataTempl::TalentItemTempl::IFT_OPEN_TALENT)
    {
        player->OpenTalent(data.talent_group_id, data.talent_id);
    }

	return Item::ITEM_USE_CONSUME; //返回表示使用后销毁
}

};
