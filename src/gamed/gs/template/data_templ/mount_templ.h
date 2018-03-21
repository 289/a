#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_Mount_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_Mount_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum MountGetMothod
{
    MOUNT_GET_SPEC,
    MOUNT_GET_VIP,
    MOUNT_GET_MALL,
    MOUNT_GET_EXCHANGE,
};

/*
 * @brief: 卡牌模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class MountTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(MountTempl, TEMPL_TYPE_MOUNT);
public:
	static const int kMaxResPathLen = 512;
	static const int kMaxDescLen = UTF8_LEN(64);

	inline void set_templ_id(TemplID id) { templ_id = id; }

	BoundArray<uint8_t, kMaxResPathLen> mount_icon;             // 坐骑界面显示图标
	BoundArray<uint8_t, kMaxResPathLen> mount_select_icon;      // 坐骑选中显示图片
	BoundArray<uint8_t, kMaxResPathLen> move_action;            // 坐骑移动动作名
	BoundArray<uint8_t, kMaxResPathLen> stand_action;           // 坐骑站立动作名
	BoundArray<uint8_t, kMaxResPathLen> path_name;              // 坐骑的拼音名
    BoundArray<uint8_t, kMaxDescLen> spec_desc;                 // 特殊途径获取的文本说明
    int32_t move_speed_delta;                                     // 坐上坐骑后的移动速度变化值
    int8_t get_method;                                          // 获取条件
    int8_t vip_level;                                           // VIP等级
    int32_t exchange_item_id;                                   // 兑换物品ID
    int32_t exchange_item_count;                                // 兑换物品数量

protected:
    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(mount_icon, mount_select_icon, move_action, stand_action, path_name, spec_desc);
        MARSHAL_TEMPLVALUE(move_speed_delta, get_method, vip_level, exchange_item_id, exchange_item_count);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(mount_icon, mount_select_icon, move_action, stand_action, path_name, spec_desc);
        UNMARSHAL_TEMPLVALUE(move_speed_delta, get_method, vip_level, exchange_item_id, exchange_item_count);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (move_speed_delta < 0)
        {
            return false;
        }
        if (get_method < MOUNT_GET_SPEC || get_method > MOUNT_GET_EXCHANGE)
        {
            return false;
        }
        if (get_method == MOUNT_GET_VIP && vip_level <= 0)
        {
            return false;
        }
        if (get_method == MOUNT_GET_EXCHANGE && (exchange_item_id <= 0 || exchange_item_count <= 0))
        {
            return false;
        }
        if (path_name.empty())
        {
            return false;
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_MOUNT_TEMPL_H_
