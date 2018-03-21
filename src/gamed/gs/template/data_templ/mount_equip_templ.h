#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_MOUNT_EQUIP_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_MOUNT_EQUIP_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

struct MountEquipLevelUp
{
    MountEquipLevelUp()
        : item_id(0), item_count(0), money(0), score(0)
    {
    }

    int32_t item_id;
    int32_t item_count;
    int32_t money;
    int32_t score;

    bool OnCheckDataValidity() const
    {
        return item_id >= 0 && item_count >= 0 && money >= 0 && score >= 0;
    }

    NESTED_DEFINE(item_id, item_count, money, score);
};

struct MountEquipProp
{
    MountEquipProp()
        : prop_index(-1), prop_base(0)
    {
    }

	static const int kMaxPropTableSize = 255;     // 属性提升表的大小

    int8_t prop_index;
    int32_t prop_base;
    BoundArray<int32_t, kMaxPropTableSize> prop_multi;
    BoundArray<int32_t, kMaxPropTableSize> prop_add;

    bool OnCheckDataValidity() const
    {
        if (prop_index < -1 || prop_index > 16 || prop_base < 0)
        {
            return false;
        }
        if (prop_multi.size() != prop_add.size())
        {
            return false;
        }
        for (size_t i = 0; i < prop_multi.size(); ++i)
        {
            if (prop_multi[i] < 0 || prop_add[i] < 0)
            {
                return false;
            }
        }
        return true;
    }

    NESTED_DEFINE(prop_index, prop_base, prop_multi, prop_add);
};

/*
 * @brief：骑具模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class MountEquipTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(MountEquipTempl, TEMPL_TYPE_MOUNT_EQUIP);
public:
    inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxNameLen        = UTF8_LEN(12);     // 最多支持12个汉字
    static const int kMaxResPathLen     = 512;
    static const int kMaxLevelCount     = 255;
    static const int kMaxPropCount      = 3;

    BoundArray<uint8_t, kMaxNameLen>    visible_name;       // 骑具名称
    BoundArray<uint8_t, kMaxResPathLen> equip_icon;         // 骑具图标
    BoundArray<MountEquipLevelUp, kMaxLevelCount> lvlup;    // 升级所需条件，0代表0级升1级所需要的条件，以此类推
    BoundArray<MountEquipProp, kMaxPropCount> prop;         // 骑具带来的属性加成，0代表0级时给自己带来的属性增强

    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(visible_name, equip_icon, lvlup, prop);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(visible_name, equip_icon, lvlup, prop);
    }

    virtual bool OnCheckDataValidity() const
    {
        for (size_t i = 0; i < lvlup.size(); ++i)
        {
            if (!lvlup[i].OnCheckDataValidity())
            {
                return false;
            }
        }
        for (size_t i = 0; i < prop.size(); ++i)
        {
            if (!prop[i].OnCheckDataValidity() || prop[i].prop_multi.size() < lvlup.size())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_MOUNT_EQUIP_TEMPL_H_
