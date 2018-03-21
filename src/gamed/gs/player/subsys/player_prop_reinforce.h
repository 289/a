#ifndef GAMED_GS_SUBSYS_PLAYER_PROP_REINFORCE_H_
#define GAMED_GS_SUBSYS_PLAYER_PROP_REINFORCE_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player属性强化子系统（初阶属性强化等）
 */
class PlayerPropReinforce : public PlayerSubSystem
{
public:
	PlayerPropReinforce(Player& player);
	virtual ~PlayerPropReinforce();

	virtual void RegisterCmdHandler();
	bool SaveToDB(common::PlayerPropRFData* pdata);
	bool LoadFromDB(const common::PlayerPropRFData& data);

    void PlayerGetPropReinforce();
    void ResetPrimaryPropRF();
    void AddPrimaryPropRFCurEnergy();


public:
    enum PrimaryPropRFIndex
    {
        PPRFI_HP = 0, // 生命值
        PPRFI_P_ATK,  // 物理攻击
        PPRFI_M_ATK,  // 魔法攻击
        PPRFI_P_DEF,  // 物理防御
        PPRFI_M_DEF,  // 魔法防御
        PPRFI_P_PIE,  // 物理穿透
        PPRFI_M_PIE,  // 魔法穿透
        PPRFI_MAX
    };


protected:
    void   CMDHandler_QueryPrimaryPropRF(const C2G::QueryPrimaryPropRF&);
    void   CMDHandler_BuyPrimaryPropRFEnergy(const C2G::BuyPrimaryPropRFEnergy&);
    void   CMDHandler_PrimaryPropReinforce(const C2G::PrimaryPropReinforce&);


private:
    class PrimaryPropRF
    {
    public:
        struct Detail
        {
            Detail() 
                : cur_add_energy(0), 
                  cur_level(0) 
            { }

            void clear()
            {
                cur_add_energy = 0;
                cur_level      = 0;
            }

            int32_t cur_add_energy; // 当前已经充了多少能量
            int32_t cur_level;      // 当前等级
        };

        PrimaryPropRF() 
            : last_recover_time(0), 
              cur_energy(0) 
        { }

        void clear()
        {
            last_recover_time = 0;
            cur_energy        = 0;
            for (size_t i = 0; i < detail.size(); ++i)
            {
                detail[i].clear();
            }
        }

        int32_t last_recover_time;  // 上次恢复能量的时间点，绝对时间，0表示上次恢复已经大于等于能量上限
        int32_t cur_energy;         // 当前能量值
        FixedArray<Detail, PPRFI_MAX> detail; // 下标对应枚举PrimaryPropRFIndex
    };


private:
    void    PlayerGetPrimaryPropRF();
    void    RecalculatePPRFEnergy(int32_t add_energy = 0, bool is_reset = false, bool sync_to_client = true);  // 重新计算当前能量值
    void    AttachPrimaryPropRFPoints() const;
    void    DetachPrimaryPropRFPoints() const;
    void    CalcPrimaryPropRF(int32_t calc_energy, bool reset_cur_energy, bool sync_levelup);
    void    CalcPrimaryPropRFLevelUp(const int32_t* option, size_t size, bool sync_to_client);
    void    refresh_pprf_points(bool is_attach) const;
    float   get_pprf_average_level() const;
    int32_t get_pprf_add_points(PrimaryPropRFIndex index) const; // 计算初阶属性强化最终加点
    int32_t calc_pprf_weight(PrimaryPropRFIndex index, float average_level) const; // 计算初阶属性强化权重
    bool    check_pprf_levelup() const; // 检查是否还有可以升级的初阶属性


private:
    PrimaryPropRF    primary_prop_rf_;
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_PROP_REINFORCE_H_
