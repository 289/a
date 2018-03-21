#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GOLEM_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GOLEM_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class GolemTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(GolemTempl, TEMPL_TYPE_GOLEM);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxModelPathLen   = 512;  // 召唤宠模型资源路径的最大长度
	static const int kMaxSkillCount     = 5;    // 最大技能个数
	static const int kMaxEffectCount    = 5;    // 最大效果个数

	BoundArray<SkillID, kMaxSkillCount>   skills;  //召唤单位的基础技能
	BoundArray<PropType, MAX_PROP_NUM>   properties; //召唤单位的基础属性
	BoundArray<uint8_t, kMaxModelPathLen> model_resource_path;//图标资源路径

	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(skills, properties, model_resource_path);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(skills, properties, model_resource_path);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < skills.size(); ++ i)
			if (skills[i] < 0)
				return false;

		for (size_t i = 0; i < properties.size(); ++ i)
			if (properties[i] < 0)
				return false;

		return true;
	}
};

};

#endif
