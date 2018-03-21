#include "achieve_if.h"

#include "game_module/achieve/include/achieve_data.h"
#include "game_module/task/include/task_data.h"
#include "gs/global/dbgprt.h"

#include "player.h"
#include "player_sender.h"

namespace gamed {

PlayerAchieveIf::PlayerAchieveIf(Player* player)
    : pplayer_(player)
{
}

PlayerAchieveIf::~PlayerAchieveIf()
{
}

int64_t PlayerAchieveIf::GetId() const
{
    return pplayer_->role_id();
}

int32_t PlayerAchieveIf::GetRoleClass() const
{
    return pplayer_->role_class();
}

int32_t PlayerAchieveIf::GetLastLoginTime() const
{
    return pplayer_->last_login_time();
}

bool PlayerAchieveIf::CanDeliverItem(int8_t package_type, int32_t item_types) const
{
    return true;
}

void PlayerAchieveIf::DeliverAchievePoint(int32_t point)
{
}

achieve::ActiveAchieve* PlayerAchieveIf::GetActiveAchieve()
{
    return pplayer_->GetActiveAchieve();
}

achieve::FinishAchieve* PlayerAchieveIf::GetFinishAchieve()
{
    return pplayer_->GetFinishAchieve();
}

achieve::AchieveData* PlayerAchieveIf::GetAchieveData()
{
    return pplayer_->GetAchieveData();
}

void PlayerAchieveIf::SendNotify(uint16_t type, const void* buf, size_t size)
{
    pplayer_->sender()->AchieveNotifyClient(type, buf, size);
}

void PlayerAchieveIf::DeliverItem(int32_t item_id, int32_t num)
{
	pplayer_->GainItem(item_id, num);
}

void PlayerAchieveIf::DeliverTitle(int32_t title_id)
{
    pplayer_->GainTitle(title_id);
}

Player* PlayerAchieveIf::GetPlayer()
{
    return pplayer_;
}

static int32_t GetBaseData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_BASE_LEVEL:
        return player->GetLevel();
    case ACHIEVE_BASE_COMBAT_VALUE:
        return player->CalcCombatValue();
    default:
        return -1;
    }
}

static int32_t GetSocialData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_SOCIAL_FRIEND:
        return player->GetFriendNum();
    case ACHIEVE_SOCIAL_ENEMY:
        return player->GetEnemyNum();
    default:
        return -1;
    }
}

static int32_t GetPetData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_PET_NUM:
        return player->GetCombatPetNum(0, 0, 0);
    case ACHIEVE_PET_POWER:
        {
            int32_t power = 0, power_cap = 0, speed = 0;
            player->QueryPetPowerInfo(power, power_cap, speed);
            return power_cap;
        }
    default:
        return player->GetCombatPetNum(0, 0, sub_type - 1);
    }
}

static int32_t GetMapData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    task::TaskData* data = player->GetActiveTask();
    if (data->IsExist(sub_type))
    {
        return 1;
    }
    data = player->GetFinishTask();
    return data->IsExist(sub_type) ? 1 : 0;
}

static int32_t GetScoreData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_SCORE_TOTAL:
        return player->GetScoreTotal();
    case ACHIEVE_SCORE_USED:
        return player->GetScoreUsed();
    default:
        return -1;
    }
}
/*
static int32_t GetCashData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_CASH_TOTAL:
        return player->GetCashTotal();
    case ACHIEVE_CASH_USED:
        return player->GetCashUsed();
    default:
        return -1;
    }
}
*/
static int32_t GetMountData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_MOUNT_CATEGORY:
        return player->GetMountCategory();
    default:
        return player->GetMountEquipLevel(sub_type - 1);
    }
}

static int32_t GetStarData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    switch (sub_type)
    {
    case ACHIEVE_STAR_SPARK:
        return player->GetSparkNum();
    default:
        return player->IsStarOpen(sub_type);
    }
}

static int32_t GetCardData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    Player* player = achieve_if->GetPlayer();
    if (sub_type == ACHIEVE_CARD_NUM)
    {
        return player->GetEquipCardNum(0, 0);
    }
    else if (sub_type >= 1000 && sub_type < 2000)
    {
        return player->GetEquipCardNum(sub_type - 1000, 0);
    }
    else if (sub_type >= 2000 && sub_type < 3000)
    {
        return player->GetEquipCardNum(0, sub_type - 2000);
    }
    else
    {
        return -1;
    }
}

static int32_t GetData(PlayerAchieveIf* achieve_if, int32_t type, int32_t sub_type)
{
    achieve::AchieveData* data = achieve_if->GetAchieveData();
    achieve::SubAchieveDataMap& sub_map = data->data[type];
    achieve::SubAchieveDataMap::const_iterator sit = sub_map.find(sub_type);
    return sit == sub_map.end() ? 0 : sit->second;
}

static int32_t GetTaskData(PlayerAchieveIf* achieve_if, int32_t sub_type)
{
    if (sub_type >= task::QUALITY_WHITE && sub_type <= task::QUALITY_GOLD)
    {
        return GetData(achieve_if, ACHIEVE_TASK, sub_type);
    }
    Player* player = achieve_if->GetPlayer();
    task::TaskData* data = player->GetFinishTask();
    task::TaskEntry* entry = data->GetEntry(sub_type);
    if (entry != NULL)
    {
        return 1;
    }
    data = player->GetFinishTimeTask();
    entry = data->GetEntry(sub_type);
    if (entry == NULL)
    {
        return 0;
    }
    int32_t* num = (int32_t*)(&(entry->data[0]));
    return num[0];
}

int32_t PlayerAchieveIf::GetAchieveData(int32_t type, int32_t sub_type)
{
    if (!IS_NORMAL_MAP(pplayer_->world_id()))
    {
        if ((type == ACHIEVE_BASE && sub_type == ACHIEVE_BASE_COMBAT_VALUE) || 
            type == ACHIEVE_PROP)
        {
            return 0;
        }
    }
    switch (type)
    {
    case ACHIEVE_PROP:
        return pplayer_->GetMaxProp(sub_type);
    case ACHIEVE_BASE:
        return GetBaseData(this, sub_type);
    case ACHIEVE_SKILL:
        return pplayer_->IsSkillExist(sub_type);
    case ACHIEVE_SOCIAL:
        return GetSocialData(this, sub_type);
    case ACHIEVE_PET:
        return GetPetData(this, sub_type);
    case ACHIEVE_REPUTATION:
        return pplayer_->GetReputation(sub_type);
    case ACHIEVE_MAP:
        return GetMapData(this, sub_type);
    case ACHIEVE_TASK:
        return GetTaskData(this, sub_type);
    case ACHIEVE_SCORE:
        return GetScoreData(this, sub_type);
    case ACHIEVE_MOUNT:
        return GetMountData(this, sub_type);
    case ACHIEVE_STAR:
        return GetStarData(this, sub_type);
    case ACHIEVE_CARD:
        return GetCardData(this, sub_type);
    //case ACHIEVE_CASH:
        //return GetCashData(this, sub_type);
    case ACHIEVE_MONSTER:
    case ACHIEVE_LOGIN:
    case ACHIEVE_REFINE:
    case ACHIEVE_INSTANCE:
    case ACHIEVE_MONEY:
        return GetData(this, type, sub_type);
    default:
        ASSERT(false);
    }
}

} // namespace gamed
