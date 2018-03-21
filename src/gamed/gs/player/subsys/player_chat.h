#ifndef GAMED_GS_SUBSYS_PLAYER_CHAT_H_
#define GAMED_GS_SUBSYS_PLAYER_CHAT_H_

#include "gs/player/player_subsys.h"

namespace gamed 
{

class ChatCtrl
{
public:
	ChatCtrl();
	virtual ~ChatCtrl();

	virtual bool CanSend(const Player& player, int64_t receiver) const;
	void Consume(Player& player);
	virtual void Send(Player& player, const C2G::ChatMsg& cmd);
protected:
	int32_t level_;
	int32_t money_;
	int32_t cash_;
	int32_t item_id_;
	int32_t item_count_;
};

class WorldChatCtrl : public ChatCtrl
{
public:
	WorldChatCtrl();
};

class TeamChatCtrl : public ChatCtrl
{
public:
	TeamChatCtrl();

	virtual bool CanSend(const Player& player, int64_t receiver) const;
};

class RegionChatCtrl : public ChatCtrl
{
public:
	RegionChatCtrl();

	virtual void Send(Player& player, const C2G::ChatMsg& cmd);
};

class PrivateChatCtrl : public ChatCtrl
{
public:
	PrivateChatCtrl();

	virtual bool CanSend(const Player& player, int64_t receiver) const;
};

/**
 * @brief：player聊天子系统
 */
class PlayerChat : public PlayerSubSystem
{
public:
	PlayerChat(Player& player);
	virtual ~PlayerChat();

	virtual void OnInit();
	virtual void OnRelease();
	
	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	void SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content);
    void SendDebugChat(const std::string& content);

protected:
	//
	// CMD处理函数
	//
	void CMDHandler_ChatMsg(const C2G::ChatMsg&);
private:
	typedef std::map<int8_t, ChatCtrl*> ChatCtrlMap;
	ChatCtrlMap ctl_map_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_TASK_H_
