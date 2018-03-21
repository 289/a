#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_TITLE_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_TITLE_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/*
 * @brief: 称号模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class TitleTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(TitleTempl, TEMPL_TYPE_TITLE);
public:
	static const int kMaxTitleNameLen    = UTF8_LEN(12);  // 最多支持12个汉字
	static const int kMaxModelPathLen    = UTF8_LEN(256); // 最大美术资源路径的长度
	static const int kMaxBriefDespLen    = UTF8_LEN(20);  // 称号简述的最大长度
	static const int kMaxDetailDespLen   = UTF8_LEN(256); // 称号描述的最大长度

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
	// 0
	BoundArray<uint8_t, kMaxTitleNameLen>  title_name;     // 称号名称
	BoundArray<uint8_t, kMaxModelPathLen>  model_path;     // 称号美术资源的路径
	BoundArray<uint8_t, kMaxBriefDespLen>  brief_desp;     // 称号的简要描述
	BoundArray<uint8_t, kMaxDetailDespLen> detail_desp;    // 称号的详细描述

	// 4
    int8_t quality;                                        // 称号品质
    int8_t type;                                           // 称号类型
    int8_t display_switch;                                 // 是否显示的开关
    BoundArray<PropType, MAX_PROP_NUM> addon_props;        // 获得称号时的附加属性
    BoundArray<SkillID, 3> addon_passive_skills;           // 获得称号时的附加被动技能


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(title_name, model_path, brief_desp, detail_desp);
		MARSHAL_TEMPLVALUE(quality, type, display_switch, addon_props, addon_passive_skills);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(title_name, model_path, brief_desp, detail_desp);
		UNMARSHAL_TEMPLVALUE(quality, type, display_switch, addon_props, addon_passive_skills);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (type < 0)
            return false;

        for (size_t i = 0; i < addon_props.size(); ++ i)
            if (addon_props[i] < 0)
                return false;

        for (size_t i = 0; i < addon_passive_skills.size(); ++ i)
            if (addon_passive_skills[i] <= 0)
                return false;

		return true;
	}
};

}; // namespace dataTempl

#endif //  _GAMED_GS_TEMPLATE_DATA_TEMPL_TITLE_TEMPL_H_
