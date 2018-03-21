#ifndef GAMED_GS_PLAYER_SUBSYS_PLAYER_SERVICE_H_
#define GAMED_GS_PLAYER_SUBSYS_PLAYER_SERVICE_H_

#include "gs/obj/service/service_man.h"
#include "gs/player/player_subsys.h"


namespace gamed {

///
/// 服务子系统，处理client发过来的服务请求
///
class PlayerService : public PlayerSubSystem
{
public:
	PlayerService(Player& player);
	~PlayerService();

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

	void SayHelloToNpc(const XID& target);


protected:
	// cmd
	void   CMDHandler_ServiceHello(const C2G::ServiceHello&);
	void   CMDHandler_ServiceServe(const C2G::ServiceServe&);
	void   CMDHandler_ServiceGetContent(const C2G::ServiceGetContent&);

	// msg
	int    MSGHandler_ServiceGreeting(const MSG&);
	int    MSGHandler_ServiceRequest(const MSG&);
	int    MSGHandler_ServiceData(const MSG&);
	int    MSGHandler_ServiceError(const MSG&);

private:
	int32_t GetNpcCatVision(int32_t tid);
	void    ResetServiceProvider();
	bool    CheckNpcVisibleRule(int32_t tid);


private:
	playerdef::ServiceProviderInfo service_provider_; // 当前给自己提供服务的商人的坐标和id
	ProviderList    service_list_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_SUBSYS_PLAYER_SERVICE_H_
