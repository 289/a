#ifndef GAMED_GS_SUBSYS_PLAYER_REPUTATION_H_
#define GAMED_GS_SUBSYS_PLAYER_REPUTATION_H_

#include "gs/player/player_subsys.h"

namespace gamed
{

/**
 * @brief：player声望子系统
 */
class PlayerReputation : public PlayerSubSystem
{
public:
	PlayerReputation(Player& player);
	virtual ~PlayerReputation();

	bool SaveToDB(common::PlayerReputationData* pData);
	bool LoadFromDB(const common::PlayerReputationData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    void PlayerGetReputationData() const;
    void OpenReputation(int32_t reputation_id);
    int32_t GetReputation(int32_t reputation_id) const;
    void ModifyReputation(int32_t reputation_id, int32_t delta);
protected:
	//
	// CMD处理函数
	//
private:
    typedef std::map<int32_t, int32_t> ReputationMap;
    ReputationMap reputation_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_REPUTATION_H_
