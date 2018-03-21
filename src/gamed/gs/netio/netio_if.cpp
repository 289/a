#include "netio_if.h"

#include "shared/logsys/logging.h"

#include "network_evloop.h"


namespace gamed {

RWLock NetIO::linkid_map_lock_;
RWLock NetIO::masterid_map_lock_;
std::map<int32_t, int32_t> NetIO::linkid_mapto_sid_;
std::map<int32_t, int32_t> NetIO::masterid_mapto_sid_;

void NetIO::SetMasterIdMapToSid(int32_t masterid, int32_t sid)
{
	RWLockWriteGuard wrlock(masterid_map_lock_);
	IDToSidMap::iterator it = masterid_mapto_sid_.find(masterid);
	assert(it == masterid_mapto_sid_.end());
	masterid_mapto_sid_[masterid] = sid;
}

void NetIO::RemoveMasterIdMapToSid(int32_t masterid)
{
	RWLockWriteGuard wrlock(masterid_map_lock_);
	size_t n = masterid_mapto_sid_.erase(masterid);
	assert(n == 1); (void)n;
}

int32_t NetIO::GetMasterIdBySid(int32_t sid)
{
	RWLockReadGuard rdlock(masterid_map_lock_);
	IDToSidMap::iterator it = masterid_mapto_sid_.begin();
	for (; it != masterid_mapto_sid_.end(); ++it)
	{
		if (it->second == sid) {
			return it->first;
		}
	}
	return -1;
}

int32_t NetIO::GetMasterSidById(int32_t masterid)
{
	RWLockReadGuard rdlock(masterid_map_lock_);
	IDToSidMap::iterator it = masterid_mapto_sid_.find(masterid);
	if (it != masterid_mapto_sid_.end())
	{
		return it->second;
	}
	return -1;
}

void NetIO::SetLinkIdMapToSid(int32_t linkid, int32_t sid)
{
	RWLockWriteGuard wrlock(linkid_map_lock_);
	IDToSidMap::iterator it = linkid_mapto_sid_.find(linkid);
	assert(it == linkid_mapto_sid_.end());
	linkid_mapto_sid_[linkid] = sid;
}

void NetIO::RemoveLinkIdMapToSid(int32_t linkid)
{
	RWLockWriteGuard wrlock(linkid_map_lock_);
	size_t n = linkid_mapto_sid_.erase(linkid);
	assert(n == 1); (void)n;
}

int32_t NetIO::GetLinkIdBySid(int32_t sid)
{
	RWLockReadGuard rdlock(linkid_map_lock_);
	IDToSidMap::iterator it = linkid_mapto_sid_.begin();
	for (; it != linkid_mapto_sid_.end(); ++it)
	{
		if (it->second == sid) {
			return it->first;
		}
	}
	return -1;
}

void NetIO::SendToAllMaster(const ProtobufCodec::MessageRef msg_ref)
{
	std::vector<int32_t> master_sid_vec;
	{
		RWLockReadGuard rdlock(masterid_map_lock_);
		IDToSidMap::iterator it = masterid_mapto_sid_.begin();
		for (; it != masterid_mapto_sid_.end(); ++it)
		{
			master_sid_vec.push_back(it->second);
		}
	}

	for (size_t i = 0; i < master_sid_vec.size(); ++i)
	{
		s_pNetworkEvLoop->SendToMaster(master_sid_vec[i], msg_ref);
	}
}

void NetIO::SendToMaster(int32_t masterid, const ProtobufCodec::MessageRef msg_ref)
{
    if (masterid > 0)
    {
        int32_t master_sid = -1;
        {
            RWLockReadGuard rdlock(masterid_map_lock_);
            IDToSidMap::iterator it = masterid_mapto_sid_.find(masterid);
            if (it == masterid_mapto_sid_.end())
            {
                LOG_ERROR << "masterid mapto sid not found! masterid:" << masterid;
                return;
            }
            master_sid = it->second;
        }

        s_pNetworkEvLoop->SendToMaster(master_sid, msg_ref);
    }
}

void NetIO::SendToMasterBySid(int32_t sid, const ProtobufCodec::MessageRef msg_ref)
{
	s_pNetworkEvLoop->SendToMaster(sid, msg_ref);
}
	
void NetIO::SendToLink(int32_t linkid, const ProtobufCodec::MessageRef msg_ref)
{
    if (linkid > 0)
    {
        int32_t link_sid = -1;
        {
            RWLockReadGuard rdlock(linkid_map_lock_);
            IDToSidMap::iterator it = linkid_mapto_sid_.find(linkid);
            if (it == linkid_mapto_sid_.end()) 
            {
                LOG_ERROR << "linkid mapto sid not found! linkid:" << linkid;
                return;
            }
            link_sid = it->second;
        }

        s_pNetworkEvLoop->SendToLink(link_sid, msg_ref);
    }
}
	
void NetIO::SendToLinkBySid(int32_t sid, const ProtobufCodec::MessageRef msg_ref)
{
	s_pNetworkEvLoop->SendToLink(sid, msg_ref);
}

///
/// 以下接口来自NetworkEvLoop, 现在对外不在直接使用NetworkEvLoop的instance
/// 
bool NetIO::Init(LinkRecvQueue* link_recvqueue, MasterRecvQueue* master_recvqueue)
{
	return s_pNetworkEvLoop->Init(link_recvqueue, master_recvqueue);
}

bool NetIO::Start()
{
	return s_pNetworkEvLoop->Start();
}

void NetIO::Stop()
{
	s_pNetworkEvLoop->Stop();
}

void NetIO::TakeAllLinkData(std::deque<QueuedRecvMsg>& queue)
{
	s_pNetworkEvLoop->TakeAllLinkData(queue);
}

void NetIO::TakeAllMasterData(std::deque<QueuedRecvMsg>& queue)
{
	s_pNetworkEvLoop->TakeAllMasterData(queue);
}

bool NetIO::HasRecvLinkData()
{
	return s_pNetworkEvLoop->HasRecvLinkData();
}

void NetIO::set_master_addr(std::string ip, uint16_t port, bool tcp_nodelay)
{
	s_pNetworkEvLoop->set_master_addr(ip, port);
	s_pNetworkEvLoop->set_master_tcp_nodelay(tcp_nodelay);
}

void NetIO::set_link_addr(std::string ip, uint16_t port, bool tcp_nodelay)
{
	s_pNetworkEvLoop->set_link_addr(ip, port);
	s_pNetworkEvLoop->set_link_tcp_nodelay(tcp_nodelay);
}

int NetIO::AddNewLink(const std::string& ip, uint16_t port)
{
	return s_pNetworkEvLoop->AddNewLink(ip, port);
}

int NetIO::AddNewMaster(const std::string& ip, uint16_t port)
{
	return s_pNetworkEvLoop->AddNewMaster(ip, port);
}

} // namespace gamed
