#ifndef GAMED_GS_SUBSYS_PLAYER_DRAMA_H_
#define GAMED_GS_SUBSYS_PLAYER_DRAMA_H_

#include "gs/player/player_subsys.h"


namespace gamed {

class ProviderList;

/**
 * @brief：player剧情子系统，负责处理剧情发起战斗、npc服务等
 */
class PlayerDrama : public PlayerSubSystem
{
	static const int kGatherTimeoutDelay  = 5; // 单位:s
public:
	PlayerDrama(Player& player);
	virtual ~PlayerDrama();

	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

protected:
	void CMDHandler_DramaTriggerCombat(const C2G::DramaTriggerCombat&);
	void CMDHandler_DramaServiceServe(const C2G::DramaServiceServe&);
	void CMDHandler_DramaGatherMaterial(const C2G::DramaGatherMaterial&);
	void CMDHandler_DramaGatherMiniGameResult(const C2G::DramaGatherMiniGameResult&);

	int  MSGHandler_GatherRequest(const MSG&);
	int  MSGHandler_GatherCancel(const MSG&);
	int  MSGHandler_GatherComplete(const MSG&);

private:
	int32_t GetNpcCatVision(int32_t tid);
	bool    CreateService(int32_t npc_tid);
	void    ResetGatherInfo();

private:
	typedef std::map<int32_t, ProviderList*> NpcServiceMap;
	NpcServiceMap  npc_service_map_;

	int32_t cur_mine_tid_;
	int32_t cur_gather_seq_no_;
	int32_t gather_timeout_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_DRAMA_H_
