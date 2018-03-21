#include "team_util.h"

#include "gs/player/player_def.h"


namespace gamed {
namespace team {

using namespace playerdef;

namespace {

static int cls_weight_list[][kTeamMemberCount+1] = 
{
    // cls                   pos1   pos2   pos3   pos4
    { CLS_NEWBIE,             6,     5,     2,     7 },
    // warrior
    { CLS_PRIMARY_WARRIOR,    8,     6,     4,     2 },
    { CLS_J_ATTACK_WARRIOR,   6,     5,     2,     7 },
    { CLS_S_ATTACK_WARRIOR,   6,     5,     2,     7 },
    { CLS_J_DEFENSE_WARRIOR,  9,     7,     5,     3 },
    { CLS_S_DEFENSE_WARRIOR,  10,    8,     6,     4 },
    // mage
    { CLS_PRIMARY_MAGE,       2,     4,     6,     8 },
    { CLS_J_DAMAGE_MAGE,      2,     4,     6,     8 },
    { CLS_S_DAMAGE_MAGE,      2,     4,     6,     8 },
    { CLS_J_HEAL_MAGE,        3,     5,     7,     9 },
    { CLS_S_HEAL_MAGE,        4,     6,     8,     10},
    // archer
    { CLS_PRIMARY_ARCHER,     2,     4,     6,     8 },
    { CLS_J_SNIPE_ARCHER,     2,     4,     6,     8 },
    { CLS_S_SNIPE_ARCHER,     2,     4,     6,     8 },
    { CLS_J_RAKE_ARCHER,      6,     5,     2,     7 },
    { CLS_S_RAKE_ARCHER,      6,     5,     2,     7 },
    // end
    { CLS_MAXCLS_LABEL,       0,     0,     0,     0 }
};

int GetClsPosWeight(int cls, int pos)
{
    if (cls < CLS_NEWBIE || cls >= CLS_MAXCLS_LABEL)
    {
        ASSERT(false);
        return 0;
    }

    if (pos < 0 || pos >= (int)kTeamMemberCount)
    {
        ASSERT(false);
        return 0;
    }

    return cls_weight_list[cls][pos+1];
}

} // Anonymous


bool TidyTeamPos(const TeamPosVec& raw, TeamPosVec& pos_info)
{
    int count = 0;
    for (size_t i = 0; i < raw.size(); ++i)
    {
        if (raw[i].roleid > 0)
            ++count;
        if (raw[i].cls < CLS_NEWBIE || raw[i].cls >= CLS_MAXCLS_LABEL)
            return false;
    }

    if (count <= 0)
        return false;

    TeamPosVec member = raw;
    while (count > 0)
    {
        int mem_i = -1;
        int pos_k = -1;
        int max_weight   = -1;
        int combat_value = -1;
        for (size_t i = 0; i < member.size(); ++i)
        {
            // 如果没有人
            if (member[i].roleid <= 0)
                continue;

            TeamPosDetail& ent = member[i];
            for (size_t k = 0; k < pos_info.size(); ++k)
            {
                // 如果该位置有人
                if (pos_info[k].roleid > 0)
                    continue;

                int weight = GetClsPosWeight(ent.cls, k);
                if (max_weight <= weight)
                {
                    // 如果权重相等，战斗力也相等
                    if (max_weight == weight && combat_value >= ent.combat_value)
                        continue;

                    max_weight   = weight;
                    combat_value = ent.combat_value;
                    mem_i = i;
                    pos_k = k;
                }
            }
        }

        if (mem_i >= 0)
        {
            ASSERT(pos_k >= 0);
            pos_info[pos_k] = member[mem_i];
            member[mem_i].clear();
            --count;
        }
    }

    for (size_t i = 0; i < raw.size(); ++i)
    {
        if (raw[i].roleid != pos_info[i].roleid)
            return true;
    }

    return false;
}

} // namespace team
} // namespace gamed
