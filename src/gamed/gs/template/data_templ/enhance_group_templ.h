#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_GROUP_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_GROUP_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum EnhanceResetType
{
    ENHANCE_RESET_NONE,
    ENHANCE_RESET_TIME,
    ENHANCE_RESET_INTERVAL,
};

struct EnhanceEntry
{
    EnhanceEntry()
        : enhance_id(0), enhance_weight(0)
    {
    }

    int32_t enhance_id;
    int32_t enhance_weight;

    NESTED_DEFINE(enhance_id, enhance_weight);
};
typedef std::vector<EnhanceEntry> EnhanceList;

/*
 * @brief：附魔库模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class EnhanceGroupTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(EnhanceGroupTempl, TEMPL_TYPE_ENHANCE_GROUP);
public:
    inline void set_templ_id(TemplID id) { templ_id = id; }

    int32_t money;
    int32_t score;
    int32_t count;
    int8_t reset_type;
    int32_t reset_time;
    EnhanceList enhance_list;

    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(money, score, count, reset_type, reset_time, enhance_list);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(money, score, count, reset_type, reset_time, enhance_list);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (money < 0 || score < 0 || count < -1)
        {
            return false;
        }
        switch (reset_type)
        {
        case ENHANCE_RESET_NONE:
            return true;
        case ENHANCE_RESET_TIME:
            return CheckTimeEntry();
        case ENHANCE_RESET_INTERVAL:
            return reset_time > 0;
        default:
            return false;
        }
    }

    void SetInterval(int32_t time)
    {
        reset_time = time;
    }

    void SetTimeEntry(int8_t wday, int8_t hour, int8_t min)
    {
        reset_time = 0;
        reset_time = wday;
        reset_time <<= 4;
        reset_time |= hour;
        reset_time <<= 4;
        reset_time |= min;
    }

    void ExtractTime(int8_t& wday, int8_t& hour, int8_t& min) const
    {
        min = reset_time & 0x0000000F;
        hour = (reset_time >> 4) & 0x0000000F;
        wday = (reset_time >> 8) & 0x0000000F;
    }
private:
    bool CheckTimeEntry() const
    {
        int8_t wday = 0, hour = 0, min = 0;
        ExtractTime(wday, hour, min);
        return wday >= -1 && wday <= 6 && hour >= 0 && hour <= 23 && min >= 0 && min <= 59;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_GROUP_TEMPL_H_
