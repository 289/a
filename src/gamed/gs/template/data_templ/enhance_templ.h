#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

enum EnhanceType
{
    ENHANCE_TYPE_PROP,
    ENHANCE_TYPE_PASSIVE_SKILL,
    ENHANCE_TYPE_SKILL_LEVEL,
};

/*
 * @brief：附魔模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class EnhanceTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(EnhanceTempl, TEMPL_TYPE_ENHANCE);
public:
    inline void set_templ_id(TemplID id) { templ_id = id; }

    std::string name;
    std::string desc;
    std::string icon;
    int32_t mutex_gid;
    int32_t lock_price;
    int32_t level;
    int8_t quality;
    int8_t enhance_type;
    std::vector<int32_t> enhance_param;

    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(name, desc, icon, mutex_gid, lock_price, level, quality, enhance_type, enhance_param);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(name, desc, icon, mutex_gid, lock_price, level, quality, enhance_type, enhance_param);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (level < 0 || quality < 0 || mutex_gid < -1 || lock_price < 1)
        {
            return false;
        }
        switch (enhance_type)
        {
        case ENHANCE_TYPE_PROP:
            return CheckPropParam();
        case ENHANCE_TYPE_PASSIVE_SKILL:
            return CheckPassiveSkillParam();
        case ENHANCE_TYPE_SKILL_LEVEL:
            return CheckSkillLevelParam();
        default:
            return false;
        }
    }

    void EnhanceProp(int32_t prop_index, int32_t prop_value)
    {
        enhance_param.clear();
        enhance_param.push_back(prop_index);
        enhance_param.push_back(prop_value);
    }

    void EnhancePassiveSkill(int32_t skill_id)
    {
        enhance_param.clear();
        enhance_param.push_back(skill_id);
    }

    void EnhanceSkillLevel(const std::vector<int32_t>& skill_gid, int32_t level)
    {
        enhance_param.clear();
        enhance_param.push_back(level);
        enhance_param.insert(enhance_param.end(), skill_gid.begin(), skill_gid.end());
    }
private:
    bool CheckPropParam() const
    {
        if (enhance_param.size() != 2)
        {
            return false;
        }
        return enhance_param[0] >= 0 && enhance_param[0] <= 15 && enhance_param[1] > 0;
    }

    bool CheckPassiveSkillParam() const
    {
        return enhance_param.size() == 1 && enhance_param[0] > 0;
    }

    bool CheckSkillLevelParam() const
    {
        if (enhance_param.size() < 1 || enhance_param.size() > 21)
        {
            return false;
        }
        for (size_t i = 0; i < enhance_param.size(); ++i)
        {
            if (enhance_param[i] <= 0)
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_ENHANCE_TEMPL_H_
