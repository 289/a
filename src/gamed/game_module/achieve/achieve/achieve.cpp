#include "achieve.h"
#include "achieve_data.h"
#include "achieve_templ.h"
#include "achieve_manager.h"
#include "achieve_interface.h"
#include "achieve_msg.h"
#include "achieve_wrapper.h"
#include "achieve_notify.h"

namespace achieve
{

using namespace shared;

static const int32_t SEC_PER_DAY = 23 * 3600 + 59 * 60 + 59;
static const int32_t MAX_DATA_VALUE = 1000000000;

bool InitAchieveSys(const char* file)
{
	return s_pAchieve->ReadFromFile(file);
}

static void OnAchieveCheckFinish(Player* player, const char* buff, size_t size)
{
    try
    {
        AchieveNotifyCheckFinish msg;
        msg.AppendBuffer(buff, size);
        msg.Unmarshal();

		__PRINTF("OnAchieveCheckFinish roleid=%ld aid=%d", player->GetId(), msg.id);
        AchieveEntry* entry = player->GetFinishAchieve()->GetEntry(msg.id);
        // 成就不能重复达成
        if (entry != NULL)
        {
            return;
        }
        // 成就不存在
        ActiveAchieve* active = player->GetActiveAchieve();
        AchieveMap::const_iterator ait = active->achieve_list.find(msg.id);
        if (ait == active->achieve_list.end())
        {
            return;
        }
        if (ait->second == NULL)
        {
            return;
        }
        // 检查是否满足完成条件
        AchieveWrapper wrapper(player, ait->second);
        if (wrapper.CheckFinish() == ERR_ACHIEVE_SUCC)
        {
            wrapper.AchieveFinish();
            NotifyUtil::SendFinish(player, msg.id);
        }
    }
    catch (const Exception& ex)
    {
        __PRINTF("OnAchieveCheckFinish Unmarshal Err %s", ex.What());
    }
}

static void OnAchieveAward(Player* player, const char* buff, size_t size)
{
    try
    {
        AchieveNotifyAward msg;
        msg.AppendBuffer(buff, size);
        msg.Unmarshal();

		__PRINTF("OnAchieveAward roleid=%ld aid=%d", player->GetId(), msg.id);
        AchieveEntry* entry = player->GetFinishAchieve()->GetEntry(msg.id);
        if (entry == NULL || entry->IsAward())
        {
            return;
        }
        // 领取成就奖励
        AchieveWrapper wrapper(player, entry->templ);
        if (wrapper.CanDeliverAward() == ERR_ACHIEVE_SUCC)
        {
            wrapper.DeliverAward();
            NotifyUtil::SendComplete(player, msg.id);
        }
    }
    catch (const Exception& ex)
    {
        __PRINTF("OnAchieveAward Unmarshal Err %s", ex.What());
    }
}

static void OnAchievePlayerRevive(Player* player, const char* buff, size_t size)
{
    try
    {
        AchieveNotifyRevive msg;
        msg.AppendBuffer(buff, size);
        msg.Unmarshal();

		__PRINTF("OnAchievePlayerRevive roleid=%ld aid=%d", player->GetId(), msg.id);
        //AchieveData* data = player->GetAchieveData();
        // 增加死亡次数的值
        // NotifyUtil::SendModify(player, 0, );
    }
    catch (const Exception& ex)
    {
        __PRINTF("OnAchievePlayerRevive Unmarshal Err %s", ex.What());
    }
}

void RecvClientNotify(Player* player, uint16_t type, const char* buff, size_t size)
{
    // 只有转职之后成就系统才开始生效
    if (player->GetRoleClass() == 0)
    {
        return;
    }

    switch (type)
    {
    case ACHIEVE_CLT_NOTIFY_CHECK_FINISH:
        return OnAchieveCheckFinish(player, buff, size);
    case ACHIEVE_CLT_NOTIFY_AWARD:
        return OnAchieveAward(player, buff, size);
    case ACHIEVE_CLT_NOTIFY_REVIVE:
        return OnAchievePlayerRevive(player, buff, size);
    default:
        return;
    }
}

static int32_t ResetTime(time_t time)
{
	struct tm date;
#ifdef PLATFORM_WINDOWS
	localtime_s(&date, &time);
#else // !PLATFORM_WINDOWS
	localtime_r(&time, &date);
#endif // PLATFORM_WINDOWS
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    return mktime(&date);
}

void PlayerLogin(Player* player, int32_t now)
{
    // 加载所有的成就
    ActiveAchieve* active = player->GetActiveAchieve();
    const IdToTemplMap* id2map = s_pAchieve->GetTemplMap(TEMPL_TYPE_ACHIEVE);
    IdToTemplMap::const_iterator iit = id2map->begin();
    for (; iit != id2map->end(); ++iit)
    {
        active->achieve_list[iit->first] = dynamic_cast<const AchieveTempl*>(iit->second);
    }

    // 去除已完成成就
    FinishAchieve* finish = player->GetFinishAchieve();
    EntryMap::const_iterator fit = finish->achieve_list.begin();
    for (; fit != finish->achieve_list.end(); ++fit)
    {
        active->achieve_list.erase(fit->first);
    }

    // 更新玩家的登录成就数据
    int32_t type = gamed::ACHIEVE_LOGIN;
    int32_t sub_type1 = gamed::ACHIEVE_LOGIN_TOTAL;
    int32_t sub_type2 = gamed::ACHIEVE_LOGIN_CONSECUTIVE;
    int32_t last_login_time = player->GetLastLoginTime();
    last_login_time = ResetTime(last_login_time);
    int32_t interval = (now - last_login_time) / SEC_PER_DAY;
    AchieveData* data = player->GetAchieveData();
    if (interval > 0)
    {
        int32_t& value = data->data[type][sub_type1];
        NotifyUtil::SendModify(player, 0, type, sub_type1, ++value);
    }
    if (interval == 1) // 连续登录
    {
        int32_t& value = data->data[type][sub_type2];
        NotifyUtil::SendModify(player, 0, type, sub_type2, ++value);
    }
    else if (interval > 1)
    {
        int32_t& day = data->data[type][sub_type2];
        day = 0;
        NotifyUtil::SendModify(player, 0, type, sub_type2, 0);
    }
}

void PlayerRefine(Player* player, int32_t level)
{
    int32_t type = gamed::ACHIEVE_REFINE;
    int32_t sub_type1 = gamed::ACHIEVE_REFINE_NUM;
    int32_t sub_type2 = gamed::ACHIEVE_REFINE_LEVEL;
    AchieveData* data = player->GetAchieveData();
    int32_t& value = data->data[type][sub_type1];
    NotifyUtil::SendModify(player, 0, type, sub_type1, ++value);
    int32_t& old_level = data->data[type][sub_type2];
    if (level > old_level)
    {
        NotifyUtil::SendModify(player, 0, type, sub_type2, level);
        old_level = level;
    }
}

void StorageTaskComplete(Player* player, int32_t taskid, int8_t quality)
{
    // 此处应该与任务里面的品质枚举对应
    assert(quality >= 1 && quality <= 7);
    int32_t type = gamed::ACHIEVE_TASK;
    AchieveData* data = player->GetAchieveData();
    int32_t& value = data->data[type][quality];
    NotifyUtil::SendModify(player, 0, type, quality, ++value);
}

void KillMonster(Player* player, int32_t mob_id, int32_t mob_count)
{
    AchieveData* data = player->GetAchieveData();
    int32_t& total_num = data->data[gamed::ACHIEVE_MONSTER][0];
    int32_t& monster_num = data->data[gamed::ACHIEVE_MONSTER][mob_id];
    if (MAX_DATA_VALUE - total_num >= mob_count)
    {
        total_num += mob_count;
    }
    if (MAX_DATA_VALUE - monster_num >= mob_count)
    {
        monster_num += mob_count;
    }
    NotifyUtil::SendModify(player, 0, gamed::ACHIEVE_MONSTER, 0, total_num);
    NotifyUtil::SendModify(player, 0, gamed::ACHIEVE_MONSTER, mob_id, monster_num);
}

void FinishInstance(Player* player, int32_t ins_id)
{
    AchieveData* data = player->GetAchieveData();
    int32_t& value = data->data[gamed::ACHIEVE_INSTANCE][ins_id];
    NotifyUtil::SendModify(player, 0, gamed::ACHIEVE_INSTANCE, ins_id, ++value);
}

void GainMoney(Player* player, int32_t money)
{
    int32_t type = gamed::ACHIEVE_MONEY;
    int32_t sub_type = gamed::ACHIEVE_MONEY_TOTAL;
    AchieveData* data = player->GetAchieveData();
    int32_t& value = data->data[type][sub_type];
    if (MAX_DATA_VALUE - value >= money)
    {
        value += money;
    }
    else
    {
        value = MAX_DATA_VALUE;
    }
    NotifyUtil::SendModify(player, 0, type, sub_type, value);
}

void SpendMoney(Player* player, int32_t money)
{
    int32_t type = gamed::ACHIEVE_MONEY;
    int32_t sub_type = gamed::ACHIEVE_MONEY_USED;
    AchieveData* data = player->GetAchieveData();
    int32_t& value = data->data[type][sub_type];
    if (MAX_DATA_VALUE - value >= money)
    {
        value += money;
    }
    else
    {
        value = MAX_DATA_VALUE;
    }
    NotifyUtil::SendModify(player, 0, type, sub_type, value);
}

int32_t GetInsFinishCount(Player* player, int32_t ins_id)
{
    AchieveData* data = player->GetAchieveData();
    return data->data[gamed::ACHIEVE_INSTANCE][ins_id];
}

} // namespace achieve
