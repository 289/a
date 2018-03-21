#include "achieve_notify.h"
#include "achieve_msg.h"
#include "achieve_interface.h"

namespace achieve
{

void NotifyUtil::SendFinish(Player* player, AchieveID id)
{
    AchieveNotifyFinish msg;
    msg.id = id;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

void NotifyUtil::SendComplete(Player* player, AchieveID id)
{
    AchieveNotifyComplete msg;
    msg.id = id;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

void NotifyUtil::SendModify(Player* player, AchieveID id, int32_t type, int32_t sub_type, int32_t value)
{
    AchieveNotifyModify msg;
    msg.id = id;
    msg.data_type = type;
    msg.data_subtype = sub_type;
    msg.value = value;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

void NotifyUtil::SendCheckFinish(Player* player, AchieveID id)
{
    AchieveNotifyCheckFinish msg;
    msg.id = id;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

void NotifyUtil::SendAward(Player* player, AchieveID id)
{
    AchieveNotifyAward msg;
    msg.id = id;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

void NotifyUtil::SendPlayerRevive(Player* player, AchieveID id)
{
    AchieveNotifyRevive msg;
    msg.id = id;
    msg.Marshal();
    player->SendNotify(msg.GetType(), msg.GetContent(), msg.GetSize());
}

} // namespace achieve
