#include "item_pet.h"

#include "gs/player/player.h"
#include "gs/template/data_templ/pet_item_templ.h"

namespace gamed
{

void ItemPet::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	const dataTempl::PetItemTempl* pTpl = dynamic_cast<const dataTempl::PetItemTempl*>(tpl);
	if (pTpl)
	{
		data.pet_id = pTpl->pet_id;
		data.pet_level = pTpl->pet_level;
		data.pet_blevel = pTpl->pet_blevel;
	}
}

void ItemPet::OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    ItemEssence& item_ess = parent->GetItemEssence();
    EssPetProp* pEss = item_ess.QueryEss<EssPetProp>(ESS_ID_PET_PROP);
    if (pEss == NULL)
    {
		//玩家游戏中获得宠物物品
		//为宠物物品生成动态属性
        pEss = new EssPetProp;
        pEss->pet_exp = 0;
        pEss->pet_level = data.pet_level;
        pEss->pet_blevel = data.pet_blevel;
        item_ess.InsertEss(pEss);
		//玩家获得宠物
		Player* player = dynamic_cast<Player*>(obj);
		player->GainPet(parent->Type(), index);
		return;
    }
	playerdef::PetEntry pet;
	pet.pet_id        = data.pet_id;
	pet.pet_exp       = pEss->pet_exp;
	pet.pet_level     = pEss->pet_level;
	pet.pet_blevel    = pEss->pet_blevel;
	pet.pet_item_idx  = index;

	//向宠物子系统注册
	Player* player = dynamic_cast<Player*>(obj);
	player->RegisterPet(pet);
}

void ItemPet::OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    ItemEssence& item_ess = parent->GetItemEssence();
    EssPetProp* pEss = item_ess.QueryEss<EssPetProp>(ESS_ID_PET_PROP);
    if (pEss == NULL)
    {
        return;
    }
	//玩家失去宠物
    Player* player = dynamic_cast<Player*>(obj);
    player->UnRegisterPet(index);
}

bool ItemPet::DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	///
	/// 宠物子系统的宠物数据更新，同步更新宠物包裹中对应的宠物物品
	///

	//从宠物子系统获取宠物信息
	playerdef::PetEntry pet;
	Player* player = dynamic_cast<Player*>(obj);
	if (!player->QueryPetInfo(index, pet))
		return false;

	if (pet.pet_id != data.pet_id)
		return false;

    ItemEssence& item_ess = parent->GetItemEssence();
    EssPetProp* pEss = item_ess.QueryEss<EssPetProp>(ESS_ID_PET_PROP);
    if (pEss == NULL)
    {
        return false;
    }

	//更新宠物属性
	pEss->pet_exp       = pet.pet_exp;
	pEss->pet_level     = pet.pet_level;
	pEss->pet_blevel    = pet.pet_blevel;
	return true;
}

};
