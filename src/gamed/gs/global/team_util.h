#ifndef GAMED_GS_GLOBAL_TIDY_TEAM_POS_H_
#define GAMED_GS_GLOBAL_TIDY_TEAM_POS_H_

#include "shared/net/packet/bytebuffer.h"

#include "game_def.h"


namespace gamed {
namespace team {

struct TeamPosDetail
{
    TeamPosDetail() {
        clear();
    }

    void clear()
    {
        roleid       = 0;
        cls          = 0;
        combat_value = 0;
    }

    int64_t roleid; // 0表示这个位置没人
    int32_t cls;
    int32_t combat_value;
};

typedef shared::net::FixedArray<TeamPosDetail, kTeamMemberCount> TeamPosVec;
bool TidyTeamPos(const TeamPosVec& raw, TeamPosVec& pos_info);

} // namespace team
} // namespace gamed

#endif // GAMED_GS_GLOBAL_TIDY_TEAM_POS_H_
