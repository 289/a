#ifndef GAMED_GS_SUBSYS_PLAYER_GEVENT_H_
#define GAMED_GS_SUBSYS_PLAYER_GEVENT_H_

#include "gs/player/player_subsys.h"

namespace gamed
{

/**
 * @brief：player活动子系统
 */
class PlayerGevent : public PlayerSubSystem
{
public:
	PlayerGevent(Player& player);
	virtual ~PlayerGevent();

	bool SaveToDB(common::PlayerGeventData* pData);
	bool LoadFromDB(const common::PlayerGeventData& data);

	virtual void RegisterCmdHandler();

    void PlayerGetGeventData() const;
    void CompleteGevent(int32_t gevent_gid);
    void SetGeventNum(int32_t gevent_gid, int32_t num);
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_JoinGevent(const C2G::JoinGevent&);
private:
    void ResetGevent(int32_t now);
    void SendJoinReply(int8_t result, int32_t gevent_gid, int32_t gevent_id = 0) const;
    void NotifyDataChange(const common::PlayerGeventData::gevent_entry& entry) const;
private:
    int32_t reset_time_;
    typedef std::map<int32_t/*gevent_gid*/, common::PlayerGeventData::gevent_entry> GeventEntryMap;
    GeventEntryMap gevent_map_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_GEVENT_H_
