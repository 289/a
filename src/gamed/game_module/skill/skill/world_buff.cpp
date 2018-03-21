#include "world_buff.h"
#include "skill_templ_manager.h"
#include "obj_interface.h"
#include "skill_def.h"
#include "effect_templ.h"
#include "active_point.h"

namespace skill
{

static int8_t OnGetWorldBuffType(const EffectTempl* templ)
{
    size_t size = templ->logic.size();
    if (size == 0)
    {
        return WORLD_BUFF_INVALID;
    }
    // 大世界Buff只允许存在一类技能效果
    int8_t type = templ->logic[0].type;
    for (size_t i = 1; i < size; ++i)
    {
        if (templ->logic[i].type != type)
        {
            return WORLD_BUFF_INVALID;
        }
    }
    switch (type)
    {
    case LOGIC_INVINCIBLE:
        return WORLD_BUFF_INVINCIBLE;
    case LOGIC_TRANSFORM:
        return WORLD_BUFF_TRANSFORM;
    case LOGIC_CHANGE_PROP:
        return WORLD_BUFF_PROP;
    default:
        return WORLD_BUFF_INVALID;
    }
}

int8_t GetWorldBuffType(EffectID id)
{
    const EffectTempl* templ = GetEffectTempl(id);
    return OnGetWorldBuffType(templ);
}

static bool GetWorldBuffInfo(const EffectTempl* templ, int8_t type, WorldBuffInfo& info)
{
    if (templ == NULL || OnGetWorldBuffType(templ) != type)
    {
        return false;
    }
    info.name = templ->name;
    info.icon = templ->icon_path;
    info.gfx = templ->buff_gfx_path;
    info.mutex_gid = templ->mutex.gid;
    info.prior = templ->mutex.prior;
    info.duration = templ->buff.time;
    return true;
}

bool GetInvincibleBuffInfo(EffectID id, InvincibleBuffInfo& info)
{
    const EffectTempl* templ = GetEffectTempl(id);
    if (!GetWorldBuffInfo(templ, WORLD_BUFF_INVINCIBLE, info))
    {
        return false;
    }
    return true;
}

bool GetTransformBuffInfo(EffectID id, TransformBuffInfo& info)
{
    const EffectTempl* templ = GetEffectTempl(id);
    if (!GetWorldBuffInfo(templ, WORLD_BUFF_TRANSFORM, info))
    {
        return false;
    }
    assert(templ->logic[0].params.size() == 1);
    info.model = templ->logic[0].params[0];
    return true;
}

bool GetPropBuffInfo(EffectID id, Player* player, PropBuffInfo& info, bool host)
{
    const EffectTempl* templ = GetEffectTempl(id);
    if (!GetWorldBuffInfo(templ, WORLD_BUFF_PROP, info))
    {
        return false;
    }

    if (host)
    {
        // 获取Buff技能伤害修正参数
        EffectDamage effect_dmg;
        CastParam param;
        param.factor = 1.0f;
        param.defenders = 1;
        param.crit = false;
        param.dmg = &effect_dmg;
        InnerMsg msg(NULL, &param);

        ActivePoint point(player, player, templ);
        point.WorldBuffChangeProp(msg);

        info.dmg_vec = effect_dmg.dmgs;
    }
    return true;
}

} // namespace skill
