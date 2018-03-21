#include "player_chat.h"

#include "shared/net/packet/codec_templ.h"
#include "gs/global/gmatrix.h"
#include "gs/global/dbgprt.h"
#include "gs/player/player_sender.h"
#include "gs/player/subsys_if.h"


namespace gamed {

namespace {

    // 需要与客户端一致（主要用于调试）
    enum CHAT_MESSAGE_ELEMENT_TYPE
    {
        CHAT_MESSAGE_ELEMENT_INVALID,
        CHAT_MESSAGE_ELEMENT_STRING,
        CHAT_MESSAGE_ELEMENT_EXPRESSION,
        CHAT_MESSAGE_ELEMENT_ITEM,
        CHAT_MESSAGE_ELEMENT_MAX
    };

    // 发给客户端的纯字符聊天信息（主要用于调试）
    class ChatMsgElementStr : public shared::net::BasePacket
    {
    public:
        std::string content;

        ChatMsgElementStr()
            : BasePacket(CHAT_MESSAGE_ELEMENT_STRING)
        { }
        
        virtual void Marshal()
        {
            buf_ << content;
        }

        virtual void Unmarshal()
        {
            buf_ >> content;
        }

        virtual BasePacket* Clone() 
        { 
            return NULL; 
        }
    };

    static shared::net::TemplPacketCodec<shared::net::BasePacket> s_codec(1, NULL, NULL);

} // Anonymous


// 聊天控制器
ChatCtrl::ChatCtrl()
	: level_(0), money_(0), cash_(0), item_id_(0), item_count_(0)
{
}

ChatCtrl::~ChatCtrl()
{
}

bool ChatCtrl::CanSend(const Player& player, int64_t receiver) const
{
	if (level_ > 0 && player.GetLevel() < level_)
	{
		return false;
	}
	if (money_ > 0 && player.GetMoney() < money_)
	{
		return false;
	}
	if (cash_ > 0 && player.GetCash() < cash_)
	{
		return false;
	}
	if (item_id_ != 0 && item_count_ > 0 && !player.CheckItem(item_id_, item_count_))
	{
		return false;
	}
	return true;
}

void ChatCtrl::Send(Player& player, const C2G::ChatMsg& cmd)
{
	int64_t sender = player.role_id();
	int64_t receiver = cmd.receiver;
	__PRINTF("ChatCtrl::Send channel=%d sender=%ld receiver=%ld", cmd.channel, sender, receiver);
	player.sender()->SendChatMsg(cmd.channel, sender, receiver, player.rolename(), cmd.msg);
}

void ChatCtrl::Consume(Player& player)
{
	if (money_ > 0)
	{
		player.SpendMoney(money_);
	}
	if (cash_ > 0)
	{
		player.UseCash(cash_);
	}
	if (item_id_ != 0 && item_count_ > 0)
	{
		player.TakeOutItem(item_id_, item_count_);
	}
}

WorldChatCtrl::WorldChatCtrl()
{
}

TeamChatCtrl::TeamChatCtrl()
{
}

RegionChatCtrl::RegionChatCtrl()
{
}

PrivateChatCtrl::PrivateChatCtrl()
{
}

bool TeamChatCtrl::CanSend(const Player& player, int64_t receiver) const
{
	if (!ChatCtrl::CanSend(player, receiver))
	{
		return false;
	}

	return player.IsInTeam();
}

void RegionChatCtrl::Send(Player& player, const C2G::ChatMsg& cmd)
{
	int64_t sender = player.role_id();
	__PRINTF("RegionChatCtrl::Send channel=%d sender=%ld", cmd.channel, sender);

	G2C::ChatMsg msg;
	msg.channel = cmd.channel;
	msg.sender = sender;
	msg.sender_name = player.rolename();
	msg.msg = cmd.msg;
	player.sender()->BroadCastCmd(msg);
}

bool PrivateChatCtrl::CanSend(const Player& player, int64_t receiver) const
{
    if (!ChatCtrl::CanSend(player, receiver))
	{
		return false;
	}

    // 跨服检查，私聊玩家必须是同服的
    if (Gmatrix::IsCrossRealmServer())
    {
        if (!Gmatrix::IsInSameServer(player.role_id(), receiver))
        {
            player.ErrorMessage(G2C::ERR_NOT_IN_SAME_SERVER);
            return false;
        }
    }

    return true;
}

// player聊天子系统
PlayerChat::PlayerChat(Player& player)
	: PlayerSubSystem(SUB_SYS_TYPE_CHAT, player)
{
}

PlayerChat::~PlayerChat()
{
}

void PlayerChat::OnInit()
{
	ctl_map_[C2G::ChatMsg::CHANNEL_WORLD] = new WorldChatCtrl;
	ctl_map_[C2G::ChatMsg::CHANNEL_TEAM] = new TeamChatCtrl;
	ctl_map_[C2G::ChatMsg::CHANNEL_REGION] = new RegionChatCtrl;
	ctl_map_[C2G::ChatMsg::CHANNEL_PRIVATE] = new PrivateChatCtrl;
}

void PlayerChat::OnRelease()
{
	ChatCtrlMap::iterator it = ctl_map_.begin();
	for (; it != ctl_map_.end(); ++it)
	{
		ChatCtrl* ctl = it->second;
		DELETE_SET_NULL(ctl);
	}
	ctl_map_.clear();
}

void PlayerChat::RegisterCmdHandler()
{
	REGISTER_NORMAL_CMD_HANDLER(C2G::ChatMsg, PlayerChat::CMDHandler_ChatMsg);
}

void PlayerChat::RegisterMsgHandler()
{
}

void PlayerChat::CMDHandler_ChatMsg(const C2G::ChatMsg& cmd)
{
	__PRINTF("PlayerChat::CMDHandler_ChatMsg channel=%d", cmd.channel);
	ChatCtrlMap::iterator it = ctl_map_.find(cmd.channel);
	if (it == ctl_map_.end() || it->second == NULL)
	{
		return;
	}

	ChatCtrl* ctl = it->second;
	if (!ctl->CanSend(player_, cmd.receiver))
	{
		__PRINTF("Can not Send");
		return;
	}
	ctl->Send(player_, cmd);
	ctl->Consume(player_);
}

void PlayerChat::SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content)
{
	__PRINTF("PlayerChat::SendSysChat channel=%d", channel);
	ChatCtrlMap::iterator it = ctl_map_.find(channel);
	if (it == ctl_map_.end() || it->second == NULL)
	{
		return;
	}
	if (channel == C2G::ChatMsg::CHANNEL_REGION)
	{
		G2C::ChatMsg msg;
		msg.channel = channel;
		msg.sender = 0;
		msg.sender_name = sender_name;
		msg.msg = content;
		player_.sender()->BroadCastCmd(msg);
	}
	else
	{
		player_.sender()->SendChatMsg(channel, 0, player_.role_id(), sender_name, content);
	}
}

void PlayerChat::SendDebugChat(const std::string& content)
{
    std::vector<BasePacket*> tmp_vec;
    ChatMsgElementStr elem_str; 
    elem_str.content = content;
    tmp_vec.push_back(static_cast<BasePacket*>(&elem_str));

    shared::net::Buffer buffer;
    if (shared::net::TPC_SUCCESS != s_codec.AssemblingEmptyBuffer(&buffer, tmp_vec))
    {
        LOG_ERROR << "AssemblingEmptyBuffer error!";
        return;
    }

    G2C::ChatMsg msg;
    msg.channel     = C2G::ChatMsg::CHANNEL_REGION;
    msg.sender      = player_.role_id();
    msg.sender_name = player_.rolename();
    msg.msg.assign(buffer.peek(), buffer.ReadableBytes());
    player_.sender()->SendCmd(msg);
}

} // namespace gamed
