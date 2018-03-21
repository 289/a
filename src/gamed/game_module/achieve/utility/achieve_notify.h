#ifndef ACHIEVE_NOTIFY_SENDER_H_
#define ACHIEVE_NOTIFY_SENDER_H_

#include "achieve_types.h"

namespace achieve
{

// 通知消息发送辅助类
class NotifyUtil
{
public:
    static void SendFinish(Player* player, AchieveID id);
    static void SendComplete(Player* player, AchieveID id);
    static void SendModify(Player* player, AchieveID id, int32_t type, int32_t sub_type, int32_t value);

    static void SendCheckFinish(Player* player, AchieveID id);
    static void SendAward(Player* player, AchieveID id);
    static void SendPlayerRevive(Player* player, AchieveID id);
};

} // namespace achieve

#endif // ACHIEVE_NOTIFY_SENDER_H_
