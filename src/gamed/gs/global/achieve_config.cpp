#include "achieve_config.h"

#include <set>

#include "common/3rd/pugixml/inc/pugixml.hpp"
#include "dbgprt.h"


namespace gamed {

using namespace pugi;

AchieveConfig::AchieveConfig()
{
}

AchieveConfig::~AchieveConfig()
{
}

bool AchieveConfig::Init(const char* path)
{
    // 相当于所有物品都不能拍卖
    if (path == NULL)
        return true;

    xml_document doc;
    xml_parse_result result = doc.load_file(path);
    if (!result)
    {
        __PRINTF("Achieve config XML load_file error! %s", path);
        return false;
    }

    xml_node monsterNode = doc.child("Monster").first_child();
    for (; monsterNode; monsterNode = monsterNode.next_sibling())
    {
        int32_t monster_id = monsterNode.attribute("id").as_int();
        monster_filter_.insert(monster_id);
    }
    return true;
}

bool AchieveConfig::IsAchieveMonster(int32_t monster_id) const
{
    AchieveFilterSet::const_iterator it = monster_filter_.find(monster_id);
    return it != monster_filter_.end();
}

} // namespace gamed
