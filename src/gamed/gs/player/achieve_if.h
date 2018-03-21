#ifndef GAMED_GS_PLAYER_ACHIEVE_IF_H_
#define GAMED_GS_PLAYER_ACHIEVE_IF_H_

#include "game_module/achieve/include/achieve_interface.h"

namespace gamed {

class Player;

/**
 * @brief PlayerAchieveIf
 */
class PlayerAchieveIf : public AchieveInterface
{
public:
    PlayerAchieveIf(Player* player);
    virtual ~PlayerAchieveIf();

	// 服务器，客户端都需实现的接口
    virtual int64_t GetId() const;
	virtual int32_t GetRoleClass() const; // 返回职业模板ID
    virtual int32_t GetLastLoginTime() const;

    virtual bool CanDeliverItem(int8_t package_type, int32_t item_types) const;
    virtual void DeliverAchievePoint(int32_t point);

	// 返回存储的任务数据
    virtual achieve::ActiveAchieve* GetActiveAchieve();
    virtual achieve::FinishAchieve* GetFinishAchieve();
    virtual achieve::AchieveData* GetAchieveData();
    virtual int32_t GetAchieveData(int32_t type, int32_t sub_type);

	virtual void SendNotify(uint16_t type, const void* buf, size_t size);

    // 只有服务器需要实现的接口
    virtual void DeliverItem(int32_t item_id, int32_t num);
    virtual void DeliverTitle(int32_t title_id);

    // 只有客户端需要实现的接口
	virtual void StatusChanged(int32_t id, int8_t status) { }

    Player* GetPlayer();
private:
    Player* pplayer_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_ACHIEVE_IF_H_
