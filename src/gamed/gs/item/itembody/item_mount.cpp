#include "item_mount.h"

#include "gs/player/player.h"
#include "gs/template/data_templ/mount_templ.h"
#include "gs/template/data_templ/templ_manager.h"

namespace gamed
{

void ItemMount::Initialize(const dataTempl::ItemDataTempl* tpl)
{
    templ = dynamic_cast<const dataTempl::MountTempl*>(tpl);
}

void ItemMount::OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//向坐骑子系统注册
    Player* player = dynamic_cast<Player*>(obj);
    player->RegisterMount(index, templ->templ_id);
}

void ItemMount::OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	//玩家失去坐骑
	Player* player = dynamic_cast<Player*>(obj);
	player->UnRegisterMount(index);
}

bool ItemMount::DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
    return true;
}

} // namespace gamed
