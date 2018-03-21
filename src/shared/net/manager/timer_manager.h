#ifndef SHARED_NET_TIMER_MANAGER_H
#define SHARED_NET_TIMER_MANAGER_H

namespace shared
{
namespace net
{

class EventLoop;
class TimedTaskManager;

class TimerManager
{
public:
	static void StartTimer(EventLoop* loop);
	static TimedTaskManager* GetTimer();
	static void EndTimer();
private:
	static TimedTaskManager* timer_;
};

} // namespace net
} // namespace shared

#endif // SHARED_NET_TIMER_MANAGER_H
