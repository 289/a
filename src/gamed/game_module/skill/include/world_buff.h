#ifndef SKILL_WORLD_BUFF_H_
#define SKILL_WORLD_BUFF_H_

#include "damage_msg.h"

namespace skill
{

enum WorldBuffType
{
    WORLD_BUFF_INVALID,
    WORLD_BUFF_INVINCIBLE,
    WORLD_BUFF_TRANSFORM,
    WORLD_BUFF_PROP,
};

struct WorldBuffInfo
{
    std::string name;
    std::string icon;
    std::string gfx;
    int32_t mutex_gid;
    int32_t prior;
    int32_t duration;
};

struct InvincibleBuffInfo : public WorldBuffInfo
{
};

struct TransformBuffInfo : public WorldBuffInfo
{
    std::string model;
};

struct PropBuffInfo : public WorldBuffInfo
{
    DamageVec dmg_vec;
};

int8_t GetWorldBuffType(EffectID id);
bool GetInvincibleBuffInfo(EffectID id, InvincibleBuffInfo& info);
bool GetTransformBuffInfo(EffectID id, TransformBuffInfo& info);
bool GetPropBuffInfo(EffectID id, Player* player, PropBuffInfo& info, bool host = true);

} // namespace skill

#endif // SKILL_WORLD_BUFF_H_
