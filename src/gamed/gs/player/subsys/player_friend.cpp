#include "player_friend.h"

#include "gamed/client_proto/G2C_proto.h"

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"
#include "gs/template/data_templ/service_npc_templ.h"
#include "gs/template/data_templ/templ_manager.h"

#include "common/protocol/gen/G2M/query_msg.pb.h"

namespace gamed
{

#define MAX_FRIEND_NUM 100
#define MAX_ENEMY_NUM 100
#define MAX_BLACK_NUM 100

using namespace common::protocol;
using namespace dataTempl;

PlayerFriend::PlayerFriend(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_FRIEND, player)
{
	SAVE_LOAD_REGISTER(common::PlayerFriendData, PlayerFriend::SaveToDB, PlayerFriend::LoadFromDB);
}

PlayerFriend::~PlayerFriend()
{
}

bool PlayerFriend::SaveToDB(common::PlayerFriendData* pData)
{
    pData->friend_list.clear();

    common::PlayerFriendData::friend_entry entry;
    FriendMap::const_iterator it = friend_list_.begin();
    for (; it != friend_list_.end(); ++it)
    {
        entry.roleid = it->first;
        entry.type = it->second;
        pData->friend_list.push_back(entry);
    }

    it = enemy_list_.begin();
    for (; it != enemy_list_.end(); ++it)
    {
        entry.roleid = it->first;
        entry.type = it->second;
        pData->friend_list.push_back(entry);
    }

    it = black_list_.begin();
    for (; it != black_list_.end(); ++it)
    {
        entry.roleid = it->first;
        entry.type = it->second;
        pData->friend_list.push_back(entry);
    }
    return true;
}

bool PlayerFriend::LoadFromDB(const common::PlayerFriendData& data)
{
    for (size_t i = 0; i < data.friend_list.size(); ++i)
    {
        const common::PlayerFriendData::friend_entry& entry = data.friend_list[i];
        if (entry.type & G2C::FriendInfo::FRIEND_ENEMY)
        {
            enemy_list_[entry.roleid] = entry.type;
        }
        else if (entry.type & G2C::FriendInfo::FRIEND_BLACKLIST)
        {
            black_list_[entry.roleid] = entry.type;
        }
        else
        {
            friend_list_[entry.roleid] = entry.type;
        }
    }
    return true;
}

void PlayerFriend::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::AddFriend, PlayerFriend::CMDHandler_AddFriend);
	REGISTER_NORMAL_CMD_HANDLER(C2G::DeleteFriend, PlayerFriend::CMDHandler_DeleteFriend);
	REGISTER_NORMAL_CMD_HANDLER(C2G::QueryRoleInfo, PlayerFriend::CMDHandler_QueryRoleInfo);
}

void PlayerFriend::RegisterMsgHandler()
{
}

bool PlayerFriend::IsFriend(int64_t id) const
{
    FriendMap::const_iterator it = friend_list_.find(id);
    if (it != friend_list_.end())
    {
        return it->second == 0;
    }
    it = enemy_list_.find(id);
    if (it != enemy_list_.end())
    {
        return !it->second == 0;
    }
    return false;
}

int32_t PlayerFriend::GetFriendNum() const
{
    return friend_list_.size();
}

int32_t PlayerFriend::GetEnemyNum() const
{
    return enemy_list_.size();
}

void PlayerFriend::AddNPCFriend(int32_t id)
{
    C2G::AddFriend packet;
    packet.roleid = id;
    packet.flag = G2C::FriendInfo::FRIEND_NPC;
    CMDHandler_AddFriend(packet);
}

void PlayerFriend::DelNPCFriend(int32_t id)
{
    C2G::DeleteFriend packet;
    packet.roleid = id;
    CMDHandler_DeleteFriend(packet);
}

void PlayerFriend::OnlineNPCFriend(int32_t id)
{
    FriendMap::iterator it = friend_list_.find(id);
    if (it != friend_list_.end() && (it->second & G2C::FriendInfo::FRIEND_NPC))
    {
        it->second &= ~(G2C::FriendInfo::FRIEND_NPC_OFFLINE);
        G2C::NPCFriendStatusChange packet;
        packet.friend_list.push_back(G2C::FriendInfo(id, it->second));
        player_.sender()->SendCmd(packet);
    }
}

void PlayerFriend::OfflineNPCFriend(int32_t id)
{
    FriendMap::iterator it = friend_list_.find(id);
    if (it != friend_list_.end() && (it->second & G2C::FriendInfo::FRIEND_NPC))
    {
        it->second |= G2C::FriendInfo::FRIEND_NPC_OFFLINE;
        G2C::NPCFriendStatusChange packet;
        packet.friend_list.push_back(G2C::FriendInfo(id, it->second));
        player_.sender()->SendCmd(packet);
    }
}

void PlayerFriend::PlayerGetFriendData()
{
    G2C::FriendData packet;
    FriendMap::const_iterator it = friend_list_.begin();
    for (; it != friend_list_.end(); ++it)
    {
        packet.friend_list.push_back(G2C::FriendInfo(it->first, it->second));
    }
    it = enemy_list_.begin();
    for (; it != enemy_list_.end(); ++it)
    {
        packet.enemy_list.push_back(G2C::FriendInfo(it->first, it->second));
    }
    it = black_list_.begin();
    for (; it != black_list_.end(); ++it)
    {
        packet.black_list.push_back(G2C::FriendInfo(it->first, it->second));
    }
    if (!packet.friend_list.empty() || !packet.enemy_list.empty() || !packet.black_list.empty())
    {
        player_.sender()->SendCmd(packet);
    }
}

void PlayerFriend::CMDHandler_AddFriend(const C2G::AddFriend& packet)
{
    // 跨服检查，玩家必须是同服的
    bool npc = packet.flag & G2C::FriendInfo::FRIEND_NPC;
    if (!npc && Gmatrix::IsCrossRealmServer())
    {
        if (!Gmatrix::IsInSameServer(player_.role_id(), packet.roleid))
        {
            player_.sender()->ErrorMessage(G2C::ERR_NOT_IN_SAME_SERVER);
            return;
        }
    }

    // 处理协议
    bool enemy = packet.flag & G2C::FriendInfo::FRIEND_ENEMY;
    bool black = packet.flag & G2C::FriendInfo::FRIEND_BLACKLIST;
    if ((npc && enemy) || (npc && black) || 
        (!black && enemy && enemy_list_.size() >= MAX_ENEMY_NUM) || 
        (!black && !enemy && friend_list_.size() >= MAX_FRIEND_NUM) || 
        (black && black_list_.size() >= MAX_BLACK_NUM) || 
        (npc && s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(packet.roleid) == NULL))
    {
        return;
    }
    if (black)
    {
        if (black_list_.find(packet.roleid) != black_list_.end())
        {
            return;
        }
        friend_list_.erase(packet.roleid);
        enemy_list_.erase(packet.roleid);
        black_list_[packet.roleid] |= G2C::FriendInfo::FRIEND_BLACKLIST;
    }
    else if (!black && enemy)
    {
        if (enemy_list_.find(packet.roleid) != enemy_list_.end())
        {
            return;
        }
        black_list_.erase(packet.roleid);
        friend_list_.erase(packet.roleid);
        enemy_list_[packet.roleid] |= G2C::FriendInfo::FRIEND_ENEMY;
    }
    else if (!black && !enemy)
    {
        if (friend_list_.find(packet.roleid) != friend_list_.end())
        {
            return;
        }
        black_list_.erase(packet.roleid);
        enemy_list_.erase(packet.roleid);
        int8_t& type = friend_list_[packet.roleid];
        if (npc)
        {
            type |= G2C::FriendInfo::FRIEND_NPC;
        }
        else
        {
            type &= ~(G2C::FriendInfo::FRIEND_NPC);
        }
    }

    G2C::AddFriendRe reply;
    reply.roleid = packet.roleid;
    reply.flag = packet.flag;
    reply.name = packet.name;
    player_.sender()->SendCmd(reply);
}

void PlayerFriend::CMDHandler_DeleteFriend(const C2G::DeleteFriend& packet)
{
    G2C::DeleteFriendRe reply;
    reply.roleid = packet.roleid;
    if (friend_list_.find(packet.roleid) != friend_list_.end())
    {
        friend_list_.erase(packet.roleid);
        player_.sender()->SendCmd(reply);
    }
    else if (enemy_list_.find(packet.roleid) != enemy_list_.end())
    {
        enemy_list_.erase(packet.roleid);
        player_.sender()->SendCmd(reply);
    }
    else if (black_list_.find(packet.roleid) != black_list_.end())
    {
        black_list_.erase(packet.roleid);
        player_.sender()->SendCmd(reply);
    }
}

void PlayerFriend::CMDHandler_QueryRoleInfo(const C2G::QueryRoleInfo& packet)
{
    G2M::QueryRoleInfo proto;
    proto.set_querier(player_.role_id());
    for (size_t i = 0; i < packet.query_list.size(); ++i)
    {
        proto.add_role_list(packet.query_list[i]);
    }
    if (proto.role_list_size() != 0)
    {
		player_.sender()->SendToMaster(proto);
    }
}

} // namespace gamed
