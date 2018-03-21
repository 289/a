#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_MOUNT_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_MOUNT_CONFIG_H_

namespace dataTempl
{

struct MountVisibleInfo
{
    MountVisibleInfo()
        : mount_id(0), visible(true)
    {
    }

    int32_t mount_id;
    bool visible;

    NESTED_DEFINE(mount_id, visible);

    bool CheckDataValidity() const
    {
        return mount_id >= 0;
    }
};

class MountConfig
{
public:
    static const int32_t kMaxMountEquipNum = 4;
    static const int32_t kMaxMountNum = 64;

    BoundArray<int32_t, kMaxMountEquipNum> mount_equip_list;
    BoundArray<MountVisibleInfo, kMaxMountNum> mount_list;

    NESTED_DEFINE(mount_equip_list, mount_list);

    bool CheckDataValidity() const
    {
        for (size_t i = 0; i < mount_equip_list.size(); ++i)
        {
            if (mount_equip_list[i] < 0)
            {
                return false;
            }
        }
        for (size_t i = 0; i < mount_list.size(); ++i)
        {
            if (!mount_list[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_MOUNT_CONFIG_H_
