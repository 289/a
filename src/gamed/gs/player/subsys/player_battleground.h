#ifndef GAMED_GS_SUBSYS_PLAYER_BATTLEGROUND_H_
#define GAMED_GS_SUBSYS_PLAYER_BATTLEGROUND_H_

#include "gs/player/player_subsys.h"
#include "gs/scene/world_def.h"


namespace dataTempl {
	class BattleGroundTempl;
} // namespace dataTempl


namespace common {
namespace protocol {

	namespace global {
		class BattleGroundInfo;
	} // namespace global

} // namespace protocol
} // namespace common


namespace gamed {

/**
 * @brief：player战场子系统
 */
class PlayerBattleGround : public PlayerSubSystem
{
    static const int kBGRequestTimeout = 10; // 单位秒
public:
	PlayerBattleGround(Player& player);
	virtual ~PlayerBattleGround();

    bool SaveToDB(common::BattleGroundData* pdata);
	bool LoadFromDB(const common::BattleGroundData& data);

	virtual void OnHeartbeat(time_t cur_time);
	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void OnRelease();
    virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    bool CheckBattleGroundCond(const playerdef::BGInfoCond& info) const;
	bool ConsumeBattleGroundCond(int32_t bg_tid);
	void SetBattleGroundInfo(const world::battleground_info& info);
	void ResetBattleGroundInfo();
	bool GetRuntimeBGInfo(MapID target_world_id, common::protocol::global::BattleGroundInfo& bginfo) const;
	bool GetBattleGroundPos(A2DVECTOR& pos) const;
	bool QueryBGEntrancePos(A2DVECTOR& pos) const;
    void EnterPveBattleGround(int32_t bg_tid) const; // 进入PVE战场
    bool IsChangeMapFillMapTeam(MapID target_world_id) const;
    void QuitBattleGround(int32_t bg_tid) const;


protected:
    void CMDHandler_PlayerQuitBattleGround(const C2G::PlayerQuitBattleGround&);
    void CMDHandler_GetBattleGroundData(const C2G::GetBattleGroundData&);

	int  MSGHandler_BGTransferPrepare(const MSG&);
    int  MSGHandler_EnterBGReply(const MSG&);


public:
    struct BGDataInfo
	{
		BGDataInfo() 
			: is_consumed(false),
			  bg_type(-1)
		{ }

		void clear()
		{
			is_consumed = false;
			bg_type     = -1;
			self_pos    = A2DVECTOR();
			info.clear();
		}

		BGDataInfo& operator=(const BGDataInfo& rhs)
		{
			is_consumed = rhs.is_consumed;
			bg_type     = rhs.bg_type;
			self_pos    = rhs.self_pos;
			info        = rhs.info;
			return *this;
		}

		bool      is_consumed;
		int32_t   bg_type;
		A2DVECTOR self_pos;
		world::battleground_info info;
	};


private:
    struct BGRequestInfo
	{
		BGRequestInfo() { clear(); }

		void clear()
		{
			bg_tid  = 0;
			time_out = 0;
		}

		bool has_request() const
		{
			return bg_tid != 0;
		}

		bool is_countdown() const
		{
			return time_out > 0;
		}

		int32_t bg_tid;
		int32_t time_out;
	};


private:
    void ResetCurBGInfo();
	bool CanEnterBattleGround() const;
	bool CheckEnterCondition(const dataTempl::BattleGroundTempl* pdata) const;
	bool CheckCondIsConsumed(int32_t bg_templ_id) const;
	void CleanUpSpecBGInfo(int32_t bg_tid);
	void ConsumeEnterCond(const dataTempl::BattleGroundTempl* pdata);
    A2DVECTOR GetPositionByFaction(const dataTempl::BattleGroundTempl* ptempl, int faction) const;


private:
    BGDataInfo      cur_bg_;

    // runtime info
	common::protocol::global::BattleGroundInfo* runtime_bginfo_;

    // cur request
	BGRequestInfo   cur_request_bg_;

    // history data
    typedef std::map<int32_t/*bg_tid*/, common::BattleGroundData::HistoryData> HistoryDataMap;
    HistoryDataMap  history_bg_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_BATTLEGROUND_H_
