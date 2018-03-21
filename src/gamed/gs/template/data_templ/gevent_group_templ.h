#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_GROUP_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_GROUP_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum GeventType
{
    GEVENT_TYPE_SINGLE,     // 单人活动
    GEVENT_TYPE_MULTI_INS,  // 多人副本
    GEVENT_TYPE_MULTI_BG,   // 多人战场
};

/*
 * @brief：活动组模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class GeventGroupTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(GeventGroupTempl, TEMPL_TYPE_GEVENT_GROUP);
public:
	static const int kMaxNameLen            = UTF8_LEN(16);     // 最多支持16个汉字
	static const int kMaxDescLen            = UTF8_LEN(256);    // 最多支持256个汉字
	static const int kMaxIconLen            = 512;              // 图标资源路径的最大长度
    static const int kMaxGeventCount        = 16;               // 每个活动组允许的活动模板数
    static const int kMaxTimeSegmentCount   = 24;               // 允许的时间段最大数目

    inline void set_templ_id(TemplID id) { templ_id = id; }

// 0
	BoundArray<uint8_t, kMaxNameLen> name;                      // 活动名称
	BoundArray<uint8_t, kMaxDescLen> desc;                      // 活动描述
	BoundArray<uint8_t, kMaxIconLen> icon;                      // 活动图标
    BoundArray<int32_t, kMaxGeventCount> gevent_list;           // 活动ID列表
    int8_t gevent_type;                                         // 活动类型，单人，多人

// 5
    int32_t min_level;                                          // 参与等级下限
    int32_t max_level;                                          // 参与等级上限
    BoundArray<TimeSegment, kMaxTimeSegmentCount> open_time;    // 活动开启时间段
    int32_t max_num;                                            // 每天允许参加的次数上限
    int32_t participation;                                      // 参加活动获得的活跃度

// 10
    int32_t world_boss_tid;                                     // 世界BOSS模板id，默认值：0


protected:
    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(name, desc, icon, gevent_list, gevent_type);
        MARSHAL_TEMPLVALUE(min_level, max_level, open_time, max_num, participation);
        MARSHAL_TEMPLVALUE(world_boss_tid);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(name, desc, icon, gevent_list, gevent_type);
        UNMARSHAL_TEMPLVALUE(min_level, max_level, open_time, max_num, participation);
        UNMARSHAL_TEMPLVALUE(world_boss_tid);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (gevent_type < GEVENT_TYPE_SINGLE || gevent_type > GEVENT_TYPE_MULTI_BG)
        {
            return false;
        }
        if (min_level < 0 || max_level < 0 || min_level > max_level)
        {
            return false;
        }
        for (size_t i = 0; i < open_time.size(); ++i)
        {
            if (open_time[i].is_valid && !CheckTimeSegment(open_time[i]))
            {
                return false;
            }
        }
        if (participation < 0)
        {
            return false;
        }
        if (world_boss_tid < 0)
        {
            return false;
        }

        return max_num >= -1;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_GROUP_TEMPL_H_
