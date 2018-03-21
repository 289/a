#ifndef GAMED_GS_NETMSG_LINK_MSG_PROC_H_
#define GAMED_GS_NETMSG_LINK_MSG_PROC_H_

#include "shared/base/singleton.h"
#include "shared/base/thread.h"
#include "shared/net/packet/codec_packet.h"

#include "gs/netio/queue_msg_type.h"

#include "dispatcher_queued_msg.h"

// 协议的前向声明放在下面这个文件，cpp里才include对应的协议.h文件
#include "proto_forward_declare.inl"


namespace gamed {

using namespace shared;
using namespace shared::net;
using namespace common::protocol;

class LinkMsgProcess : public shared::Singleton<LinkMsgProcess>
{
	friend class shared::Singleton<LinkMsgProcess>;
public:
	static inline LinkMsgProcess* GetInstance() {
		return &(get_mutable_instance());
	}

	network::LinkRecvQueue* get_recv_queue();

	bool    StartProcThread();
	void    StopProcThread();


protected:
	LinkMsgProcess();
	~LinkMsgProcess();

	static void* ThreadFunc(void*);
	void    RunInThread();
	void    HandleLinkMessage(network::QueuedRecvMsg& msg);

  // proto
	void    StartupRegisterHandler();
	void    ProtoDefaultHandler(const MessageRef, const WrapRef);
	void    ServerStatusNotify(const global::ServerStatusNotify&, const WrapRef);
	void    LinkOnConnected(const global::ConnectEstablished&, const WrapRef);
	void    LinkConnDestroyed(const global::ConnectDestroyed&, const WrapRef);
	void    C2SGamedataSend(const L2G::C2SGamedataSend&, const WrapRef);
	void    AnnounceLinkInfo(const L2G::AnnounceLinkInfo&, const WrapRef);
	void    LinkPlayerLogout(const L2G::PlayerLogout&, const WrapRef);
	void    RedirectPlayerGSError(const L2G::RedirectPlayerGSError&, const WrapRef);


private:
	static network::LinkRecvQueue recv_queue_;

	Thread                proc_thread_;
	QueuedMsgDispatcher   dispatcher_;

	bool    is_done_;
};

#define s_pLinkMsgProc gamed::LinkMsgProcess::GetInstance()

} // namespace gamed

#endif // GAMED_GS_NETMSG_LINK_MSG_PROC_H_
