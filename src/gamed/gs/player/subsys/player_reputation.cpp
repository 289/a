#include "player_reputation.h"

#include "gamed/client_proto/G2C_proto.h"

#include "gs/global/dbgprt.h"
#include "gs/global/msg_pack_def.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/reputation_templ.h"

namespace gamed
{

using namespace std;
using namespace shared::net;
using namespace common::protocol;
using namespace dataTempl;

#define GetRepTempl(id) s_pDataTempl->QueryDataTempl<ReputationTempl>(id)

PlayerReputation::PlayerReputation(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_REPUTATION, player)
{
	SAVE_LOAD_REGISTER(common::PlayerReputationData, PlayerReputation::SaveToDB, PlayerReputation::LoadFromDB);
}

PlayerReputation::~PlayerReputation()
{
}

bool PlayerReputation::LoadFromDB(const common::PlayerReputationData& data)
{
	for (size_t i = 0; i < data.reputation_list.size(); ++i)
	{
        const PlayerReputationData::reputation_entry& entry = data.reputation_list[i];
        reputation_[entry.reputation_id] = entry.reputation_point;
	}
	return true;
}

bool PlayerReputation::SaveToDB(common::PlayerReputationData* pData)
{
    pData->reputation_list.clear();
    ReputationMap::const_iterator rit = reputation_.begin();
    for (; rit != reputation_.end(); ++rit)
    {
        PlayerReputationData::reputation_entry entry;
        entry.reputation_id = rit->first;
        entry.reputation_point = rit->second;
        pData->reputation_list.push_back(entry);
    }
	return true;
}

void PlayerReputation::RegisterCmdHandler()
{
}

void PlayerReputation::RegisterMsgHandler()
{
}

void PlayerReputation::PlayerGetReputationData() const
{
    G2C::ReputationData packet;
    packet.reputation = reputation_;
    player_.sender()->SendCmd(packet);
}

void PlayerReputation::OpenReputation(int32_t reputation_id)
{
    if (reputation_id == 0)
    {
        return;
    }
    ReputationMap::const_iterator rit = reputation_.find(reputation_id);
    if (rit != reputation_.end())
    {
        return;
    }
    const ReputationTempl* templ = GetRepTempl(reputation_id);
    if (templ == NULL)
    {
        LOG_ERROR << "ReputationTempl not found! reputation_id:" << reputation_id;
        return;
    }
    //ASSERT(templ != NULL);
    reputation_[reputation_id] = templ->init_value;

    G2C::OpenReputation packet;
    packet.reputation_id = reputation_id;
    player_.sender()->SendCmd(packet);
}

int32_t PlayerReputation::GetReputation(int32_t reputation_id) const
{
    ReputationMap::const_iterator rit = reputation_.find(reputation_id);
    return rit == reputation_.end() ? -1 : rit->second;
}

void PlayerReputation::ModifyReputation(int32_t reputation_id, int32_t delta)
{
    if (reputation_id == 0 || delta == 0)
    {
        return;
    }
    ReputationMap::const_iterator rit = reputation_.find(reputation_id);
    if (rit == reputation_.end())
    {
        return;
    }
    int32_t& value = reputation_[reputation_id];
    int32_t new_value = value + delta;
    const ReputationTempl* templ = GetRepTempl(reputation_id);
    if (new_value < templ->min_value)
    {
        new_value = templ->min_value;
    }
    else if (new_value > templ->max_value)
    {
        new_value = templ->max_value;
    }
    delta = new_value - value;
    value = new_value;

    G2C::ModifyReputation packet;
    packet.reputation_id = reputation_id;
    packet.reputation_delta = delta;
    player_.sender()->SendCmd(packet);
}

} // namespace gamed
