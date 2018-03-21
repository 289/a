#ifndef GAMED_GS_PLAYER_INTER_MSG_FILTER_H_
#define GAMED_GS_PLAYER_INTER_MSG_FILTER_H_

#include "gs/global/message.h"

#include "player_state.h"


namespace gamed {

class Player;
class InterMsgFilter
{
public:
	enum
	{
		MSG_OP_DENY = 0,
		MSG_OP_ACCEPT,
		MSG_OP_SPECIAL_HANDLE,
	};

	static int DispatchMessage(Player* player, const MSG& msg);

	// just for derivative class
	virtual int HandleMessage(Player* a_player, const MSG& a_msg);

private:
	static InterMsgFilter*    s_special_filter_[PlayerState::STATE_COUNT];
};

///
/// PlayerMsgDispatcher
///
class PlayerMsgDispatcher
{
public:
	static PlayerMsgDispatcher*    DENY();
	static PlayerMsgDispatcher*    ACCEPT();
	static PlayerMsgDispatcher*    SPECIAL_HANDLE();

	static PlayerMsgDispatcher* GetMsgDispatcher(const MSG& msg, PlayerState::MaskType state);

	virtual int DispatchMsg(Player* a_player, InterMsgFilter* a_special, const MSG& a_msg);


private:
	static PlayerMsgDispatcher*    s_deny_;
	static PlayerMsgDispatcher*    s_accept_;
	static PlayerMsgDispatcher*    s_special_handle_;
	static int                     s_init_msg_map_; 
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_INTER_MSG_FILTER_H_
