#ifndef GAMED_GS_NETMSG_PROTO_FORWARD_DECLARE_INL_
#define GAMED_GS_NETMSG_PROTO_FORWARD_DECLARE_INL_

namespace common { 
namespace protocol {

	namespace global
	{
		class ServerStatusNotify;
		class ConnectEstablished;
		class ConnectDestroyed;
		class InstanceInfo;
        class BattleGroundInfo;
		class GlobalDataChange;
        class RePunchCardHelp;
        class RePunchCardHelpReply;
        class RePunchCardHelpError;
	}

	namespace M2G
	{
		class AnnounceMasterInfo;
		class PlayerEnterWorld;
		class JoinTeam;
		class LeaveTeam;
		class ChangeTeamLeader;
		class ChangeTeamPos;
		class TeamInfo;
		class QueryTeamMember;
		class JoinTeamReq;
        class TeamLeaderConvene;
		class PlayerChangeMap;
		class PlayerChangeMapError;
		class EnterInsReply;
		class GlobalCounterInfo;
		class ModifyGlobalCounter;
        class SetGlobalCounter;
		class QueryItemInfo;
		class GetMailAttach_Re;
		class DeleteMail_Re;
		class SendMail_Re;
		class AnnounceNewMail;
		class TeammemberLocalInsReply;
		class TeamLeaderStartLocalIns;
		class TeammemberQuitLocalIns;
		class TeamLocalInsInvite;
        class TeamCrossInsInvite;
        class TeamCrossInsInviteReply;
        class AuctionInvalidQuery;
        class QueryRoleInfo;
        class EnterBGReply;
	}

	namespace L2G
	{
		class C2SGamedataSend;
		class AnnounceLinkInfo;
		class PlayerLogout;
		class RedirectPlayerGSError;
	}

} // namespace protocol
} // namespace common

#endif // GAMED_GS_NETMSG_PROTO_FORWARD_DECLARE_INL_
