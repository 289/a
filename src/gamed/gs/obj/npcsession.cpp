#include "npcsession.h"

#include "gs/global/game_util.h"

#include "npc.h"


namespace gamed {

///
/// NpcSession
///
NpcSession::NpcSession(Npc* npc)
	: ActiveSession(npc),
	  pnpc_(npc),
	  ai_task_id_(-1)
{
}

NpcSession::~NpcSession()
{
}

bool NpcSession::StartSession(const ActiveSession* next_ses)
{
	if (OnStartSession(next_ses))
	{
		NotifySessionStart();
		return true;
	}

	return false;
}
	
bool NpcSession::EndSession()
{
	int retcode = OnEndSession();
	if (retcode == NSRC_SUCCESS)
	{
		NotifySessionEnd(NSRC_SUCCESS);
		return true;
	}

	NotifySessionEnd(retcode);
	return false;
}

bool NpcSession::TerminateSession() 
{ 
	EndSession(); 
	return true; 
}

void NpcSession::NotifySessionStart()
{
	pnpc_->NPCSessionStart(ai_task_id_, session_id());
}
	
void NpcSession::NotifySessionEnd(int retcode)
{
	pnpc_->NPCSessionEnd(ai_task_id_, session_id(), retcode);
}


///
/// NpcCombatSession
///
bool NpcCombatSession::OnStartSession(const ActiveSession* next_ses) 
{ 
	int tick = second_to_tick(kMaxCombatRetainSecs);
	AutoSetTimer(tick, 1);
	return true; 
}

} // namespace gamed
