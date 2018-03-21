#ifndef GAMED_GS_SUBSYS_PLAYER_DUEL_H_
#define GAMED_GS_SUBSYS_PLAYER_DUEL_H_

#include "gs/player/player_subsys.h"
#include "gs/global/msg_pack_def.h"


namespace gamed {

/**
 * @brief: 玩家决斗（切磋）子系统
 *  （1）可以个人发起，对方可以是一个人或一个队
 *  （2）已经组队的只能由队长发起
 *  （3）接收方是组队的，只能由队长接收，队长太远则发起失败
 */
class PlayerDuel : public PlayerSubSystem
{
    static const int kDuelPrepareTime = 6; // 单位s，决斗开始的准备时间
public:
	PlayerDuel(Player& player);
	virtual ~PlayerDuel();

	virtual void OnRelease();
	virtual void OnHeartbeat(time_t cur_time);
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    void DuelCombatEnd(const playerdef::PVPEndInfo& info, playerdef::PVPEndExtraData& data);


protected:
    void CMDHandler_DuelRequest(const C2G::DuelRequest&);
    void CMDHandler_DuelRequestReply(const C2G::DuelRequest_Re&);
    void CMDHandler_TeammateDuelRequestReply(const C2G::TeammateDuelRequest_Re&);

    int  MSGHandler_DuelRequest(const MSG&);
    int  MSGHandler_DuelRequestReply(const MSG&);
    int  MSGHandler_TeammateDuelRequest(const MSG&);
    int  MSGHandler_TeammateDuelRequestReply(const MSG&);
    int  MSGHandler_DuelPrepare(const MSG&);
    int  MSGHandler_StartJoinDuelCombat(const MSG&);


private:
    struct DuelRequestInfo
    {
        DuelRequestInfo() { clear(); }

        void clear()
        {
            duel_roleid = 0;
            time_out    = 0;
        }

        bool is_countdown() const
		{
			return time_out > 0;
		}

        RoleID  duel_roleid; // 决斗对象
        int32_t time_out;
    };

    struct DuelRequester
    {
        DuelRequester() { clear(); }

        void clear()
        {
            id       = 0;
            time_out = 0;
        }

        bool is_countdown() const
        {
            return time_out > 0;
        }

        RoleID  id;
        int32_t time_out;
    };

    typedef std::vector<duel_team_info> DuelTeamInfoVec;


private:
    void HandleTeamDuelRequest(RoleID duel_roleid);
    void HandleSoloDuelRequest(RoleID duel_roleid);
    bool CheckDuelCondition(RoleID duel_roleid);
    void HandleDuelStartCombat();
    void RequestTransmitToLeader(RoleID duel_roleid);
    void QueryDuelMembers(DuelTeamInfoVec& team_info);
    void SendDuelPrepareMsg(const XID& target);


private:
    typedef std::set<RoleID> DuelRequesterSet;
    DuelRequesterSet waiting_requester_;
    DuelRequester    duel_requester_; // 接收方记录发起者id
    DuelRequestInfo  request_info_;   // 发起方记录的信息
}; 

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_DUEL_H_
