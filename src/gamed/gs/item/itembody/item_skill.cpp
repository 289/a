#include "item_skill.h"

#include "gs/player/player.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/skill_item_templ.h"
#include "gs/template/data_templ/cooldown_group.h"


namespace gamed {

using namespace dataTempl;

void ItemSkill::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	pSkill = dynamic_cast<const dataTempl::SkillItemTempl*>(tpl);
    int32_t cd_group_id = pSkill->cooldown_data.cd_group_id;
    if (cd_group_id > 0)
    {
        pCD = s_pDataTempl->QueryDataTempl<const CoolDownGroupTempl>(cd_group_id);
    }
}

bool ItemSkill::TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    Player* player = dynamic_cast<Player*>(obj);
    if (player->GetLevel() < pSkill->cls_level_limit)
        return false;

    if (!player->CheckMoney(pSkill->money_need_on_use))
        return false;

    if (!player->CheckCash(pSkill->cash_need_on_use))
        return false;

    if (pSkill->item_need_on_use > 0 && pSkill->item_count_on_use > 0 && !player->CheckItem(pSkill->item_need_on_use, pSkill->item_count_on_use))
        return false;

    if (!player->TestItemCoolDown(pSkill->cooldown_data.cd_group_id, parent->Type()))
        return false;

    return true;
}

Item::ITEM_USE ItemSkill::OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    Player* player = dynamic_cast<Player*>(obj);

    //先设置冷却，成功与否都要设置冷却
    if (pSkill->cooldown_data.cd_group_id > 0)
    {
        int32_t cd_group_time = pCD == NULL ? 0 : pCD->cd_group_time;
        player->SetCoolDown(pSkill->cooldown_data.cd_group_id, cd_group_time);
    }
    if (pSkill->cooldown_data.cd_time > 0)
    {
        player->SetCoolDown(parent->Type(), pSkill->cooldown_data.cd_time);
    }

    //添加BUFF
    for (size_t i = 0; i < pSkill->buffs_addon.size(); ++ i)
    {
        player->ModifyFilterByID(pSkill->buffs_addon[i], true);
    }

    //使用成功
    player->SpendMoney(pSkill->money_need_on_use);
    player->UseCash(pSkill->cash_need_on_use);
    player->TakeOutItem(pSkill->item_need_on_use, pSkill->item_count_on_use);
    
    return pSkill->consume_on_use ? Item::ITEM_USE_CONSUME : Item::ITEM_USE_RETAIN;
}

};
