#include "player_enhance.h"

#include "gs/global/dbgprt.h"
#include "gs/player/subsys_if.h"
#include "gs/global/randomgen.h"
#include "gs/template/data_templ/enhance_templ.h"
#include "gs/template/data_templ/enhance_group_templ.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/player/player_sender.h"


namespace gamed
{

using namespace std;
using namespace shared::net;
using namespace common::protocol;
using namespace dataTempl;

#define GetEnhanceTempl(id) s_pDataTempl->QueryDataTempl<EnhanceTempl>(id)
#define GetEnhanceGroupTempl(id) s_pDataTempl->QueryDataTempl<EnhanceGroupTempl>(id)

size_t PlayerEnhance::GetMaxSlotNum(int32_t mode)
{
    static const size_t kMaxCashEnhanceNum = 5;
    static const size_t kMaxLevelEnhanceNum = 6;
    static const size_t kMaxVIPEnhanceNum = 5;

    switch (mode)
    {
    case ENHANCE_OPEN_CASH:
        return kMaxCashEnhanceNum;
    case ENHANCE_OPEN_LEVEL:
        return kMaxLevelEnhanceNum;
    case ENHANCE_OPEN_VIP:
        return kMaxVIPEnhanceNum;
    default:
        return 0;
    }
}

PlayerEnhance::PlayerEnhance(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_ENHANCE, player)
{
	SAVE_LOAD_REGISTER(common::PlayerEnhanceData, PlayerEnhance::SaveToDB, PlayerEnhance::LoadFromDB);
}

PlayerEnhance::~PlayerEnhance()
{
}

static void EnhanceAward(Player& player, int32_t enhance_id, bool add)
{
    const EnhanceTempl* templ = GetEnhanceTempl(enhance_id);
    if (templ == NULL)
    {
        return;
    }
    if (templ->enhance_type == ENHANCE_TYPE_PROP && add)
    {
        player.IncPropPoint(templ->enhance_param[0], templ->enhance_param[1]);
    }
    else if (templ->enhance_type == ENHANCE_TYPE_PROP && !add)
    {
        player.DecPropPoint(templ->enhance_param[0], templ->enhance_param[1]);
    }
    else if (templ->enhance_type == ENHANCE_TYPE_SKILL_LEVEL)
    {
        int32_t param2 = add ? templ->enhance_param[0] : -1*templ->enhance_param[0];
        size_t size = templ->enhance_param.size();
        if (size == 0)
        {
            player.LevelUpAllSkill(param2, LVLUP_MODE_ENHANCE);
        }
        else
        {
            for (size_t i = 1; i < size; ++i)
            {
                player.LevelUpSkill(templ->enhance_param[i], param2, LVLUP_MODE_ENHANCE);
            }
        }
    }
}

bool PlayerEnhance::LoadFromDB(const common::PlayerEnhanceData& data)
{
    enhance_list_.clear();
    for (size_t i = 0; i < data.enhance_list.size(); ++i)
    {
        G2C::enhance_entry entry;
        entry.open_mode = data.enhance_list[i].open_mode;
        entry.slot_status = data.enhance_list[i].slot_status;
        entry.enhance_id = data.enhance_list[i].enhance_id;
        enhance_list_.push_back(entry);
        EnhanceAward(player_, entry.enhance_id, true);
    }

	return true;
}

bool PlayerEnhance::SaveToDB(common::PlayerEnhanceData* pData)
{
    pData->enhance_list.clear();
    pData->enhance_list.resize(enhance_list_.size());
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        pData->enhance_list[i].open_mode = enhance_list_[i].open_mode;
        pData->enhance_list[i].slot_status = enhance_list_[i].slot_status;
        pData->enhance_list[i].enhance_id = enhance_list_[i].enhance_id;
    }
	return true;
}

void PlayerEnhance::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::OpenEnhanceSlot, PlayerEnhance::CMDHandler_OpenEnhanceSlot);
	REGISTER_NORMAL_CMD_HANDLER(C2G::ProtectEnhanceSlot, PlayerEnhance::CMDHandler_ProtectEnhanceSlot);
	REGISTER_NORMAL_CMD_HANDLER(C2G::UnProtectEnhanceSlot, PlayerEnhance::CMDHandler_UnProtectEnhanceSlot);
}

void PlayerEnhance::RegisterMsgHandler()
{
}

void PlayerEnhance::PlayerGetEnhanceData() const
{
    G2C::EnhanceData packet;
    packet.enhance_list = enhance_list_;
    player_.sender()->SendCmd(packet);
}

size_t PlayerEnhance::GetSlotNum(int32_t mode) const
{
    size_t num = 0;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        if (enhance_list_[i].open_mode == mode)
        {
            ++num;
        }
    }
    return num;
}

void PlayerEnhance::OpenEnhanceSlot(int8_t mode)
{
    if (enhance_list_.size() == kMaxEnhanceNum || GetSlotNum(mode) == GetMaxSlotNum(mode))
    {
        return;
    }

    int8_t slot_index = enhance_list_.size();
    if (mode == ENHANCE_OPEN_CASH)
    {
        const GlobalConfigTempl* pTpl = s_pDataTempl->QueryGlobalConfigTempl();
        int32_t price = pTpl->enhance_config.open_price * (1 + 0.5 * cash_slot_num());
        if (price <= player_.GetCash())
        {
            player_.UseCash(price);
        }
        else
        {
            slot_index = -1;
        }
    }

    G2C::OpenEnhanceSlotRe packet;
    packet.slot_index = slot_index;
    packet.new_slot.open_mode = mode;
    if (slot_index != -1)
    {
        enhance_list_.push_back(packet.new_slot);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerEnhance::ProtectEnhanceSlot(int8_t slot_index)
{
    if ((size_t)slot_index >= enhance_list_.size())
    {
        return;
    }
    G2C::enhance_entry& entry = enhance_list_[slot_index];
    if (entry.enhance_id == 0 || entry.slot_status != ENHANCE_SLOT_NORMAL)
    {
        return;
    }
    const EnhanceTempl* templ = GetEnhanceTempl(entry.enhance_id);
    if (player_.GetCash() >= templ->lock_price)
    {
        player_.UseCash(templ->lock_price);
        entry.slot_status = ENHANCE_SLOT_PROTECTED;
    }
    else
    {
        slot_index = -1;
    }

    G2C::ProtectEnhanceSlotRe reply;
    reply.slot_index = slot_index;
    player_.sender()->SendCmd(reply);
}

void PlayerEnhance::UnProtectEnhanceSlot(int8_t slot_index)
{
    if ((size_t)slot_index >= enhance_list_.size())
    {
        return;
    }
    G2C::enhance_entry& entry = enhance_list_[slot_index];
    if (entry.slot_status == ENHANCE_SLOT_PROTECTED)
    {
        entry.slot_status = ENHANCE_SLOT_NORMAL;
        G2C::UnProtectEnhanceSlotRe reply;
        reply.slot_index = slot_index;
        player_.sender()->SendCmd(reply);
    }
}

int32_t PlayerEnhance::RandSelectEnhanceId(int32_t enhance_gid) const
{
    // 选出附魔库中所有可用的附魔ID
    const EnhanceGroupTempl* gtempl = GetEnhanceGroupTempl(enhance_gid);
    if (gtempl == NULL)
    {
        return 0;
    }
    size_t i = 0, j = 0;
    int32_t total_weight = 0, cur_weight = 0;
    map<int32_t, int32_t> valid_enhance;
    for (i = 0; i < gtempl->enhance_list.size(); ++i)
    {
        const EnhanceEntry& entry = gtempl->enhance_list[i];
        for (j = 0; j < enhance_list_.size(); ++j)
        {
            // 相同互斥组的不能同时出现
            const EnhanceTempl* new_tpl = GetEnhanceTempl(entry.enhance_id);
            const EnhanceTempl* old_tpl = GetEnhanceTempl(enhance_list_[j].enhance_id);
            // 只有原附魔位不为空，并且新旧的互斥组相同并且不是-1时才不能同时存在
            if (old_tpl != NULL && old_tpl->mutex_gid != -1 && new_tpl->mutex_gid != -1 && new_tpl->mutex_gid == old_tpl->mutex_gid)
            {
                break;
            }
        }
        if (j == enhance_list_.size())
        {
            valid_enhance[entry.enhance_id] = entry.enhance_weight;
            total_weight += entry.enhance_weight;
        }
    }
    // 按权重随机选择本次附魔的ID
    int32_t result = mrand::Rand(0, total_weight);
    map<int32_t, int32_t>::const_iterator vit = valid_enhance.begin();
    for (; vit != valid_enhance.end(); ++vit)
    {
        cur_weight += vit->second;
        if (cur_weight > result)
        {
            return vit->first;
        }
    }
    return 0;
}

size_t PlayerEnhance::GetUnProtectSlot(vector<int32_t>& slot_vec) const
{
    // 统计空的以及未保护的附魔位
    size_t empty_slot = kMaxEnhanceNum;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        if (empty_slot == kMaxEnhanceNum && enhance_list_[i].enhance_id == 0)
        {
            empty_slot = i;
        }
        if (enhance_list_[i].slot_status == ENHANCE_SLOT_NORMAL)
        {
            slot_vec.push_back(i);
        }
    }
    return empty_slot;
}

int32_t PlayerEnhance::GetProtectSlotNum() const
{
    int32_t num = 0;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        const G2C::enhance_entry& entry = enhance_list_[i];
        if (entry.slot_status == ENHANCE_SLOT_PROTECTED && entry.enhance_id != 0)
        {
            ++num;
        }
    }
    return num;
}

int32_t PlayerEnhance::GetUnProtectSlotNum() const
{
    vector<int32_t> unprotect_slot;
    GetUnProtectSlot(unprotect_slot);
    return unprotect_slot.size();
}

void PlayerEnhance::DoEnhance(int32_t enhance_gid, int32_t count)
{
    vector<int32_t> unprotect_slot;
    int32_t enhance_slot = GetUnProtectSlot(unprotect_slot);
    // 如果所有位置都被保护则无法附魔
    if (unprotect_slot.empty())
    {
        return;
    }

    int32_t enhance_id = RandSelectEnhanceId(enhance_gid);
    if (enhance_id == 0) // 没有可用的附魔ID
    {
        return;
    }
    if (enhance_slot == (int32_t)kMaxEnhanceNum)
    {
        enhance_slot = unprotect_slot[mrand::Rand(0, unprotect_slot.size() - 1)];
    }

    EnhanceAward(player_, enhance_list_[enhance_slot].enhance_id, false);
    EnhanceAward(player_, enhance_id, true);
    enhance_list_[enhance_slot].enhance_id = enhance_id;

    player_.sender()->EnhanceReply(0, enhance_slot, enhance_id, count);
}

void PlayerEnhance::CMDHandler_OpenEnhanceSlot(const C2G::OpenEnhanceSlot& packet)
{
    OpenEnhanceSlot(ENHANCE_OPEN_CASH);
}

void PlayerEnhance::CMDHandler_ProtectEnhanceSlot(const C2G::ProtectEnhanceSlot& packet)
{
    ProtectEnhanceSlot(packet.slot_index);
}

void PlayerEnhance::CMDHandler_UnProtectEnhanceSlot(const C2G::UnProtectEnhanceSlot& packet)
{
    UnProtectEnhanceSlot(packet.slot_index);
}

void PlayerEnhance::QueryEnhanceSkill(std::set<int32_t>& skills) const
{
    const EnhanceTempl* templ = NULL;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        templ = GetEnhanceTempl(enhance_list_[i].enhance_id);
        if (templ != NULL && templ->enhance_type == ENHANCE_TYPE_PASSIVE_SKILL)
        {
            skills.insert(templ->enhance_param[0]);
        }
    }
}

int32_t PlayerEnhance::GetEnhanceNum(int32_t level, int32_t rank) const
{
    int32_t num = 0;
    const EnhanceTempl* templ = NULL;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        templ = GetEnhanceTempl(enhance_list_[i].enhance_id);
        if (templ != NULL && templ->level >= level && templ->quality >= rank)
        {
            ++num;
        }
    }
    return num;
}

} // namespace gamed
