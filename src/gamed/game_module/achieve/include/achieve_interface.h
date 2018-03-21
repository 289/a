#ifndef GAMED_ACHIEVE_INTERFACE_H_
#define GAMED_ACHIEVE_INTERFACE_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace achieve
{
class ActiveAchieve;
class FinishAchieve;
class AchieveData;
} // namespace achieve

namespace gamed
{

enum AchieveType
{
    // 0
    ACHIEVE_BASE,
    ACHIEVE_PROP,
    ACHIEVE_SKILL,
    ACHIEVE_SOCIAL,
    ACHIEVE_PET,
    // 5
    ACHIEVE_REPUTATION,
    ACHIEVE_MONSTER,
    ACHIEVE_LOGIN,
    ACHIEVE_MAP,
    ACHIEVE_TASK,
    // 10
    ACHIEVE_REFINE,
    ACHIEVE_SCORE,
    ACHIEVE_INSTANCE,
    ACHIEVE_MONEY,
    ACHIEVE_MOUNT,
    // 15
    ACHIEVE_STAR,
    ACHIEVE_CARD

    //ACHIEVE_CASH,
};

enum BaseAchieveType
{
    ACHIEVE_BASE_LEVEL,
    ACHIEVE_BASE_COMBAT_VALUE,
};

enum SocialAchieveType
{
    ACHIEVE_SOCIAL_FRIEND,
    ACHIEVE_SOCIAL_ENEMY,
};

enum PetAchieveType
{
    ACHIEVE_PET_POWER = 0,
    ACHIEVE_PET_NUM = 1,
    // 大于1的类型表示指定星等的宠物的数量
};

enum LoginAchieveType
{
    ACHIEVE_LOGIN_TOTAL,
    ACHIEVE_LOGIN_CONSECUTIVE,
};

enum RefineAchieveType
{
    ACHIEVE_REFINE_NUM,
    ACHIEVE_REFINE_LEVEL,
};

enum ScoreAchieveType
{
    ACHIEVE_SCORE_TOTAL,
    ACHIEVE_SCORE_USED,
};

enum MoneyAchieveType
{
    ACHIEVE_MONEY_TOTAL,
    ACHIEVE_MONEY_USED,
};

//enum CashAchieveType
//{
    //ACHIEVE_CASH_TOTAL,
    //ACHIEVE_CASH_USED,
//};

enum MountAchieveType
{
    ACHIEVE_MOUNT_CATEGORY,
    // 剩下为对应骑具位的等级
};

enum StarAchieveType
{
    ACHIEVE_STAR_SPARK,
};

enum CardAchieveType
{
    ACHIEVE_CARD_NUM, // 镶嵌的星核数量
    // 1000+品质数表示对应品质镶嵌的星核数
    // 2000+等级数表示对应等级镶嵌的星核数
};

///
/// 对象封装类
///
class AchieveInterface
{
public:
    AchieveInterface()
    {
    }
    virtual ~AchieveInterface()
    {
    }

	// 服务器，客户端都需实现的接口
	virtual int64_t GetId() const = 0;
	virtual int32_t GetRoleClass() const = 0;
    virtual int32_t GetLastLoginTime() const = 0;

    virtual bool CanDeliverItem(int8_t package_type, int32_t item_types) const = 0;
    virtual void DeliverAchievePoint(int32_t point) = 0;

    virtual achieve::ActiveAchieve* GetActiveAchieve() = 0;
    virtual achieve::FinishAchieve* GetFinishAchieve() = 0;
    virtual achieve::AchieveData* GetAchieveData() = 0;
    virtual int32_t GetAchieveData(int32_t type, int32_t sub_type) = 0;

	virtual void SendNotify(uint16_t type, const void* buf, size_t size) = 0;

    // 只有客户端需要实现的接口
	virtual void StatusChanged(int32_t id, int8_t status) = 0;

    // 只有服务器需要实现的接口
    virtual void DeliverItem(int32_t item_id, int32_t num) = 0;
    virtual void DeliverTitle(int32_t title_id) = 0;
};

} // namespace gamed

#endif // GAMED_ACHIEVE_INTERFACE_H_
