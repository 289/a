#include "skill_if.h"

#include "player.h"


namespace gamed {

PlayerSkillIf::PlayerSkillIf(Player* player)
    : pplayer_(player)
{
}

PlayerSkillIf::~PlayerSkillIf()
{
}

int32_t PlayerSkillIf::GetProperty(int32_t index) const
{
    if (index >= 1000)
    {
        //比例提升和降低根据基础属性计算,所以这里做了特殊处理,index>=1000表示请求基础属性
        return pplayer_->GetBaseProp(index-1000);
    }
    
    return pplayer_->GetMaxProp(index);
}

} // namespace gamed
