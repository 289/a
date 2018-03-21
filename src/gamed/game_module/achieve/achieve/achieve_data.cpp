#include "achieve_data.h"
#include "achieve_manager.h"
#include "achieve_templ.h"

namespace achieve
{

void FinishAchieve::LoadComplete()
{
    EntryMap::iterator ait = achieve_list.begin();
    for (; ait != achieve_list.end();)
    {
        AchieveEntry& entry = ait->second;
        entry.templ = s_pAchieve->GetAchieve(ait->first);
        if (entry.templ == NULL)
        {
            achieve_list.erase(ait++);
            continue;
        }
        ++ait;
    }
}

} // namespace achieve
