#include "player_cooldown.h"

#include "common/obj_data/gen/player/cooldown_data.pb.h"
#include "gamed/client_proto/G2C_proto.h"

#include "gs/global/glogger.h"
#include "gs/global/timer.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

SHARED_STATIC_ASSERT((int)playerdef::PCD_INDEX_INVALID  == (int)G2C::PCD_INDEX_INVALID);
SHARED_STATIC_ASSERT((int)playerdef::PCD_INDEX_TIDY_INV == (int)G2C::PCD_INDEX_TIDY_INV);
SHARED_STATIC_ASSERT((int)playerdef::PCD_INDEX_MAX      == (int)G2C::PCD_INDEX_MAX);

///
/// PlayerCoolDown
///
PlayerCoolDown::PlayerCoolDown(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_COOLDOWN, player)
{
    SAVE_LOAD_REGISTER(common::PlayerCoolDownData, PlayerCoolDown::SaveToDB, PlayerCoolDown::LoadFromDB);
}

PlayerCoolDown::~PlayerCoolDown()
{
}

bool PlayerCoolDown::SaveToDB(common::PlayerCoolDownData* pdata)
{
    CoolDownMan::CoolDownMap cd_map;
    cooldown_man_.SaveToData(cd_map);

    if (cd_map.size() > 0)
    {
        common::scalable::CoolDownList scalable_data;
        CoolDownMan::CoolDownMap::iterator it = cd_map.begin();
        for (; it != cd_map.end(); ++it)
        {
            common::scalable::CoolDownList::Entry* ent = scalable_data.add_cd_entry();
            ent->set_id(it->first);
            ent->set_interval(it->second.interval);
            ent->set_expire_time(it->second.expire_time);
        }

        std::string& tmpbuf = pdata->cd_list;
        tmpbuf.resize(scalable_data.ByteSize());
        if (!scalable_data.SerializeToArray((void*)tmpbuf.c_str(), tmpbuf.size()))
        {
            GLog::log("PlayerCoolDown::SaveToDB Serialize error! roleid:%ld", player_.role_id());
            return false;
        }
    }
    return true;
}

bool PlayerCoolDown::LoadFromDB(const common::PlayerCoolDownData& data)
{
    if (data.cd_list.size() > 0)
    {
        common::scalable::CoolDownList scalable_data;
        const std::string& bufRef = data.cd_list;
        if (!scalable_data.ParseFromArray(bufRef.c_str(), bufRef.size()))
        {
            GLog::log("PlayerCoolDown::LoadFromDB error! roleid: ", player_.role_id());
            return false;
        }

        CoolDownMan::CoolDownMap cd_map;
        for (int i = 0; i < scalable_data.cd_entry_size(); ++i)
        {
            CoolDownMan::cd_entry ent;
            ent.interval    = scalable_data.cd_entry(i).interval();
            ent.expire_time = scalable_data.cd_entry(i).expire_time();
            cd_map[scalable_data.cd_entry(i).id()] = ent;
        }
        
        cooldown_man_.LoadFromData(cd_map);
    }
    return true;
}

void PlayerCoolDown::PlayerGetCoolDownData()
{
    CoolDownMan::CoolDownMap cd_map;
    cooldown_man_.SaveToData(cd_map);

    int64_t now = g_timer->GetSysTimeMsecs();
	G2C::CoolDownData packet;
    CoolDownMan::CoolDownMap::iterator it = cd_map.begin();
    for (; it != cd_map.end(); ++it)
    {
        G2C::CDEntry entry;
		entry.index    = it->first;
		entry.interval = it->second.expire_time - now;
		packet.cd_vec.push_back(entry);
    }
    player_.sender()->SendCmd(packet);
}

void PlayerCoolDown::SetCoolDown(int id, int msec)
{
    cooldown_man_.SetCoolDown(id, msec);
	player_.sender()->SetCoolDown(id, msec);
}

void PlayerCoolDown::ClrCoolDown(int id)
{
    cooldown_man_.ClrCoolDown(id);
	player_.sender()->SetCoolDown(id, 0);
}

bool PlayerCoolDown::TestCoolDown(int index)
{
	ASSERT(index > 0 && index < PCD_INDEX_MAX);
    return cooldown_man_.TestCoolDown(index);
}

bool PlayerCoolDown::TestItemCoolDown(int cd_group_id, int item_id)
{
    if (item_id <= 0)
        return false;

    if (cd_group_id > 0 && !cooldown_man_.TestCoolDown(cd_group_id))
        return false;

    if (!cooldown_man_.TestCoolDown(item_id))
        return false;

    return true;
}

} // namespace gamed
