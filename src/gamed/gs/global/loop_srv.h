#ifndef GAMED_GS_GLOBAL_LOOP_SRV_H_
#define GAMED_GS_GLOBAL_LOOP_SRV_H_

#include "shared/base/conf.h"
#include "shared/base/singleton.h"


namespace shared {
namespace net {
	class EventLoop;
	class EventLoopThread;
} // net 
} // shared


namespace gamed {

class LoopService : public shared::Singleton<LoopService>
{
	friend class shared::Singleton<LoopService>;
public:
	static inline LoopService* GetInstance() {
		return &(get_mutable_instance());
	}

	bool Init(const shared::Conf& conf, int port_delta);
	void StartLoop();
	void StopLoop();

	inline shared::net::EventLoop* GetLoop();


protected:
	LoopService();
	~LoopService();


private:
	void    LoopThreadRunStatusCB(shared::net::EventLoop* loop, bool is_start_run);
	void    DeleteMemberInOrder();
	void    StartServiceModule();
	void    StopServiceModule();


private:
	shared::net::EventLoopThread* evloop_thread_;
	shared::net::EventLoop*       loop_;
	bool         is_started_;
    int          port_delta_;
	shared::Conf conf_;
};

///
/// inline func
///
inline shared::net::EventLoop* LoopService::GetLoop()
{
	return loop_;
}

#define s_pLoopService gamed::LoopService::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_LOOP_SRV_H_
