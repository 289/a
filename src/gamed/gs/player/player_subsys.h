#ifndef GAMED_GS_PLAYER_PLAYER_SUBSYS_H_
#define GAMED_GS_PLAYER_PLAYER_SUBSYS_H_

#include "shared/net/packet/dispatcher_packet.h"

#include "player.h"
#include "player_ctrl.h"


namespace gamed {


#define REGISTER_NORMAL_CMD_HANDLER(packet, handler) \
	player_.commander()->cmd_disp().Register<packet>(BIND_MEM_CB(&handler, this));

#define REGISTER_DEBUG_CMD_HANDLER(packet, handler) \
	player_.commander()->debug_cmd_disp().Register<packet>(BIND_MEM_CB(&handler, this));

#define REGISTER_MSG_HANDLER(msg, handler) \
	subsys_if_.RegisterMsgHandler(msg, BIND_MEM_CB(&handler, this));

#define SAVE_LOAD_REGISTER(attr_name, save_cb, load_cb) \
	subsys_if_.Register<attr_name>(BIND_MEM_CB(&save_cb, this), BIND_MEM_CB(&load_cb, this));


/**
 * @brief PlayerSubSystem
 *    1.子系统一旦创建，在player整个运行过程中不会delete，因此packet的dispatcher不会有问题 
 *    2.添加新子系统的时候，需要在SubSysIf::CreateSubSys函数中添加一行创建本子系统的代码
 *    3.各子系统相互独立，通过MSG通信，或者在player.h里写函数，各个子系统通过player_调用函数来通信，
 *      但是这种方法注意要防止循环调用。
 */

#define SUBSYS_MASK_COUNT 256
typedef shared::net::BitSet<SUBSYS_MASK_COUNT> SubSysMask;
typedef uint32_t SubSysType;
	
class SubSysIf;
class PlayerSubSystem
{
public:
	enum
	{
		SUB_SYS_TYPE_INVALID = -1,
      // 0
		SUB_SYS_TYPE_INVENTORY,
		SUB_SYS_TYPE_COMBAT,
		SUB_SYS_TYPE_SKILL,
		SUB_SYS_TYPE_SERVICE,
		SUB_SYS_TYPE_TEAM,
      // 5
		SUB_SYS_TYPE_PET,
		SUB_SYS_TYPE_TRANSFER,
		SUB_SYS_TYPE_TASK,
		SUB_SYS_TYPE_INSTANCE,
		SUB_SYS_TYPE_BUDDY,
      // 10
		SUB_SYS_TYPE_BUFF,
		SUB_SYS_TYPE_GATHER,
		SUB_SYS_TYPE_DRAMA,
		SUB_SYS_TYPE_CHAT,
		SUB_SYS_TYPE_MALL,
      // 15
		SUB_SYS_TYPE_MAIL,
		SUB_SYS_TYPE_MAP_TEAM,
		SUB_SYS_TYPE_TALENT,
        SUB_SYS_TYPE_AUCTION,
        SUB_SYS_TYPE_FRIEND,
      // 20
        SUB_SYS_TYPE_COOLDOWN,
        SUB_SYS_TYPE_DUEL,
        SUB_SYS_TYPE_REPUTATION,
        SUB_SYS_TYPE_PROP_REINFORCE,
        SUB_SYS_TYPE_ACHIEVE,
      // 25
        SUB_SYS_TYPE_ENHANCE,
        SUB_SYS_TYPE_TITLE,
        SUB_SYS_TYPE_STAR,
        SUB_SYS_TYPE_BATTLEGROUND,
        SUB_SYS_TYPE_LANDMINE,
      // 30
        SUB_SYS_TYPE_CARD,
        SUB_SYS_TYPE_ARENA,
        SUB_SYS_TYPE_BW_LIST,
        SUB_SYS_TYPE_PLAYER_COUNTER,
        SUB_SYS_TYPE_PUNCH_CARD,
      // 35
        SUB_SYS_TYPE_GEVENT,
        SUB_SYS_TYPE_MOUNT,
        SUB_SYS_TYPE_PARTICIPATION,
        SUB_SYS_TYPE_BOSS_CHALLENGE,
        SUB_SYS_TYPE_WORLD_BOSS,
      // 40

		SUB_SYS_TYPE_MAX
	};
	
	typedef shared::net::ProtoPacket& PacketRef;
	typedef shared::bind::Callback<int (const MSG& msg)> MsgDispatcherCB;

	PlayerSubSystem(SubSysType t, Player& player);
	virtual ~PlayerSubSystem();
	SubSysType GetType() const { return type_; }
	void Initialize();

	virtual void OnInit() { }
	virtual void OnHeartbeat(time_t cur_time) { }
	virtual void OnEnterWorld() { }
	virtual void OnLeaveWorld() { }
	virtual void OnDeath() { }
	virtual void OnRelease() { }
	virtual void OnTransferCls() { }
    
protected:
	virtual void RegisterMsgHandler() { }
	virtual void RegisterCmdHandler() { }

	void SendCmd(const PacketRef packet) const;
	void BroadCastCmd(const PacketRef packet) const;
	void SendError(int err_no) const;

protected:
	SubSysType  type_;
	Player&     player_;
	SubSysIf&   subsys_if_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_PLAYER_SUBSYS_H_
