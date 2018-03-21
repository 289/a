#ifndef GAMED_GS_TEMPLATE_DATATEMPL_PRIMARY_PROP_REINFORCE_CFG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_PRIMARY_PROP_REINFORCE_CFG_H_


namespace dataTempl {

enum PrimaryPropRFUpgradeType
{
    PPRF_UT_FAILURE = 1, // 强化等级提升失败
    PPRF_UT_NORMAL,      // 强化等级普通提升
    PPRF_UT_CRITICAL,    // 强化等级暴击提升
    PPRF_UT_SUPER,       // 强化等级超级提升
};

enum PrimaryPropRFIndex
{
    PPRFI_HP = 0, // 生命值
    PPRFI_P_ATK,  // 物理攻击，physical attack
    PPRFI_M_ATK,  // 魔法攻击，magic attack
    PPRFI_P_DEF,  // 物理防御，physical defense
    PPRFI_M_DEF,  // 魔法防御，magic defense
    PPRFI_P_PIE,  // 物理穿透，physical pierce
    PPRFI_M_PIE,  // 魔法穿透，magic pierce
    PPRFI_MAX
};

/**
 * @brief 初级属性强化配置表
 */
class PrimaryPropReinforceConfig
{
public:
    static const size_t kMaxUpgradeCount  = 4;
    static const size_t kMaxEnergyForSale = 3;

    struct RFLevelUpProbData
    {
        int8_t  type;  // 强化成功的级别，对应枚举PrimaryPropRFUpgradeType
        int32_t prob;  // 强化概率，万分数
        int32_t count; // 强化成功时，提升的等级数（提升多少级）
        NESTED_DEFINE(type, prob, count);
    };
    
    struct EnergyRecover
    {
        int32_t time;  // 强化能量的回复速度，默认值300，单位秒
        int32_t value; // 强化能量的回复值，默认值1
        NESTED_DEFINE(time, value);
    };

    // 卖能量
    struct EnergyForSale
    {
        int32_t cash_price;    // 元宝价格
        int32_t energy_value;  // 所能买到的能量数
        NESTED_DEFINE(cash_price, energy_value);
    };


public:
// 0
    // 属性强化最终加值为：属性强化系数 * 属性强化等级
    // 生命值强化系数，默认值40
    // 物理攻击强化系数，默认值4
    // 魔法攻击强化系数，默认值4
    // 物理防御强化系数，默认值5
    // 魔法防御强化系数，默认值5
    // 物理穿透强化系数，默认值5
    // 魔法穿透强化系数，默认值5
    BoundArray<int32_t, PPRFI_MAX> rf_factor; // 强化系数，下标对应枚举PrimaryPropRFIndex
    BoundArray<RFLevelUpProbData, kMaxUpgradeCount> rf_level_upgrade; // 升级概率，需要归一化
    int32_t rf_level_upper_limit;             // 强化等级上限，默认值100
    int32_t rf_energy_upper_limit;            // 强化能量上限，默认值65
    int32_t rf_lvlup_required_energy;         // 升级所需的能量值，默认值10，7条强化项都用这项

// 5
    EnergyRecover  rf_energy_recover;         // 强化能量恢复
    BoundArray<EnergyForSale, kMaxEnergyForSale> sale_energy; // 可以出售的能量


    NESTED_DEFINE(rf_factor, rf_level_upgrade, rf_level_upper_limit, rf_energy_upper_limit, rf_lvlup_required_energy, 
                  rf_energy_recover, sale_energy);


public:
	bool CheckDataValidity() const
	{
        if (rf_factor.size() != PPRFI_MAX)
            return false;

        for (size_t i = 0; i < rf_factor.size(); ++i)
        {
            if (rf_factor[i] <= 0)
                return false;
        }
        
        if (rf_level_upper_limit <= 0 || rf_level_upper_limit >= 100000)
            return false;

        if (rf_level_upgrade.size() <= 0)
            return false;

        int32_t prob_sum = 0;
        for (size_t i = 0; i < rf_level_upgrade.size(); ++i)
        {
            const RFLevelUpProbData& data = rf_level_upgrade[i];

            if (data.type != PPRF_UT_FAILURE && 
                data.type != PPRF_UT_NORMAL &&
                data.type != PPRF_UT_CRITICAL && 
                data.type != PPRF_UT_SUPER)
            {
                return false;
            }

            if (data.type == PPRF_UT_FAILURE && data.count != 0)
                return false;

            if (data.type != PPRF_UT_FAILURE && data.count <= 0)
                return false;

            if (data.prob <= 0 || data.prob > 10000)
                return false;

            prob_sum += data.prob;
        }

        // 归一化检查
        if (prob_sum != 10000)
            return false;

        if (rf_energy_upper_limit <= 0 || rf_energy_upper_limit >= 1000)
            return false;

        if (rf_lvlup_required_energy <= 0 || rf_lvlup_required_energy >= 1000)
            return false;

        if (rf_energy_recover.time <= 0 || rf_energy_recover.value <= 0)
            return false;

        if (sale_energy.size() == 0)
            return false;

        for (size_t i = 0; i < sale_energy.size(); ++i)
        {
            const EnergyForSale& sale = sale_energy[i];
            if (sale.cash_price <= 0 || sale.energy_value <= 0)
                return false;

            if (i != 0)
            {
                const EnergyForSale& prev = sale_energy[i-1];
                if (prev.cash_price >= sale.cash_price ||
                    prev.energy_value >= sale.energy_value)
                {
                    return false;
                }
            }
        }

        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_PRIMARY_PROP_REINFORCE_CFG_H_
