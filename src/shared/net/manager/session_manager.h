#ifndef SHARED_NET_SESSION_MANAGER_H
#define SHARED_NET_SESSION_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <google/protobuf/message.h>
#include "shared/base/rwlock.h"
#include "shared/base/atomic.h"

namespace shared
{
namespace net
{

class Session;
class ProtoPacket;
class RpcMessage;

class SessionManager
{
	friend class Session;
	friend class NetWork;
	friend class RpcService;
public:
	typedef google::protobuf::Message& MessageRef;

	enum ServerType
	{
		SERVER = 0,
		CLIENT,
	};

	SessionManager();
	virtual ~SessionManager();

	void Release();
	int32_t GetManagerId() const;
	virtual std::string Identification() const = 0;

	void CloseAllSession();
	// 当managertype_=CLIENT时，才应该使用sid为0的默认参数
	Session* GetSession(int32_t sid = 0);
	void Close(int32_t sid = 0);
	void Send(ProtoPacket& packet, int32_t sid = 0);
	void Send(MessageRef packet, int32_t sid = 0);
	void Send(RpcMessage& packet, int32_t sid = 0);

	void EnableRetry(bool retry);
	void Stop();
	void Restart();
protected:
	static int32_t AllocateSessionId();
	void SetServerType(ServerType type);
	// NetWork调用
	virtual Session* CreateSession(int32_t sockfd) = 0;
	virtual void StartRegisterRpcService();
	void SetHandler(void* phandler);
private:
	static void ClearDelSession(void* pdata);
	void ClearDelSession();

	void Retry();

	// Session调用
	void AddSession(int32_t sid, Session* session);
	void DelSession(int32_t sid, int32_t status = 0);
protected:
	ServerType managertype_;
	void* phandler_;
	bool retry_;

	shared::RWLock sessionlock_;
	static AtomicInt32 s_next_sessionid_;
	typedef std::map<int32_t, Session*> SessionMap;
	SessionMap sessions_;

	static AtomicInt32 s_next_managerid_;
	int32_t managerid_;
	std::vector<Session*> to_be_delete_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_SESSION_Manager_H
