#ifndef GAMED_GS_SUBSYS_PLAYER_ARENA_H_
#define GAMED_GS_SUBSYS_PLAYER_ARENA_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief：player竞技场子系统
 */
class PlayerArena : public PlayerSubSystem
{
public:
	PlayerArena(Player& player);
	virtual ~PlayerArena();

	bool SaveToDB(common::PlayerArenaData* pData);
	bool LoadFromDB(const common::PlayerArenaData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

protected:
    //void CMDHandler_ArenaODQueryData(const C2G::ArenaODQueryData&);
    //void CMDHandler_ArenaODInviteAssist(const C2G::ArenaODInviteAssist&);
    //void CMDHandler_ArenaODAssistSelect(const C2G::ArenaODAssistSelect&);
    //void CMDHandler_ArenaODBuyTicket(const C2G::ArenaODBuyTicket&);
    //void CMDHandler_ArenaODCombatStart(const C2G::ArenaODCombatStart&);
 
private:
    common::PlayerArenaData arena_data;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_ARENA_H_
