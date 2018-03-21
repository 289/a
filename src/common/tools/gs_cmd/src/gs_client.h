#ifndef COMMON_TOOLS_GSCMD_GS_CLIENT_H_
#define COMMON_TOOLS_GSCMD_GS_CLIENT_H_

#include "shared/net/manager/session_manager.h"
#include "shared/base/singleton.h"


namespace gsCmd {

using namespace shared;
using namespace shared::net;

class GSClient : public SessionManager, public Singleton<GSClient>
{
	friend class Singleton<GSClient>;
public:
	static inline GSClient* GetInstance()
	{
		return &(get_mutable_instance());
	}

	virtual std::string Identification() const
	{
		return "GSClient";
	}

	inline void set_connected(bool is_connected);
	inline bool is_connected() const;


protected:
	GSClient() : is_connected_(false)
	{
		SetServerType(CLIENT);
	}
	~GSClient()
	{
	}

	virtual Session* CreateSession(int sockfd);


private:
	bool is_connected_;
};

///
/// inline func
///
inline void GSClient::set_connected(bool is_connected)
{
	is_connected_ = is_connected;
}

inline bool GSClient::is_connected() const
{
	return is_connected_;
}

#define s_pGSClient GSClient::GetInstance()

} // namespace gsCmd

#endif // COMMON_TOOLS_GSCMD_GS_CLIENT_H_
