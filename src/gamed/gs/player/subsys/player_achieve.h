#ifndef GAMED_GS_SUBSYS_PLAYER_ACHIEVE_H_
#define GAMED_GS_SUBSYS_PLAYER_ACHIEVE_H_

#include "game_module/achieve/include/achieve_data.h"

#include "gs/player/player_subsys.h"

namespace gamed {

/**
 * @brief：player成就子系统
 */
class PlayerAchieve : public PlayerSubSystem
{
public:
    PlayerAchieve(Player& player);
    virtual ~PlayerAchieve();

	bool SaveToDB(common::PlayerAchieveData* pData);
	bool LoadFromDB(const common::PlayerAchieveData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    inline achieve::ActiveAchieve* GetActiveAchieve();
    inline achieve::FinishAchieve* GetFinishAchieve();
    inline achieve::AchieveData* GetAchieveData();

    void PlayerGetAchieveData();
    void RefineAchieve(int32_t level);
    void StorageTaskComplete(int32_t taskid, int8_t quality);
    void FinishInstance(int32_t ins_id);
    void KillMonster(int32_t monster_id, int32_t monster_num);
    void GainMoney(int32_t money);
    void SpendMoney(int32_t money);
    int32_t GetInsFinishCount(int32_t ins_id) const;
protected:
	//
	// CMD处理函数
	//
    void CMDHandler_AchieveNotify(const C2G::AchieveNotify&);
private:
    achieve::ActiveAchieve  active_achieve_;
    achieve::FinishAchieve  finish_achieve_;
    achieve::AchieveData    achieve_data_;
};

///
/// inline func
///
inline achieve::ActiveAchieve* PlayerAchieve::GetActiveAchieve()
{
    return &active_achieve_;
}

inline achieve::FinishAchieve* PlayerAchieve::GetFinishAchieve()
{
    return &finish_achieve_;
}

inline achieve::AchieveData* PlayerAchieve::GetAchieveData()
{
    return &achieve_data_;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_ACHIEVE_H_
