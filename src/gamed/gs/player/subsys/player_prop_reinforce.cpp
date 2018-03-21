#include "player_prop_reinforce.h"

#include "common/obj_data/gen/player/prop_reinforce.pb.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/global_config.h"
#include "gs/global/glogger.h"
#include "gs/global/game_util.h"
#include "gs/global/timer.h"
#include "gs/global/dbgprt.h"
#include "gs/global/randomgen.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

namespace {

    static const size_t kMaxUpgradeCount = 4;
    SHARED_STATIC_ASSERT(kMaxUpgradeCount == dataTempl::PrimaryPropReinforceConfig::kMaxUpgradeCount);

} // namespace Anonymous

// 必须和协议的enum一致
SHARED_STATIC_ASSERT((int)G2C::PPRFI_HP    == (int)PlayerPropReinforce::PPRFI_HP);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_P_ATK == (int)PlayerPropReinforce::PPRFI_P_ATK);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_M_ATK == (int)PlayerPropReinforce::PPRFI_M_ATK);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_P_DEF == (int)PlayerPropReinforce::PPRFI_P_DEF);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_M_DEF == (int)PlayerPropReinforce::PPRFI_M_DEF);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_P_PIE == (int)PlayerPropReinforce::PPRFI_P_PIE);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_M_PIE == (int)PlayerPropReinforce::PPRFI_M_PIE);
SHARED_STATIC_ASSERT((int)G2C::PPRFI_MAX   == (int)PlayerPropReinforce::PPRFI_MAX);

// 必须和模板的enum一致
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_HP    == (int)PlayerPropReinforce::PPRFI_HP);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_P_ATK == (int)PlayerPropReinforce::PPRFI_P_ATK);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_M_ATK == (int)PlayerPropReinforce::PPRFI_M_ATK);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_P_DEF == (int)PlayerPropReinforce::PPRFI_P_DEF);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_M_DEF == (int)PlayerPropReinforce::PPRFI_M_DEF);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_P_PIE == (int)PlayerPropReinforce::PPRFI_P_PIE);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_M_PIE == (int)PlayerPropReinforce::PPRFI_M_PIE);
SHARED_STATIC_ASSERT((int)dataTempl::PPRFI_MAX   == (int)PlayerPropReinforce::PPRFI_MAX);


///
/// PlayerPropReinforce
///
PlayerPropReinforce::PlayerPropReinforce(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_PROP_REINFORCE, player)
{
    SAVE_LOAD_REGISTER(common::PlayerPropRFData, PlayerPropReinforce::SaveToDB, PlayerPropReinforce::LoadFromDB);
}

PlayerPropReinforce::~PlayerPropReinforce()
{
}

bool PlayerPropReinforce::SaveToDB(common::PlayerPropRFData* pdata)
{
    common::scalable::PrimaryPropRFData scalable_data;
    scalable_data.set_last_recover_time(primary_prop_rf_.last_recover_time);
    scalable_data.set_cur_energy(primary_prop_rf_.cur_energy);
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        common::scalable::PrimaryPropRFData::Detail* pdetail = scalable_data.add_detail();
        pdetail->set_cur_add_energy(primary_prop_rf_.detail[i].cur_add_energy);
        pdetail->set_cur_level(primary_prop_rf_.detail[i].cur_level);
    }

    pdata->primary_rf_data.clear();
    std::string& bufRef = pdata->primary_rf_data;
    bufRef.resize(scalable_data.ByteSize());
    if (!scalable_data.SerializeToArray((void*)bufRef.c_str(), bufRef.size()))
    {
        GLog::log("PlayerPropReinforce::SaveToDB error! roleid:%ld", player_.role_id());
        return false;
    }

    return true;
}

bool PlayerPropReinforce::LoadFromDB(const common::PlayerPropRFData& data)
{
    if (data.primary_rf_data.size() > 0)
    {
        common::scalable::PrimaryPropRFData scalable_data;
        const std::string& bufRef = data.primary_rf_data;
        if (!scalable_data.ParseFromArray(bufRef.c_str(), bufRef.size()))
        {
            GLog::log("PlayerPropReinforce::LoadFromDB error! roleid:%ld", player_.role_id());
            return false;
        }

        primary_prop_rf_.last_recover_time = scalable_data.last_recover_time();
        primary_prop_rf_.cur_energy        = scalable_data.cur_energy();
        for (size_t i = 0; i < PPRFI_MAX; ++i)
        {
            primary_prop_rf_.detail[i].cur_add_energy = scalable_data.detail(i).cur_add_energy();
            primary_prop_rf_.detail[i].cur_level      = scalable_data.detail(i).cur_level();
        }

        AttachPrimaryPropRFPoints();
    }
    return true;
}

int32_t PlayerPropReinforce::get_pprf_add_points(PrimaryPropRFIndex index) const
{
    ASSERT(index >= 0 && index < PPRFI_MAX);
    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    int64_t points = (int64_t)primary_prop_rf_.detail[index].cur_level * (int64_t)gcfg->primary_prop_rf.rf_factor[index];
    ASSERT(points >= 0 && points < GS_INT32_MAX);
    return static_cast<int32_t>(points);
}

float PlayerPropReinforce::get_pprf_average_level() const
{
    float sum = 0.f;
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        sum += primary_prop_rf_.detail[i].cur_level;
    }
    float res = sum / (float)PPRFI_MAX;
    return res;
}

int32_t PlayerPropReinforce::calc_pprf_weight(PrimaryPropRFIndex index, float average_level) const
{
    // 计算公式为：权重 =（平均属性强化等级 – 自身属性强化等级 + 5）^ 3，算出的权重值再进行随机上下50%浮动，得到最终的权重值
    double weight = average_level - (float)primary_prop_rf_.detail[index].cur_level + (float)5;
    weight = weight * weight * weight;
    ASSERT(weight < GS_INT32_MAX); // 权重可以小于0，小于0的权重直接忽略
    // 上下浮动50%
    float tmp = mrand::RandF(0.f, 0.5f);
    if (!mrand::RandSelectF(0.5))
    {
        tmp *= -1;
    }
    weight = weight + (weight * tmp);
    return static_cast<int32_t>(weight);
}

bool PlayerPropReinforce::check_pprf_levelup() const
{
    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t rf_level_upper_limit = gcfg->primary_prop_rf.rf_level_upper_limit;

    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        if (primary_prop_rf_.detail[i].cur_level < rf_level_upper_limit)
            return true;
    }
    return false;
}

void PlayerPropReinforce::PlayerGetPropReinforce()
{
    PlayerGetPrimaryPropRF();
}

void PlayerPropReinforce::ResetPrimaryPropRF()
{
    primary_prop_rf_.clear();
}

void PlayerPropReinforce::AddPrimaryPropRFCurEnergy()
{
    primary_prop_rf_.last_recover_time = 0;
    RecalculatePPRFEnergy();
}

void PlayerPropReinforce::PlayerGetPrimaryPropRF()
{
    // 重新计算cur_energy
    RecalculatePPRFEnergy(0, false, false);

    // send to client
    G2C::PlayerPrimaryPropRF packet;
    packet.info.last_recover_time = primary_prop_rf_.last_recover_time;
    packet.info.cur_energy        = primary_prop_rf_.cur_energy;
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        G2C::PlayerPrimaryPropRF::Detail data;
        data.cur_add_energy = primary_prop_rf_.detail[i].cur_add_energy;
        data.add_points     = get_pprf_add_points(static_cast<PrimaryPropRFIndex>(i));
        packet.detail.push_back(data);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerPropReinforce::RecalculatePPRFEnergy(int32_t add_energy, bool is_reset, bool sync_to_client)
{
    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t rf_energy_upper_limit = gcfg->primary_prop_rf.rf_energy_upper_limit;
    // 第一次计算把能量充满
    if (primary_prop_rf_.last_recover_time == 0)
    {
        primary_prop_rf_.last_recover_time = g_timer->GetSysTime();
        primary_prop_rf_.cur_energy        = rf_energy_upper_limit;
    }

    // 元宝加能量
    if (add_energy > 0 && primary_prop_rf_.cur_energy < rf_energy_upper_limit)
    {
        if ((primary_prop_rf_.cur_energy + add_energy) >= rf_energy_upper_limit)
        {
            primary_prop_rf_.last_recover_time = g_timer->GetSysTime();
        }
    }

    primary_prop_rf_.cur_energy += add_energy;
    if (primary_prop_rf_.cur_energy > rf_energy_upper_limit)
    {
        // 可能是策划改了数据，导致存盘数据大于模板数据
        primary_prop_rf_.cur_energy = rf_energy_upper_limit;
        GLog::log("初阶属性强化当前能量居然大于全职配置表里的最大能量！");
    }
    if (is_reset)
    {
        primary_prop_rf_.last_recover_time = g_timer->GetSysTime();
        primary_prop_rf_.cur_energy        = 0;
    }

    // 按时间恢复
    if (primary_prop_rf_.cur_energy < rf_energy_upper_limit)
    {
        int32_t now       = g_timer->GetSysTime();
        int32_t time_diff = now - primary_prop_rf_.last_recover_time;
        int32_t multiple  = (int)time_diff / (int)(gcfg->primary_prop_rf.rf_energy_recover.time);
        if (multiple > 0)
        {
            primary_prop_rf_.last_recover_time = now;
            primary_prop_rf_.cur_energy       += gcfg->primary_prop_rf.rf_energy_recover.value * multiple;
            if (primary_prop_rf_.cur_energy > rf_energy_upper_limit)
            {
                primary_prop_rf_.cur_energy = rf_energy_upper_limit;
            }
        }
    }

    if (sync_to_client)
    {
        // sync to client
        G2C::QueryPrimaryPropRF_Re packet;
        packet.info.last_recover_time = primary_prop_rf_.last_recover_time;
        packet.info.cur_energy        = primary_prop_rf_.cur_energy;
        player_.sender()->SendCmd(packet);
    }
}

void PlayerPropReinforce::AttachPrimaryPropRFPoints() const
{
    refresh_pprf_points(true);
}

void PlayerPropReinforce::DetachPrimaryPropRFPoints() const
{
    refresh_pprf_points(false);
}

void PlayerPropReinforce::refresh_pprf_points(bool is_attach) const
{
    for (size_t i = 0; i < PPRFI_MAX; ++i)
    {
        int32_t add_point  = get_pprf_add_points(static_cast<PrimaryPropRFIndex>(i));
        int32_t prop_index = -1;
        switch (i)
        {
            case PPRFI_HP:
                prop_index = PROP_INDEX_MAX_HP;
                break;

            case PPRFI_P_ATK:
                prop_index = PROP_INDEX_PHYSICAL_ATTACK;
                break;

            case PPRFI_M_ATK:
                prop_index = PROP_INDEX_MAGIC_ATTACK;
                break;

            case PPRFI_P_DEF:
                prop_index = PROP_INDEX_PHYSICAL_DEFENCE;
                break;

            case PPRFI_M_DEF:
                prop_index = PROP_INDEX_MAGIC_DEFENCE;
                break;

            case PPRFI_P_PIE:
                prop_index = PROP_INDEX_PHYSICAL_PIERCE;
                break;

            case PPRFI_M_PIE:
                prop_index = PROP_INDEX_MAGIC_PIERCE;
                break;

            default:
                ASSERT(false);
                break;
        }

        ASSERT(prop_index >= 0);
        if (is_attach)
        {
            player_.IncPropPoint(prop_index, add_point);
        }
        else
        {
            player_.DecPropPoint(prop_index, add_point);
        }
    }
}

void PlayerPropReinforce::RegisterCmdHandler()
{
    REGISTER_NORMAL_CMD_HANDLER(C2G::QueryPrimaryPropRF, PlayerPropReinforce::CMDHandler_QueryPrimaryPropRF);
    REGISTER_NORMAL_CMD_HANDLER(C2G::BuyPrimaryPropRFEnergy, PlayerPropReinforce::CMDHandler_BuyPrimaryPropRFEnergy);
    REGISTER_NORMAL_CMD_HANDLER(C2G::PrimaryPropReinforce, PlayerPropReinforce::CMDHandler_PrimaryPropReinforce);
}

void PlayerPropReinforce::CMDHandler_QueryPrimaryPropRF(const C2G::QueryPrimaryPropRF& cmd)
{
    RecalculatePPRFEnergy();
}

void PlayerPropReinforce::CMDHandler_BuyPrimaryPropRFEnergy(const C2G::BuyPrimaryPropRFEnergy& cmd)
{
    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    if (cmd.index < 0 || cmd.index >= (int)gcfg->primary_prop_rf.sale_energy.size())
    {
        player_.sender()->ErrorMessage(G2C::ERR_CMD_PARAM_INVALID);
        return;
    }

    if (!check_pprf_levelup())
    {
        __PRINTF("CMDHandler_BuyPrimaryPropRFEnergy 没有可以强化的初阶属性！");
        player_.sender()->ErrorMessage(G2C::ERR_HAS_NOT_PRIMARY_PROP_RF);
        return;
    }

    int32_t energy_value = gcfg->primary_prop_rf.sale_energy[cmd.index].energy_value;
    int32_t cash_needed  = gcfg->primary_prop_rf.sale_energy[cmd.index].cash_price;
    
    G2C::BuyPrimaryPropRFEnergy_Re packet;
    packet.sale_index = cmd.index;
    if (!player_.CheckCash(cash_needed))
    {
        packet.result = G2C::BuyPrimaryPropRFEnergy_Re::EC_NO_CASH;
        player_.sender()->SendCmd(packet);
        return;
    }
    player_.UseCash(cash_needed);

    ///
    /// buy success
    ///

    // 随多次可以更平均
    while (energy_value > 0)
    {
        int tmp_value = (energy_value > 50) ? 50 : energy_value;
        // calc pprf
        CalcPrimaryPropRF(tmp_value, false, false);
        energy_value -= tmp_value;
    }

    // send to client
    packet.result = G2C::BuyPrimaryPropRFEnergy_Re::EC_SUCCESS;
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        G2C::BuyPrimaryPropRFEnergy_Re::Detail send_detail;

        PrimaryPropRF::Detail& detail = primary_prop_rf_.detail[i];
        send_detail.cur_energy = detail.cur_add_energy;
        send_detail.add_points = get_pprf_add_points(static_cast<PrimaryPropRFIndex>(i));
        packet.detail.push_back(send_detail);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerPropReinforce::CMDHandler_PrimaryPropReinforce(const C2G::PrimaryPropReinforce& cmd)
{
    // 先刷一下时间
    RecalculatePPRFEnergy(0, false, false);

    // 检查当前能量
    if (primary_prop_rf_.cur_energy <= 0)
        return;

    CalcPrimaryPropRF(primary_prop_rf_.cur_energy, true, true);
}

void PlayerPropReinforce::CalcPrimaryPropRF(int32_t calc_energy, bool reset_cur_energy, bool sync_levelup)
{
    if (calc_energy <= 0)
        return;

    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t rf_level_upper_limit = gcfg->primary_prop_rf.rf_level_upper_limit;

    // 选出权重大于0的项
    float average_level = get_pprf_average_level();
    int32_t prob_value[PPRFI_MAX] = {0};
    int32_t prob_index[PPRFI_MAX] = {0};
    size_t  prob_size = 0;
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        int32_t value = calc_pprf_weight(static_cast<PrimaryPropRFIndex>(i), average_level);
        if (value > 0 && primary_prop_rf_.detail[i].cur_level < rf_level_upper_limit)
        {
            prob_value[prob_size] = value;
            prob_index[prob_size] = i;
            ++prob_size;
        }
    }
    if (prob_size == 0)
    {
        __PRINTF("CMDHandler_PrimaryPropReinforce 没有可以强化的初阶属性！");
        player_.sender()->ErrorMessage(G2C::ERR_HAS_NOT_PRIMARY_PROP_RF);
        return;
    }

    // 归一化
    int sum = 0;
    mrand::Normalization(prob_value, prob_size);
    for (size_t i = 0; i < prob_size; ++i)
    {
        sum += prob_value[i];
    }

    // 算出各项的最终点数
    int add_value_sum = 0;
    int32_t add_value[PPRFI_MAX] = {0};
    for (size_t i = 0; i < prob_size; ++i)
    {
        add_value[i]   = ((float)prob_value[i] / (float)sum) * calc_energy;
        add_value_sum += add_value[i];
    }

    // 计算余数（先把整的分配出去，剩下的随机分配，保证不少加点）
    int remainder = calc_energy - add_value_sum;
    ASSERT(remainder >= 0);
    while (remainder > 0)
    {
        int index = mrand::Rand(0, 10000) % prob_size;
        add_value[index] += 1;
        --remainder;
    }

    // 先把原来的加点去掉
    DetachPrimaryPropRFPoints();

    // 加到当前的属性值上
    int32_t energy_this_round[PPRFI_MAX] = {0};
    for (size_t i = 0; i < prob_size; ++i)
    {
        primary_prop_rf_.detail[prob_index[i]].cur_add_energy += add_value[i];
        energy_this_round[prob_index[i]] = add_value[i];
    }
    // reset first
    RecalculatePPRFEnergy(0, reset_cur_energy, false);
    // calc levelup
    CalcPrimaryPropRFLevelUp(energy_this_round, PPRFI_MAX, sync_levelup);

    // 重新计算加点
    AttachPrimaryPropRFPoints();
}

void PlayerPropReinforce::CalcPrimaryPropRFLevelUp(const int32_t* option, size_t size, bool sync_to_client)
{
    ASSERT(size == PPRFI_MAX);
    
    G2C::PrimaryPropReinforce_Re packet;
    packet.info.last_recover_time = primary_prop_rf_.last_recover_time;
    packet.info.cur_energy        = primary_prop_rf_.cur_energy;

    // 获取配置表数据
    const dataTempl::GlobalConfigTempl* gcfg = s_pDataTempl->QueryGlobalConfigTempl();
    int32_t rf_lvlup_required_energy         = gcfg->primary_prop_rf.rf_lvlup_required_energy;
    int32_t prob_value[kMaxUpgradeCount]     = {0};
    int32_t prob_size = gcfg->primary_prop_rf.rf_level_upgrade.size();
    for (size_t i = 0; i < gcfg->primary_prop_rf.rf_level_upgrade.size(); ++i)
    {
        prob_value[i] = gcfg->primary_prop_rf.rf_level_upgrade[i].prob;
    }

    // 计算升级
    for (size_t i = 0; i < primary_prop_rf_.detail.size(); ++i)
    {
        G2C::PrimaryPropReinforce_Re::Detail send_detail;

        PrimaryPropRF::Detail& detail = primary_prop_rf_.detail[i];
        while (detail.cur_add_energy >= rf_lvlup_required_energy)
        {
            // 选出升级等级
            int selected_index = mrand::RandSelect(prob_value, prob_size);
            const dataTempl::PrimaryPropReinforceConfig::RFLevelUpProbData& data 
                = gcfg->primary_prop_rf.rf_level_upgrade[selected_index];

            // 记录升级结果
            detail.cur_level      += data.count;
            detail.cur_add_energy -= rf_lvlup_required_energy;

            // 记录升级的type
            G2C::PrimaryPropReinforce_Re::UpgradeInfo upinfo;
            upinfo.type       = data.type;
            upinfo.add_points = get_pprf_add_points(static_cast<PrimaryPropRFIndex>(i));
            send_detail.upgrade_info.push_back(upinfo);
        }

        send_detail.cur_energy = detail.cur_add_energy;
        send_detail.add_points = get_pprf_add_points(static_cast<PrimaryPropRFIndex>(i));
        send_detail.add_energy = option[i];
        packet.detail.push_back(send_detail);
    }

    if (sync_to_client)
    {
        player_.sender()->SendCmd(packet);
    }
}

} // namespace gamed
