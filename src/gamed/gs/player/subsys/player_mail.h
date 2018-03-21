#ifndef GAMED_GS_SUBSYS_PLAYER_MAIL_H_
#define GAMED_GS_SUBSYS_PLAYER_MAIL_H_

#include "gs/player/player_subsys.h"
#include "gamed/client_proto/G2C_proto.h"

namespace gamed
{

typedef common::PlayerMailList::Mail Mail;

class PlayerMail : public PlayerSubSystem
{
public:
	PlayerMail(Player& player);
	virtual ~PlayerMail();

	bool SaveToDB(common::PlayerMailList* pData);
	bool LoadFromDB(const common::PlayerMailList& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	void AnnounceNewMail();
	void SendSysMail(int32_t score, int32_t item_id);
	void SendSysMail(shared::net::ByteBuffer& data);

    static void SendSysMail(int32_t master_id, RoleID receiver, shared::net::ByteBuffer& data); // 会修改data的值
    static void SendSysMail(int32_t master_id, RoleID receiver, const playerdef::SysMail& sysmail);

protected:
	//
	// CMD处理函数
	//
	void CMDHandler_GetMailList(const C2G::GetMailList&);
	void CMDHandler_GetMailAttach(const C2G::GetMailAttach&);
	void CMDHandler_DeleteMail(const C2G::DeleteMail&);
	void CMDHandler_TakeoffMailAttach(const C2G::TakeoffMailAttach&);
	void CMDHandler_SendMail(const C2G::SendMail&);
	//
	// MSG处理函数
	//
	int32_t MSGHandler_GetAttachRe(const MSG&);
	//int32_t MSGHandler_DeleteMailRe(const MSG&);
	int32_t MSGHandler_SendMailRe(const MSG&);
	int32_t MSGHandler_NewMail(const MSG&);
private:
	typedef std::map<int64_t, Mail> MailList;
	typedef std::map<int64_t, G2C::MailAttach> AttachList;
	MailList mail_list_;
	AttachList attach_list_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_MAIL_H_
