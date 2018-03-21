#include "player_star.h"

#include "gs/global/dbgprt.h"
#include "gs/player/subsys_if.h"
#include "gs/global/randomgen.h"
#include "gs/template/data_templ/star_templ.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/player/player_sender.h"

namespace gamed
{

using namespace std;
using namespace shared::net;
using namespace common::protocol;
using namespace dataTempl;

#define GetStarTempl(id) s_pDataTempl->QueryDataTempl<StarTempl>(id)

PlayerStar::PlayerStar(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_STAR, player)
{
	SAVE_LOAD_REGISTER(common::PlayerStarData, PlayerStar::SaveToDB, PlayerStar::LoadFromDB);
}

PlayerStar::~PlayerStar()
{
}

void PlayerStar::AddSparkProp(int32_t prop, int32_t value)
{
    player_.IncPropPoint(prop, value);
}

bool PlayerStar::LoadFromDB(const common::PlayerStarData& data)
{
    star_map.clear();
    for (size_t i = 0; i < data.star_list.size(); ++i)
    {
        const common::PlayerStarData::star_entry& entry = data.star_list[i];
        if (entry.spark_index == -1)
        {
            star_map[entry.star_id].clear();
        }
        else
        {
            star_map[entry.star_id][entry.spark_index] = entry.spark_locked;
        }
    }
    // 让星盘生效
    const StarTempl* templ = NULL;
    StarMap::const_iterator sit = star_map.begin();
    for (; sit != star_map.end(); ++sit)
    {
        templ = GetStarTempl(sit->first);
        for (size_t i = 0; i < sit->second.size(); ++i)
        {
            const SparkProp& prop = templ->prop_list[i];
            AddSparkProp(prop.prop, prop.value);
        }
    }
    return true;
}

bool PlayerStar::SaveToDB(common::PlayerStarData* pData)
{
    pData->star_list.clear();
    StarMap::const_iterator sit = star_map.begin();
    for (; sit != star_map.end(); ++sit)
    {
        const SparkMap& spark_map = sit->second;
        common::PlayerStarData::star_entry entry;
        if (spark_map.empty())
        {
            entry.star_id = sit->first;
            entry.spark_index = -1;
            entry.spark_locked = false;
            pData->star_list.push_back(entry);
        }
        else
        {
            SparkMap::const_iterator pit = spark_map.begin();
            for (; pit != spark_map.end(); ++pit)
            {
                entry.star_id = sit->first;
                entry.spark_index = pit->first;
                entry.spark_locked = pit->second;
                pData->star_list.push_back(entry);
            }
        }
    }
    return true;
}

void PlayerStar::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::ActivateSpark, PlayerStar::CMDHandler_ActivateSpark);
}

void PlayerStar::RegisterMsgHandler()
{
}

void PlayerStar::CMDHandler_ActivateSpark(const C2G::ActivateSpark& packet)
{
    ActivateSpark(packet.star_id);
}

void PlayerStar::PlayerGetStarData() const
{
    G2C::StarData packet;
    packet.star = star_map;
    player_.sender()->SendCmd(packet);
}

bool PlayerStar::CanOpenStar(int32_t star_id) const
{
    if (star_id == 0 || star_map.find(star_id) != star_map.end())
    {
        return false;
    }
    return GetStarTempl(star_id) != NULL;
}

void PlayerStar::OpenStar(int32_t star_id)
{
    if (!CanOpenStar(star_id))
    {
        return;
    }
    star_map[star_id];
    G2C::OpenStarRe packet;
    packet.star_id = star_id;
    player_.sender()->SendCmd(packet);
}

void PlayerStar::OpenStar(const vector<int32_t>& star_vec)
{
    for (size_t i = 0; i < star_vec.size(); ++i)
    {
        OpenStar(star_vec[i]);
    }
}

void PlayerStar::CloseStar(int32_t star_id)
{
    if (star_id == 0)
    {
        star_map.clear();
    }
    else
    {
        star_map.erase(star_id);
    }
}

static void ClearSparkLocked(SparkMap& spark_map)
{
    SparkMap::iterator pit = spark_map.begin();
    for (; pit != spark_map.end(); ++pit)
    {
        pit->second = false;
    }
}

static int8_t RandSelectSpark(SparkMap& spark_map, int32_t total_num)
{
    // 获取本次随到的星火
    vector<int8_t> valid_index;
    for (int8_t i = 0; i < total_num; ++i)
    {
        SparkMap::const_iterator pit = spark_map.find(i);
        // 锁定了本次不参加随机
        if (pit != spark_map.end() && pit->second)
        {
            continue;
        }
        valid_index.push_back(i);
    }
    int32_t rand_num = mrand::Rand(0, valid_index.size() - 1);
    int8_t spark_index = valid_index[rand_num];
    // 试图点亮星火
    if (spark_map.find(spark_index) != spark_map.end())
    {
        // 锁定星火，下次不参加随机
        spark_map[spark_index] = true;
    }
    else
    {
        // 点亮星火，清除锁定标记
        spark_map[spark_index] = false;
        ClearSparkLocked(spark_map);
    }
    return spark_index;
}

static void SendActivateSparkRe(Player& player, int32_t star_id, int8_t spark_index)
{
    G2C::ActivateSparkRe packet;
    packet.star_id = star_id;
    packet.spark_index = spark_index;
    player.sender()->SendCmd(packet);
}

bool PlayerStar::CanActivateSpark(const StarTempl* templ) const
{
    if (templ == NULL) // 对应的星盘是否存在
    {
        return false;
    }
    StarMap::const_iterator sit = star_map.find(templ->templ_id);
    if (sit == star_map.end()) // 星盘还未开启
    {
        return false;
    }
    if ((int32_t)sit->second.size() >= templ->spark_num) // 是否已经激活满
    {
        return false;
    }
    if (!player_.CheckMoney(templ->spark_activate_money)) // 金钱不足
    {
        SendActivateSparkRe(player_, templ->templ_id, -1);
        return false;
    }
    if (!player_.CheckScore(templ->spark_activate_score)) // 学分不足
    {
        SendActivateSparkRe(player_, templ->templ_id, -1);
        return false;
    }
    return true;
}

void PlayerStar::ActivateSpark(int32_t star_id)
{
    const StarTempl* templ = GetStarTempl(star_id);
    // 检查激活条件
    if (!CanActivateSpark(templ))
    {
        return;
    }

    // 随机选择星火并通知客户端
    SparkMap& spark_map = star_map[star_id];
    int8_t spark_index = RandSelectSpark(spark_map, templ->spark_num);
    SendActivateSparkRe(player_, star_id, spark_index);
    player_.SpendMoney(templ->spark_activate_money);
    player_.SpendScore(templ->spark_activate_score);

    // 增加星火激活效果
    if (!spark_map[spark_index])
    {
        const SparkProp& prop = templ->prop_list[spark_map.size() - 1];
        AddSparkProp(prop.prop, prop.value);
    }

    // 全部激活后可能会开启其他星盘
    if ((int32_t)spark_map.size() >= templ->spark_num)
    {
        OpenStar(templ->activate_star);
    }
}

bool PlayerStar::AllSparkActivate(int32_t star_id) const
{
    StarMap::const_iterator sit = star_map.find(star_id);
    if (sit == star_map.end())
    {
        return false;
    }
    const StarTempl* templ = GetStarTempl(star_id);
    return (int)sit->second.size() >= templ->spark_num;
}

int32_t PlayerStar::GetFullActivateStarNum() const
{
    int32_t full_activate_num = 0;
    StarMap::const_iterator sit = star_map.begin();
    for (; sit != star_map.end(); ++sit)
    {
        const StarTempl* templ = GetStarTempl(sit->first);
        if (templ != NULL && (int32_t)sit->second.size() >= templ->spark_num)
        {
            ++full_activate_num;
        }
    }
    return full_activate_num;
}

bool PlayerStar::IsStarOpen(int32_t star_id) const
{
    StarMap::const_iterator it = star_map.find(star_id);
    return it != star_map.end();
}

int32_t PlayerStar::GetSparkNum() const
{
    int32_t spark_num = 0;
    StarMap::const_iterator it = star_map.begin();
    for (; it != star_map.end(); ++it)
    {
        spark_num += it->second.size();
    }
    return spark_num;
}

} // namespace gamed
