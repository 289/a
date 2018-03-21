#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_STAR_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_STAR_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief：星火模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
struct SparkProp
{
    SparkProp()
        : prop(0), value(0)
    {
    }

    int8_t prop;
    int32_t value;

    NESTED_DEFINE(prop, value);

    bool CheckDataValidity() const
    {
        return prop >= 0 && prop <= 15 && value >= 0;
    }
};
typedef std::vector<SparkProp> SparkPropList;

// 星辰模板
class StarTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(StarTempl, TEMPL_TYPE_STAR);
public:
    inline void set_templ_id(TemplID id) { templ_id = id; }

    std::string name;
    std::string desc;
    std::string icon;
    std::string background;
    int8_t rank;
    std::vector<int32_t> activate_star;
    int32_t spark_num;
    int32_t spark_activate_money;
    int32_t spark_activate_score;
    SparkPropList prop_list;

    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(name, desc, icon, background, rank, activate_star, spark_num, spark_activate_money, spark_activate_score, prop_list);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(name, desc, icon, background, rank, activate_star, spark_num, spark_activate_money, spark_activate_score, prop_list);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (name.length() > UTF8_LEN(32) || desc.length() > UTF8_LEN(128))
        {
            return false;
        }
        if (rank < 0 || spark_num <= 0 || spark_activate_money < 0 || spark_activate_score < 0 || activate_star.size() > 3 || prop_list.size() > 16)
        {
            return false;
        }
        for (size_t i = 0; i < activate_star.size(); ++i)
        {
            if (activate_star[i] < 0)
            {
                return false;
            }
        }
        for (size_t i = 0; i < prop_list.size(); ++i)
        {
            if (!prop_list[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_STAR_TEMPL_H_
