#ifndef GAMED_GS_SUBSYS_PLAYER_INSTANCE_H_
#define GAMED_GS_SUBSYS_PLAYER_INSTANCE_H_

#include "common/protocol/gen/global/instance_msg.pb.h"
#include "gs/template/data_templ/instance_templ.h"
#include "gs/scene/world_def.h"
#include "gs/player/player_subsys.h"
#include "gs/global/msg_pack_def.h"


namespace dataTempl {
	class InstanceTempl;
	class InstanceGroup;
} // namespace dataTempl


namespace common {
namespace protocol {

	namespace global {
		class InstanceInfo;
	} // namespace global

	namespace G2M {
		class EnterInsRequest;
	} // namespace G2M

} // namespace protocol
} // namespace common


namespace G2C {
	class InstanceEnd;
} // namespace G2C


namespace gamed {

/**
 * @brief：player副本子系统
 */
class PlayerInstance : public PlayerSubSystem
{
	static const int kInsRequestTimeout;     // 单位秒
    static const int kTeamCIRMaxRecordTime;  // 单位秒, TeamCrossInsRequest
    static const int kTeamCIRMinRecordTime;  // 单位秒
    static const int kQueryInsInfoCooldown;  // 单位秒
public:
	PlayerInstance(Player& player);
	virtual ~PlayerInstance();

	bool SaveToDB(common::PlayerInstanceData* pdata);
	bool LoadFromDB(const common::PlayerInstanceData& data);

	virtual void OnHeartbeat(time_t cur_time);
	virtual void OnRelease();
	virtual void OnEnterWorld();
	virtual void OnLeaveWorld();
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	bool CheckInstanceCond(const playerdef::InsInfoCond& info) const;
	bool ConsumeInstanceCond(int32_t ins_tid, bool is_create_ins);
	void SetInstanceInfo(const world::instance_info& info);
	void ResetInstanceInfo();
	bool GetRuntimeInsInfo(MapID target_world_id, common::protocol::global::InstanceInfo& insinfo) const;
	bool GetInstancePos(A2DVECTOR& pos) const;
	bool QueryInsEntrancePos(A2DVECTOR& pos) const;
	void TaskTransferSoloIns(int32_t ins_tid) const;   // 任务传送单人副本
	void GeventTransferSoloIns(int32_t ins_tid) const; // 活动传送单人副本
	void CalcInstanceRecord(int32_t ins_tid, int32_t clear_time, bool is_svr_record, int32_t last_record);
	void FinishInstance(int32_t ins_id, bool succ) const;


public:
	struct InsDataInfo
	{
		InsDataInfo() 
			: is_consumed(false),
			  ins_type(-1)
		{ }

		void clear()
		{
			is_consumed = false;
			ins_type    = -1;
			self_pos    = A2DVECTOR();
			info.clear();
		}

		InsDataInfo& operator=(const InsDataInfo& rhs)
		{
			is_consumed = rhs.is_consumed;
			ins_type    = rhs.ins_type;
			self_pos    = rhs.self_pos;
			info        = rhs.info;
			return *this;
		}

		bool      is_consumed;
		int32_t   ins_type;
		A2DVECTOR self_pos;
		world::instance_info info;
	};

    // cross instance request
    struct CIRTeammateData
    {
        RoleID  roleid;
        int32_t src_world_id;
        float   src_pos_x;
        float   src_pos_y;
        common::protocol::global::PCrossInsInfo detail;
    };
	

protected:
	void CMDHandler_GetInstanceData(const C2G::GetInstanceData&);
	void CMDHandler_TeamLocalInsInvite(const C2G::TeamLocalInsInvite&);
	void CMDHandler_TeamCrossInsRequest(const C2G::TeamCrossInsRequest&);
	void CMDHandler_LaunchTeamLocalIns(const C2G::LaunchTeamLocalIns&);
	void CMDHandler_QuitTeamLocalInsRoom(const C2G::QuitTeamLocalInsRoom&);
	void CMDHandler_QuitTeamCrossInsRoom(const C2G::QuitTeamCrossInsRoom&);
	void CMDHandler_LaunchSoloLocalIns(const C2G::LaunchSoloLocalIns&);
	void CMDHandler_TeamLocalInsInviteRe(const C2G::TeamLocalInsInvite_Re&);
	void CMDHandler_TeamCrossInsReady(const C2G::TeamCrossInsReady&);
	void CMDHandler_PlayerQuitInstance(const C2G::PlayerQuitInstance&);
    void CMDHandler_TeamCrossInsInviteRe(const C2G::TeamCrossInsInvite_Re&);
    void CMDHandler_QueryInstanceRecord(const C2G::QueryInstanceRecord&);

	int  MSGHandler_InsTransferPrepare(const MSG&);
	int  MSGHandler_EnterInsReply(const MSG&);
	int  MSGHandler_TeammemberLocalInsReply(const MSG&);
	int  MSGHandler_QuitTeamLocalIns(const MSG&);
	int  MSGHandler_TeamLocalInsInvite(const MSG&);
    int  MSGHandler_TeamCrossInsInvite(const MSG&);
    int  MSGHandler_TeamCrossInsInviteRe(const MSG&);


private:
	void ResetCurInsInfo();
	bool CheckEnterCondition(const dataTempl::InstanceTempl* pdata, bool check_team = true) const;
	bool CheckCreateCondition(const dataTempl::InstanceTempl* pdata) const;
	void ConsumeCreateCond(const dataTempl::InstanceTempl* pdata);
	void ConsumeEnterCond(const dataTempl::InstanceTempl* pdata);
	bool CheckInsTypeCount() const;
	bool CheckCondIsConsumed(int32_t ins_templ_id) const;
	void FillReenterInfo(int32_t ins_templ_id, common::protocol::G2M::EnterInsRequest& proto);
	bool CanEnterInstance() const;
    void HandleTeamCrossInsRequest();


private:
	void CleanUpHistoryList(const InsDataInfo& new_curins);
	void CleanUpSpecInsInfo(int32_t ins_tid);
	void CopyToHistoryList(const InsDataInfo& oldins);
	bool ReplaceTeamInsInfo(const InsDataInfo& insinfo);
	bool ReplaceSoloInsInfo(const InsDataInfo& insinfo);
	bool CheckLocalInsReply(const msgpack_teammeber_localins_reply& param);
    bool CheckCrossInsReply(RoleID member_roleid, int32_t ins_group_id, int32_t ins_templ_id);
	void BroadcastAgreeTeammateLC(shared::net::ProtoPacket& packet, RoleID except_id = 0) const;
	void BroadcastAgreeTeammateCR(shared::net::ProtoPacket& packet, RoleID except_id = 0) const;
	void BroadcastToAllTeammate(shared::net::ProtoPacket& packet) const; // 发给所有组队里的队员，不包含队长自己
	void DeliverInsAward(const dataTempl::InstanceTempl::InsCompleteAward& award, G2C::InstanceEnd& packet);
    void SysEnterSoloIns(int32_t ins_tid, int32_t type) const;


private:
	struct TeamLocalInsInfo
	{
		TeamLocalInsInfo() 
			: ins_group_id(0), 
			  group_templ(NULL)
		{ }

		void clear()
		{
			ins_group_id = 0;
			group_templ  = NULL;
			members.clear();
		}

		struct Detail
		{
			RoleID roleid;
			std::vector<int32_t> ins_tid_vec;
		};

		int32_t ins_group_id;
		Detail  leader;
		std::vector<Detail> members;
		const dataTempl::InstanceGroup* group_templ;
	};

	struct TeamCrossInsInfo
	{
		TeamCrossInsInfo()  { clear(); }
        ~TeamCrossInsInfo() { clear(); }

		void clear()
		{
			ins_group_id = 0;
			ins_templ_id = 0;
            time_out     = 0;
            reply_count  = 0;
            members.clear();
		}

        bool has_info() const
        {
            if (ins_group_id > 0 && ins_templ_id > 0)
                return true;
            return false;
        }

        bool is_countdown() const
        {
            return time_out > 0;
        }

		int32_t ins_group_id;
		int32_t ins_templ_id;
        int32_t time_out;
        int32_t reply_count;
        std::vector<CIRTeammateData> members; // 只存队友信息，不存自己
	};

	struct InsRequestInfo
	{
		InsRequestInfo() { clear(); }

		void clear()
		{
			ins_tid  = 0;
			time_out = 0;
		}

		bool has_request() const
		{
			return ins_tid != 0;
		}

		bool is_countdown() const
		{
			return time_out > 0;
		}

		int32_t ins_tid;
		int32_t time_out;
	};


private:
	InsDataInfo cur_ins_;

	// ins_templ_id, InsDataInfo
	typedef std::map<int32_t, InsDataInfo> InsDataInfoMap;
	InsDataInfoMap   history_ins_;

	common::protocol::global::InstanceInfo* runtime_insinfo_;

	// local ins
	TeamLocalInsInfo team_localins_info_;

	// cross ins
	TeamCrossInsInfo team_crossins_info_;
	
	// cur request
	InsRequestInfo   cur_request_ins_;

    // query ins info cooldown
    typedef std::map<int32_t/*ins_tid*/, int32_t/*countdown*/> QueryCoolDownMap;
    QueryCoolDownMap query_cooldown_map_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_INSTANCE_H_
