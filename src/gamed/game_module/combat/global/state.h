#ifndef __GAME_MODULE_COMBAT_GLOBAL_STATE_H__
#define __GAME_MODULE_COMBAT_GLOBAL_STATE_H__

#include <set>
#include <assert.h>

namespace combat
{

typedef int OPT;
typedef int TICK;
typedef int EVENT;
typedef int STATUS;

/**
 * @class XState
 * @brief 状态基类
 */
template <class T>
class XState
{
protected:
	STATUS status;
	TICK   timeout;
	std::set<OPT> opts;

public:
	XState():
		status(0),
		timeout(-1)
	{
	}
	explicit XState(STATUS st):
		status(st),
		timeout(-1)
	{
	}
	XState(STATUS st, OPT* first, size_t n):
		status(st),
		timeout(-1),
		opts(first, first+n)
	{
	}
	virtual ~XState()
	{
		status = 0;
		timeout = -1;
		opts.clear();
	}

public:
	virtual XState* Clone() const = 0;
	virtual void OnInit(T* obj)  {}
	virtual void OnEnter(T* obj) {}
	virtual void OnLeave(T* obj) {}
	virtual void OnHeartBeat(T* obj) {}

    bool operator == (STATUS status) const;
	bool operator == (const XState& rhs) const;
	bool operator != (const XState& rhs) const;

	STATUS GetStatus() const;
	TICK GetTimeOut() const;
	void SetTimeOut(TICK tick);
	bool OptionPolicy(OPT opt) const;
	int  HeartBeat(T* obj);//返回-1表示永不超时;返回0表示超时;返回大于0表示未超时
	void Clear();
};

///
/// inline func
///
template<class T>
bool XState<T>::operator == (STATUS st) const
{
    return st == status;
}

template<class T>
bool XState<T>::operator == (const XState& rhs) const
{ 
	return status == rhs.status;
}

template<class T>
bool XState<T>::operator != (const XState& rhs) const
{
	return status != rhs.status;
}

template<class T>
void XState<T>::SetTimeOut(TICK tick)
{
    assert(tick != 0);
	timeout = tick;
}

template<class T>
STATUS XState<T>::GetStatus() const
{
    return status;
}

template<class T>
int XState<T>::GetTimeOut() const
{
	return timeout;
}

template<class T>
bool XState<T>::OptionPolicy(OPT opt) const
{
	return opts.find(opt) != opts.end();
}

template<class T>
int XState<T>::HeartBeat(T* obj)
{
	OnHeartBeat(obj);

	if (timeout == -1)
	{
        //won't timeout forever
		return -1;
	}

	if (-- timeout <= 0)
	{
		//state timeout
		return 0;
	}

    //hasn't timeout until now
	return timeout;
}

template<class T>
void XState<T>::Clear()
{
	status = 0;
	timeout = -1;
	opts.clear();
}

}; // namespace combat

#endif // __GAME_MODULE_COMBAT_GLOBAL_STATE_H__
