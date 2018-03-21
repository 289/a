#ifndef GAMED_GS_SUBSYS_PLAYER_PARTICIPATION_H_
#define GAMED_GS_SUBSYS_PLAYER_PARTICIPATION_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace gamed
{

class PlayerParticipation : public PlayerSubSystem
{
public:
    PlayerParticipation(Player& player);
    virtual ~PlayerParticipation();

	bool SaveToDB(common::PlayerParticipationData* pData);
	bool LoadFromDB(const common::PlayerParticipationData& data);

	virtual void RegisterCmdHandler();

    void PlayerGetParticipationData() const;
    void ModifyParticipation(int32_t value);
    void SetParticipation(int32_t value);
    void ClearParticipationAward();
private:
    void ResetParticipation(int32_t now);
protected:
	void CMDHandler_GetParticipationAward(const C2G::GetParticipationAward&);
private:
    int32_t reset_time_;
    common::PlayerParticipationData data_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_PARTICIPATION_H_
