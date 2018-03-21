#ifndef GAMED_GS_PLAYER_CMDPREPARE_H_
#define GAMED_GS_PLAYER_CMDPREPARE_H_

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"

#include "player_state.h"
#include "player_ctrl.h"
#include "player.h"


namespace gamed {

class CmdPacketDispatcher
{
	static const int kNotFound       = -1;
	static const int kIsNotGScmdType = -2;
public:
	CmdPacketDispatcher();
	~CmdPacketDispatcher();

	inline int DispatchCommand(Player* pPlayer, int cmd_type_no, shared::net::ProtoPacket* packet);


protected:
	class ExecCmd
	{
	public:
		virtual ~ExecCmd() { }
		virtual int Dispatch(PlayerController* pCtrl, int cmd_type_no, shared::net::ProtoPacket* packet) const = 0;
	};

	class ExecCmd1 : public ExecCmd
	{
	public:
		virtual ~ExecCmd1() { }
		virtual int Dispatch(PlayerController* pCtrl, int cmd_type_no, shared::net::ProtoPacket* packet) const
		{
			return pCtrl->UnLockInventoryHandler(packet);
		}
	};

	class ExecCmd2 : public ExecCmd
	{
	public:
		virtual ~ExecCmd2() { }
		virtual int Dispatch(PlayerController* pCtrl, int cmd_type_no, shared::net::ProtoPacket* packet) const
		{
			return pCtrl->InvalidCommandHandler(packet);
		}
	};

	class ExecCmd3 : public ExecCmd
	{
	public:
		virtual ~ExecCmd3() { }
		virtual int Dispatch(PlayerController* pCtrl, int cmd_type_no, shared::net::ProtoPacket* packet) const
		{
			if (!Gmatrix::GetServerParam().permit_debug_cmd)
				return 0;
			return pCtrl->DebugCommandHandler(packet);
		}
	};

	struct cmd_define
	{
		PlayerState::MaskType state_mask;
		PlayerState::MaskType exclude_state_mask;
		PlayerState::MaskType spec_state;
		ExecCmd* spec_handler;
	};

	bool    InitCommandList();
	bool    IsDebugCmd(int cmd_type_no);
	PlayerState::MaskType GetCommandState(const char* str);


private:
	typedef std::map<int32_t, cmd_define> CmdDefineMap;
	CmdDefineMap    standard_cmd_;
};

///
/// inline function
///
inline int CmdPacketDispatcher::DispatchCommand(Player* pPlayer, int cmd_type_no, shared::net::ProtoPacket* packet)
{
	//TODO
	/*
	if (IsDebugCmd(cmd_type_no))
	{
		return pPlayer->commander()->GMCommandHandler(packet);
	}
	*/

	if (!PacketManager::IsGameServerType(cmd_type_no))
	{
		return kIsNotGScmdType;
	}

	std::map<int32_t, cmd_define>::const_iterator it = standard_cmd_.find(cmd_type_no);
	if (standard_cmd_.end() == it)
	{
		return kNotFound;
	}

	const cmd_define& cmd           = it->second;
	PlayerState::MaskType cur_state = (1 << pPlayer->GetState());
	// ????????
	//if (pPlayer->IsDead()) cur_state |= (1 << PlayerState::STATE_DEAD);
	
	if (!(cur_state & cmd.exclude_state_mask))
	{
		if (cur_state & cmd.state_mask)
		{
			return pPlayer->commander()->CommandHandler(packet);
		}
	}

	if (cur_state & cmd.spec_state)
	{
		return cmd.spec_handler->Dispatch(pPlayer->commander(), cmd_type_no, packet);
	}

	__PRINTF("roleid:%ld 目前不能执行命令 %d", pPlayer->role_id(), cmd_type_no);
	return 0;
}

inline bool CmdPacketDispatcher::IsDebugCmd(int cmd_type_no)
{
	if (cmd_type_no >= C2G_DEBUG_CMD_LOWER_LIMIT &&
		cmd_type_no <= C2G_DEBUG_CMD_UPPER_LIMIT)
	{
		return true;
	}

	return false;
}

} // namespace gamed

#endif // GAMED_GS_PLAYER_CMDPREPARE_H_
