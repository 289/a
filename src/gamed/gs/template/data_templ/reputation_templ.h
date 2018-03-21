#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_REPUTATION_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_REPUTATION_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum ReputationFlag
{
    REPUTATION_FLAG_HIDE = 0x01,
};

struct ReputationLevel
{
    ReputationLevel()
        : point(0), color(0)
    {
    }

    int32_t point;
    int32_t color;
    BoundArray<uint8_t, UTF8_LEN(12)> name;

    NESTED_DEFINE(point, color, name);
};

/*
 * @brief：声望模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class ReputationTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(ReputationTempl, TEMPL_TYPE_REPUTATION);
public:
	static const int kMaxNameLen   = UTF8_LEN(12);
    static const int kMaxDescLen   = UTF8_LEN(128);
    static const int kMaxLevelSize = 16;

    typedef BoundArray<ReputationLevel, kMaxLevelSize> LevelList;
    inline void set_templ_id(TemplID id) { templ_id = id; }
    inline bool reputation_show() const { return !(flag & REPUTATION_FLAG_HIDE); }

    int8_t type;
    int8_t flag;
    int32_t min_value;
    int32_t max_value;
    int32_t init_value;
    BoundArray<uint8_t, kMaxNameLen> name;
    BoundArray<uint8_t, kMaxDescLen> desc;
    LevelList level_list;

    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(type, flag, min_value, max_value, init_value, name, desc, level_list);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(type, flag, min_value, max_value, init_value, name, desc, level_list);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (init_value < min_value || init_value > max_value || 
            min_value >= max_value || min_value < 0)
        {
            return false;
        }

        int32_t rep = -1;
        for (size_t i = 0; i < level_list.size(); ++i)
        {
            const ReputationLevel& ent = level_list[i];
            if (ent.point <= rep)
                return false;

            rep = ent.point;
        }

        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_REPUTATION_TEMPL_H_
