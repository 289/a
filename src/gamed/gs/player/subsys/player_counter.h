#ifndef GAMED_GS_SUBSYS_PLAYER_COUNTER_H_
#define GAMED_GS_SUBSYS_PLAYER_COUNTER_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief: player计数器系统（给策划使用的玩家自身计数器，大部分由任务修改）
 */
class PlayerCounter : public PlayerSubSystem
{
public:
	PlayerCounter(Player& player);
	virtual ~PlayerCounter();

    bool    LoadFromDB(const common::PlayerCounterData& data);
	bool    SaveToDB(common::PlayerCounterData* pData);

    bool    RegisterCounter(int32_t tid);
    bool    UnregisterCounter(int32_t tid);
    bool    ModifyCounter(int32_t tid, int32_t delta);
    bool    GetCounter(int32_t tid, int32_t& value) const;
    bool    SetCounter(int32_t tid, int32_t value);

    // sync to client after getalldata
    void    SendCounterList() const;

private:
    void    NotifyCounterChange(int8_t type, int32_t tid, int32_t value);

private:
    typedef std::map<int32_t/*tid*/, int32_t/*value*/> CounterMap;
    CounterMap    counter_map_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_COUNTER_H_
