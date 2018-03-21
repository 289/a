#ifndef GAMED_GS_SUBSYS_PLAYER_LANDMINE_H_
#define GAMED_GS_SUBSYS_PLAYER_LANDMINE_H_

#include "gs/player/player_subsys.h"


namespace mapDataSvr {
    class AreaMonster;
} // namespace mapDataSvr


namespace gamed {

/**
 * @brief：player暗雷子系统
 *  （1）该子系统相当于一个暗雷的发生器
 *  （2）玩家自己伪装成暗雷给自己触发暗雷概率
 */
class PlayerLandmine : public PlayerSubSystem
{
    static const float kMinCalcDistanceSquare; // 暗雷最小的计算距离
public:
	PlayerLandmine(Player& player);
	virtual ~PlayerLandmine();

    virtual void OnHeartbeat(time_t cur_time);
	virtual void OnRelease();
	virtual void RegisterMsgHandler();

    void ResetTriggerData();


protected:
	int  MSGHandler_NotifyLandmineInfo(const MSG&);


private:
    struct entry_t
    {
        entry_t()
            : encounter_timer(0),
              pelemdata(NULL)
        { }

        XID object_xid;
        int encounter_timer;
        const mapDataSvr::AreaMonster* pelemdata;
    };

    enum TriggerType
    {
        TT_NORMAL,
        TT_ACTIVE_TASK,
        TT_FINISH_TASK,
    };

    void TriggerCombat(entry_t& ent);
    bool RandTriggerCombat(entry_t& ent, TriggerType type);


private:
    A2DVECTOR prev_pos_; // 上次计算的位置

    typedef std::map<int64_t/*obj_id*/, entry_t> LandmineInfoMap;
    LandmineInfoMap landmine_map_;
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_LANDMINE_H_
