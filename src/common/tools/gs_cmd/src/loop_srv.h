#ifndef COMMON_TOOLS_GSCMD_LOOP_SRV_H_
#define COMMON_TOOLS_GSCMD_LOOP_SRV_H_

#include "shared/base/singleton.h"


namespace shared {
namespace net {
	class EventLoop;
	class EventLoopThread;
} // net 
} // shared


namespace gsCmd {

class LoopService : public shared::Singleton<LoopService>
{
	friend class shared::Singleton<LoopService>;
public:
	static inline LoopService* GetInstance() {
		return &(get_mutable_instance());
	}

	bool Init();
	void StartLoop();
	void StopLoop();

	inline shared::net::EventLoop* GetLoop();


protected:
	LoopService();
	~LoopService();


private:
	void    LoopThreadRunStatusCB(shared::net::EventLoop* loop, bool is_start_run);
	void    DeleteMemberInOrder();


private:
	shared::net::EventLoopThread* evloop_thread_;
	shared::net::EventLoop*       loop_;
	bool        is_started_;
};

///
/// inline func
///
inline shared::net::EventLoop* LoopService::GetLoop()
{
	return loop_;
}

#define s_pLoopService gsCmd::LoopService::GetInstance()

} // namespace gamed

#endif // COMMON_TOOLS_GSCMD_LOOP_SRV_H_
