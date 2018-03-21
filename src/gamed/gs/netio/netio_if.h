#ifndef GAMED_GS_NETIO_NETIO_IF_H_
#define GAMED_GS_NETIO_NETIO_IF_H_

#include <stdint.h>
#include <vector>
#include <map>

#include "shared/base/rwlock.h"
#include "shared/net/protobuf/codec_proto.h"

#include "queue_msg_type.h"


namespace gamed {

using namespace shared;
using namespace shared::net;
using namespace gamed::network;

class NetIO
{
public:
	// ---- thread safe ----
	static void    SetMasterIdMapToSid(int32_t masterid, int32_t sid);
	static int32_t GetMasterIdBySid(int32_t sid);
	static int32_t GetMasterSidById(int32_t masterid);
	static void    RemoveMasterIdMapToSid(int32_t masterid);

	// ---- thread safe ----
	static void    SetLinkIdMapToSid(int32_t linkid, int32_t sid);
	static int32_t GetLinkIdBySid(int32_t sid);
	static void    RemoveLinkIdMapToSid(int32_t linkid);

	// ---- thread safe ----
	static void    SendToAllMaster(const ProtobufCodec::MessageRef msg_ref);
	static void    SendToMaster(int32_t masterid, const ProtobufCodec::MessageRef msg_ref);
	static void    SendToMasterBySid(int32_t sid, const ProtobufCodec::MessageRef msg_ref);
	static void    SendToLink(int32_t linkid, const ProtobufCodec::MessageRef msg_ref);
	static void    SendToLinkBySid(int32_t sid, const ProtobufCodec::MessageRef msg_ref);

	///
	/// 以下接口来自NetworkEvLoop, 现在对外不在直接使用NetworkEvLoop的instance
	/// 
	// **** thread unsafe ****
	static bool    Init(LinkRecvQueue* link_recvqueue, MasterRecvQueue* master_recvqueue);

	/**
	 * @brief Start 
	 *    1.调用Start前需要调用set_master_addr(),set_link_addr()设置link和master的地址
	 *      还需要调用Init()初始化接收队列.
	 */
	static bool    Start();

	static void    Stop();

	// ---- thread safe ----
	// 取出的数据是new出来的，处理完毕后需要delete
	static void    TakeAllLinkData(std::deque<QueuedRecvMsg>& queue);
	// blocking func
	static void    TakeAllMasterData(std::deque<QueuedRecvMsg>& queue);

	static bool    HasRecvLinkData();

	// **** thread unsafe ****
	// 只在运行时控制gs使用以下两个接口，初始化的时候不要使用
	static int     AddNewLink(const std::string& ip, uint16_t port);
	static int     AddNewMaster(const std::string& ip, uint16_t port);

	// **** thread unsafe ****
	static void    set_master_addr(std::string ip, uint16_t port, bool tcp_nodelay);
	static void    set_link_addr(std::string ip, uint16_t port, bool tcp_nodelay);
	///
	/// end interface from NetworkEvLoop


private:
	typedef std::map<int32_t, int32_t> IDToSidMap;
	static IDToSidMap linkid_mapto_sid_;
	static IDToSidMap masterid_mapto_sid_;

	static RWLock  linkid_map_lock_;
	static RWLock  masterid_map_lock_;
};

} // namespace gamed

#endif // GAMED_GS_NETIO_NETIO_IF_H_
