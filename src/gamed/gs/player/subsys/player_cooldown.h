#ifndef GAMED_GS_SUBSYS_PLAYER_COOLDOWN_H_
#define GAMED_GS_SUBSYS_PLAYER_COOLDOWN_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player冷却子系统
 *  （1）冷却id，可以是服务器自己定义的index也可以是策划配的模板id
 *       检查物品冷却都是策划配的模板id（包括冷却组id，物品自己的模板id）
 */
class PlayerCoolDown : public PlayerSubSystem
{
public:
    PlayerCoolDown(Player& player);
	virtual ~PlayerCoolDown();

    bool SaveToDB(common::PlayerCoolDownData* pdata);
    bool LoadFromDB(const common::PlayerCoolDownData& data);

    void PlayerGetCoolDownData();
	void SetCoolDown(int id, int msec);
	void ClrCoolDown(int id);
	bool TestCoolDown(int index); // 检查index，index枚举见playerdef
	bool TestItemCoolDown(int cd_group_id, int item_id); // 检查物品冷却用这个函数

private:
	CoolDownMan  cooldown_man_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_COOLDOWN_H_
