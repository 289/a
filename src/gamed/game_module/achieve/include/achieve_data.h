#ifndef ACHIEVE_ACHIEVE_DATA_H_
#define ACHIEVE_ACHIEVE_DATA_H_

#include "achieve_types.h"

#define DECLARE_ACHIEVE(name, type) \
	DECLARE_PACKET_COMMON_TEMPLATE(name, type, shared::net::BasePacket)

#define ACHIEVE_DEFINE PACKET_DEFINE

namespace achieve
{

enum AchieveType
{
    ACHIEVE_ACTIVE,
    ACHIEVE_FINISH,
    ACHIEVE_DATA,
};

enum AchieveState
{
    ACHIEVE_STATE_AWARD = 0x01, // 是否领取奖励
};

class ActiveAchieve
{
public:
    AchieveMap achieve_list;
};

class AchieveEntry
{
public:
    AchieveEntry()
        : state(0), templ(NULL)
    {
    }

    inline void SetAward();
    inline bool IsAward() const;
public:
    int8_t state;

    const AchieveTempl* templ;

    NESTED_DEFINE(state);
};
typedef std::map<AchieveID, AchieveEntry> EntryMap;

class FinishAchieve : public shared::net::BasePacket
{
    DECLARE_ACHIEVE(FinishAchieve, ACHIEVE_FINISH);
public:
    EntryMap achieve_list;
    ACHIEVE_DEFINE(achieve_list);

    inline AchieveEntry* GetEntry(AchieveID id);
    void LoadComplete();
};

typedef std::map<int32_t, int32_t> SubAchieveDataMap;
typedef std::map<int32_t, SubAchieveDataMap> AchieveDataMap;
class AchieveData : public shared::net::BasePacket
{
    DECLARE_ACHIEVE(AchieveData, ACHIEVE_DATA);
public:
    AchieveDataMap data;
    ACHIEVE_DEFINE(data);
};

inline void AchieveEntry::SetAward()
{
    state |= ACHIEVE_STATE_AWARD;
}

inline bool AchieveEntry::IsAward() const
{
    return state & ACHIEVE_STATE_AWARD;
}

inline AchieveEntry* FinishAchieve::GetEntry(AchieveID id)
{
    EntryMap::iterator it = achieve_list.find(id);
    return it == achieve_list.end() ? NULL : &(it->second);
}

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_DATA_H_
