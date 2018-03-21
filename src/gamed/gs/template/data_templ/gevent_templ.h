#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum GeventJoinType
{
    GEVENT_JOIN_TASK,           // 发放任务
    GEVENT_JOIN_INSTANCE,       // 进入副本
    GEVENT_JOIN_BATTLEGROUND,   // 进入战场
};

/*
 * @brief：活动模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class GeventTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(GeventTempl, TEMPL_TYPE_GEVENT);
public:
    inline void set_templ_id(TemplID id) { templ_id = id; }

    int32_t min_level;                              // 参与等级下限
    int32_t max_level;                              // 参与等级上限

    int8_t join_type;                               // 活动参与方式
    std::vector<int32_t> task_list;                 // 发放任务列表
    int32_t ins_group_id;                           // 副本组ID
    int32_t ins_id;                                 // 副本ID
    int32_t battleground_id;                        // 战场ID

protected:
    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(min_level, max_level);
        MARSHAL_TEMPLVALUE(join_type, task_list, ins_group_id, ins_id, battleground_id);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(min_level, max_level);
        UNMARSHAL_TEMPLVALUE(join_type, task_list, ins_group_id, ins_id, battleground_id);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (join_type < GEVENT_JOIN_TASK || join_type > GEVENT_JOIN_BATTLEGROUND)
        {
            return false;
        }
        if (min_level < 0 || max_level < 0 || min_level > max_level)
        {
            return false;
        }
        if (join_type == GEVENT_JOIN_TASK && task_list.empty())
        {
            return false;
        }
        if (join_type == GEVENT_JOIN_INSTANCE && (ins_group_id == 0 || ins_id == 0))
        {
            return false;
        }
        if (join_type == GEVENT_JOIN_BATTLEGROUND && battleground_id == 0)
        {
            return false;
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_GEVENT_TEMPL_H_
