#ifndef GAMED_GS_SUBSYS_PLAYER_TRANSFER_H_
#define GAMED_GS_SUBSYS_PLAYER_TRANSFER_H_

#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief 传送子系统
 *    （1）玩家的各种传送方式都在这个子系统实现
 */
class PlayerTransfer : public PlayerSubSystem
{
	static const int kFailureRetryCounter    = 15; // 单位：s
	static const int kFailureTryPlaneSwitch  = 1;  // 单位：s
	static const int kWaitingClientReply     = 2;  // 单位：s
	static const int kWaitingMasterReply     = 3;  // 单位：s
public:
	PlayerTransfer(Player& player);
	virtual ~PlayerTransfer();

	virtual void OnHeartbeat(time_t cur_time);
	virtual void OnRelease();
	virtual void RegisterMsgHandler();
	virtual void RegisterCmdHandler();

	void ChangeMapError(bool is_change_gs_err);


protected:
	void CMDHandler_TransferPrepareFinish(const C2G::TransferPrepareFinish&);

	int  MSGHandler_RegionTransfer(const MSG&);
	int  MSGHandler_PlayerSwitchError(const MSG&); // 本gs内地图切换失败
	int  MSGHandler_InsTransferStart(const MSG&);
	int  MSGHandler_BGTransferStart(const MSG&);


private:
	void ExecuteTransport();
	bool IsLocalMap(int32_t target_wid) const;
	bool CheckTransferCond(int32_t area_elem_id);
	inline void reset_trans_param();


private:
	msg_player_region_transport transport_param_;
	int32_t retry_counter_;
	int32_t waiting_counter_;
};

///
/// inline func
///
inline void PlayerTransfer::reset_trans_param()
{
	memset(&transport_param_, 0, sizeof(transport_param_));
}

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_TRANSFER_H_
