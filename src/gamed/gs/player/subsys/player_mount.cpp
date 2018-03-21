#include "player_mount.h"

#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/template/data_templ/mount_templ.h"
#include "gs/template/data_templ/mount_equip_templ.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"

namespace gamed
{

using namespace std;
using namespace dataTempl;

#define GetMountTempl(id) s_pDataTempl->QueryDataTempl<MountTempl>(id)
#define GetMountEquipTempl(id) s_pDataTempl->QueryDataTempl<MountEquipTempl>(id)

PlayerMount::PlayerMount(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_MOUNT, player)
{
	SAVE_LOAD_REGISTER(common::PlayerMountData, PlayerMount::SaveToDB, PlayerMount::LoadFromDB);
}

PlayerMount::~PlayerMount()
{
}

void PlayerMount::NotifyMountChange(int32_t mount_id)
{
    int64_t visible_mask = G2C::PlayerVisibleInfoChange::VM_MOUNT;
    std::vector<int32_t> visible_list;
    visible_list.push_back(mount_id);
    player_.sender()->PlayerVisibleInfoChange(visible_mask, visible_list);
}

bool PlayerMount::SaveToDB(common::PlayerMountData* pData)
{
    pData->mount_index = mount_index_;
    pData->mount_equip_level.clear();
    pData->mount_equip_level.resize(mount_equip_level_.size());
    for (size_t i = 0; i < mount_equip_level_.size(); ++i)
    {
        pData->mount_equip_level[i] = mount_equip_level_[i];
    }
    return true;
}

static void ActivateMountEquip(Player& player, const MountEquipTempl* templ, int32_t cur_level, bool load)
{
    if (templ == NULL)
    {
        return;
    }
    for (size_t i = 0; i < templ->prop.size(); ++i)
    {
        const MountEquipProp& prop = templ->prop[i];
        int32_t prop_delta = prop.prop_base;
        if (cur_level < (int32_t)prop.prop_multi.size())
        {
            if (load)
            {
                prop_delta *= prop.prop_multi[cur_level] / 1000.0;
                prop_delta += prop.prop_add[cur_level];
            }
            else
            {
                if (cur_level == 0)
                {
                    prop_delta *= prop.prop_multi[cur_level] / 1000.0;
                    prop_delta += prop.prop_add[cur_level];
                }
                else
                {
                    prop_delta *= (prop.prop_multi[cur_level] - prop.prop_multi[cur_level - 1]) / 1000.0;
                    prop_delta += prop.prop_add[cur_level] - prop.prop_add[cur_level - 1];
                }
            }
        }
        if (prop_delta > 0)
        {
            player.IncPropPoint(prop.prop_index, prop_delta);
        }
        else
        {
            player.DecPropPoint(prop.prop_index, -1 * prop_delta);
        }
    }
}

bool PlayerMount::LoadFromDB(const common::PlayerMountData& data)
{
    const GlobalConfigTempl* global = s_pDataTempl->QueryGlobalConfigTempl();
    mount_equip_level_.resize(global->mount_config.kMaxMountEquipNum);
    mount_index_ = data.mount_index;
    for (size_t i = 0; i < mount_equip_level_.size(); ++i)
    {
        mount_equip_level_[i] = i < data.mount_equip_level.size() ? data.mount_equip_level[i] : -1;
    }
    // 让坐骑跟骑具生效
    if (mount_index_ != -1 && mount_inv_.find(mount_index_) == mount_inv_.end())
    {
        mount_index_ = -1;
    }
    if (mount_index_ != -1)
    {
        player_.IncSpeed(mount_inv_[mount_index_]->move_speed_delta);
        player_.visible_state().SetMountFlag(mount_inv_[mount_index_]->templ_id);
    }
    for (size_t i = 0; i < mount_equip_level_.size(); ++i)
    {
        int32_t cur_level = mount_equip_level_[i];
        if (cur_level == -1)
        {
            continue;
        }
        int32_t equip_id = global->mount_config.mount_equip_list[i];
        ActivateMountEquip(player_, GetMountEquipTempl(equip_id), cur_level, true);
    }
    return true;
}

void PlayerMount::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::MountMount, PlayerMount::CMDHandler_MountMount);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MountExchange, PlayerMount::CMDHandler_MountExchange);
	REGISTER_NORMAL_CMD_HANDLER(C2G::MountEquipLevelUp, PlayerMount::CMDHandler_MountEquipLevelUp);
}

void PlayerMount::PlayerGetMountData() const
{
    G2C::MountData packet;
    packet.mount_index = mount_index_;
    packet.mount_equip_level = mount_equip_level_;
    player_.sender()->SendCmd(packet);
}

void PlayerMount::RegisterMount(int32_t item_index, int32_t item_id)
{
    const MountTempl* templ = GetMountTempl(item_id);
    if (templ != NULL)
    {
        mount_inv_[item_index] = templ;
        ++mount_category_[templ->templ_id];
        G2C::GainMount packet;
        packet.mount_index = item_index;
        player_.sender()->SendCmd(packet);
    }
}

void PlayerMount::UnRegisterMount(int32_t item_index)
{
    const dataTempl::MountTempl* templ = mount_inv_[item_index];
    --mount_category_[templ->templ_id];
    if (mount_category_[templ->templ_id] == 0)
    {
        mount_category_.erase(templ->templ_id);
    }
    if (mount_index_ == item_index)
    {
        if (mount_index_ != -1)
        {
            player_.DecSpeed(mount_inv_[item_index]->move_speed_delta);
        }
        mount_index_ = -1;
        player_.visible_state().SetMountFlag(0);
        NotifyMountChange(0);
    }
    mount_inv_.erase(item_index);
    G2C::LostMount packet;
    packet.mount_index = item_index;
    player_.sender()->SendCmd(packet);
}

void PlayerMount::OpenMountEquip(int32_t equip_index)
{
    if (equip_index < 0 || equip_index >= (int32_t)mount_equip_level_.size())
    {
        return;
    }
    int32_t& cur_level = mount_equip_level_[equip_index];
    if (cur_level != -1)
    {
        return;
    }
    cur_level = 0;
    G2C::OpenMountEquip packet;
    packet.equip_index = equip_index;
    player_.sender()->SendCmd(packet);
}

void PlayerMount::CMDHandler_MountMount(const C2G::MountMount& cmd)
{
    if (cmd.item_index == mount_index_)
    {
        return;
    }
    G2C::MountMount_Re reply;
    reply.item_index = cmd.item_index;
    reply.item_id = cmd.item_id;
    MountMap::const_iterator it = mount_inv_.find(cmd.item_index);
    if (cmd.item_index != -1 && (it == mount_inv_.end() || it->second->templ_id != cmd.item_id))
    {
        reply.result = 1;
    }
    else
    {
        reply.result = 0;
        if (mount_index_ != -1)
        {
            player_.DecSpeed(mount_inv_[mount_index_]->move_speed_delta);
        }
        mount_index_ = cmd.item_index;
        if (mount_index_ != -1)
        {
            player_.IncSpeed(mount_inv_[mount_index_]->move_speed_delta);
        }
        int32_t mount_id = cmd.item_index == -1 ? 0 : mount_inv_[mount_index_]->templ_id;
        player_.visible_state().SetMountFlag(mount_id);
        NotifyMountChange(mount_id);
    }
    player_.sender()->SendCmd(reply);
}

void PlayerMount::CMDHandler_MountExchange(const C2G::MountExchange& cmd)
{
    const GlobalConfigTempl* global = s_pDataTempl->QueryGlobalConfigTempl();
    if (cmd.config_index < 0 || cmd.config_index > (int32_t)global->mount_config.mount_list.size())
    {
        return;
    }

    const MountVisibleInfo& info = global->mount_config.mount_list[cmd.config_index];
    if (!info.visible)
    {
        return;
    }

    const MountTempl* templ = GetMountTempl(info.mount_id);
    if (templ->get_method != MOUNT_GET_EXCHANGE)
    {
        return;
    }

    G2C::MountExchange_Re reply;
    if (!player_.CheckItem(templ->exchange_item_id, templ->exchange_item_count))
    {
        reply.result = 1;
    }
    else
    {
        reply.result = 0;
        player_.TakeOutItem(templ->exchange_item_id, templ->exchange_item_count);
        player_.GainItem(info.mount_id, 1);
    }
    player_.sender()->SendCmd(reply);
}

void PlayerMount::CMDHandler_MountEquipLevelUp(const C2G::MountEquipLevelUp& cmd)
{
    if (cmd.equip_index < 0 || cmd.equip_index >= (int32_t)mount_equip_level_.size())
    {
        return;
    }
    const GlobalConfigTempl* global = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t mount_equip_id = global->mount_config.mount_equip_list[cmd.equip_index];
    const MountEquipTempl* templ = GetMountEquipTempl(mount_equip_id);
    if (templ == NULL)
    {
        return;
    }
    int32_t& cur_level = mount_equip_level_[cmd.equip_index];
    if (cur_level == -1 || cur_level > player_.level() || cur_level >= (int32_t)templ->lvlup.size())
    {
        return;
    }

    G2C::MountEquipLevelUp_Re reply;
    reply.result = 0;
    reply.equip_index = cmd.equip_index;
    const MountEquipLevelUp& lvlup = templ->lvlup[cur_level];
    if ((lvlup.item_id != 0 && !player_.CheckItem(lvlup.item_id, lvlup.item_count)) ||
        (lvlup.money != 0 && !player_.CheckMoney(lvlup.money)))
    {
        reply.result = 1;
        player_.sender()->SendCmd(reply);
        return;
    }
    if (lvlup.item_id != 0)
    {
        player_.TakeOutItem(lvlup.item_id, lvlup.item_count);
    }
    if (lvlup.money != 0)
    {
        player_.SpendMoney(lvlup.money);
    }
    if (lvlup.score != 0)
    {
        player_.SpendScore(lvlup.score);
    }
    ActivateMountEquip(player_, templ, cur_level, false);
    cur_level += 1;
    player_.sender()->SendCmd(reply);
}

int32_t PlayerMount::GetMountCategory() const
{
    return mount_category_.size();
}

int32_t PlayerMount::GetMountEquipLevel(int32_t index) const
{
    if (index < 0 || index >= (int32_t)mount_equip_level_.size())
    {
        return -1;
    }
    return mount_equip_level_[index];        
}

} // namespace gamed
