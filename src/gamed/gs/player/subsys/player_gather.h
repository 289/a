#ifndef GAMED_GS_SUBSYS_PLAYER_GATHER_H_
#define GAMED_GS_SUBSYS_PLAYER_GATHER_H_

#include "gs/player/player_subsys.h"


namespace dataTempl 
{
	class MineTempl;
} // namespace dataTempl


namespace gamed {

/**
 * @brief：player采集子系统
 */
class PlayerGather : public PlayerSubSystem
{
	static const int kHeartbeatTimeout = 10;
public:
	PlayerGather(Player& player);
	virtual ~PlayerGather();

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

protected:
	void CMDHandler_GatherMaterial(const C2G::GatherMaterial&);
	void CMDHandler_GatherMiniGameResult(const C2G::GatherMiniGameResult&);

	int  MSGHandler_GatherReply(const MSG&);
	int  MSGHandler_MineHasBeenRob(const MSG&);
	int  MSGHandler_GatherResult(const MSG&);
	int  MSGHandler_DramaGather(const MSG&);
	int  MSGHandler_DramaGatherMiniGameResult(const MSG&);
	
private:
	inline int32_t GetNextGatherSeqNo();
	bool CheckRequireCondByTid(int32_t tid);
	bool CheckRequireCondition(const dataTempl::MineTempl* pdata);
	bool CheckMiniGameCondition(const dataTempl::MineTempl* pdata);
	void ConsumeMiniGameCond(const dataTempl::MineTempl* pdata);
	void DeliverAwardItem(const dataTempl::MineTempl* pdata);
	void DeliverTask(const dataTempl::MineTempl* pdata);
	void RecycleTask(const dataTempl::MineTempl* pdata);
	void GatherMaterialStart(const XID& target, int32_t tid);
	void HandleGatherMiniGameResult(const XID& target, int32_t gather_seq_no, bool is_success);

private:
	int32_t cur_gather_seq_no_;
};

///
/// inline func
///
inline int32_t PlayerGather::GetNextGatherSeqNo()
{
	return ++cur_gather_seq_no_;
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_GATHER_H_
