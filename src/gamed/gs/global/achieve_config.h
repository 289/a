#ifndef GAMED_GS_GLOBAL_ACHIEVE_CONFIG_H_
#define GAMED_GS_GLOBAL_ACHIEVE_CONFIG_H_

#include <map>
#include <set>
#include <stdint.h>

#include "shared/base/singleton.h"

namespace gamed 
{

class AchieveConfig : public shared::Singleton<AchieveConfig>
{
    friend class shared::Singleton<AchieveConfig>;
public:
    static inline AchieveConfig* GetInstance() {
		return &(get_mutable_instance());
	}

    bool Init(const char* path);
    bool IsAchieveMonster(int32_t monster_id) const;

protected:
    AchieveConfig();
    ~AchieveConfig();

private:
    typedef std::set<int32_t> AchieveFilterSet;
    AchieveFilterSet monster_filter_;
};


#define s_pAchieveCfg gamed::AchieveConfig::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_ACHIEVE_CONFIG_H_
