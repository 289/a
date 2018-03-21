#ifndef ACHIEVE_ACHIEVE_MSG_H_
#define ACHIEVE_ACHIEVE_MSG_H_

#include "achieve_types.h"

#define DECLARE_NOTIFY(name, type) \
	DECLARE_PACKET_COMMON_TEMPLATE(name, type, AchieveNotifyBase)

#define NOTIFY_DEFINE PACKET_DEFINE

namespace achieve
{

enum AchieveNotifyType
{
	ACHIEVE_SVR_NOTIFY_FINISH,	        // 成就条件达成
	ACHIEVE_SVR_NOTIFY_COMPLETE,	    // 成就领取奖励完成
    ACHIEVE_SVR_NOTIFY_MODIFY,          // 成就数据变化

	ACHIEVE_CLT_NOTIFY_CHECK_FINISH,	// 检查成就条件达成
	ACHIEVE_CLT_NOTIFY_AWARD,	        // 领取成就奖励
    ACHIEVE_CLT_NOTIFY_REVIVE,          // 检查玩家死亡成就
};

class AchieveNotifyBase : public shared::net::BasePacket
{
public:
	AchieveNotifyBase(uint16_t type);
	virtual ~AchieveNotifyBase();

	virtual void Marshal();
	virtual void Unmarshal();
protected:
	virtual void OnMarshal() = 0;
	virtual void OnUnmarshal() = 0;
public:
	int32_t id;
};

class AchieveNotifyFinish : public AchieveNotifyBase
{
	DECLARE_NOTIFY(AchieveNotifyFinish, ACHIEVE_SVR_NOTIFY_FINISH);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

class AchieveNotifyComplete : public AchieveNotifyBase
{
	DECLARE_NOTIFY(AchieveNotifyComplete, ACHIEVE_SVR_NOTIFY_COMPLETE);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

class AchieveNotifyModify : public AchieveNotifyBase
{
    DECLARE_NOTIFY(AchieveNotifyModify, ACHIEVE_SVR_NOTIFY_MODIFY);
protected:
    virtual void OnMarshal();
    virtual void OnUnmarshal();
public:
    int32_t data_type;
    int32_t data_subtype;
    int32_t value;
};

class AchieveNotifyCheckFinish : public AchieveNotifyBase
{
	DECLARE_NOTIFY(AchieveNotifyCheckFinish, ACHIEVE_CLT_NOTIFY_CHECK_FINISH);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

class AchieveNotifyAward : public AchieveNotifyBase
{
	DECLARE_NOTIFY(AchieveNotifyAward, ACHIEVE_CLT_NOTIFY_AWARD);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

class AchieveNotifyRevive : public AchieveNotifyBase
{
	DECLARE_NOTIFY(AchieveNotifyRevive, ACHIEVE_CLT_NOTIFY_REVIVE);
protected:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
};

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_MSG_H_
