#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TALENT_GROUP_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TALENT_GROUP_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 天赋组模板
 */
class TalentGroupTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(TalentGroupTempl, TEMPL_TYPE_TALENT_GROUP);
public:
	static const int kMaxVisibleNameLen	= UTF8_LEN(8);      // 最多支持8个汉字
	static const int kMaxTalentCount    = 50;               // 天赋组最多可以有多少个天赋
	static const int kMaxIconPathLen    = 512;              // 图标资源路径的最大长度
	static const int kMaxDescriptionLen = UTF8_LEN(128);    // 最多支持128个汉字

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
    uint8_t init_open;                                     // 初始是否开启
    BoundArray<uint8_t, kMaxIconPathLen>    icon_path;     // 天赋组图标路径
	BoundArray<uint8_t, kMaxVisibleNameLen> visible_name;  // 特定等级对应的显示名称
	BoundArray<TemplID, kMaxTalentCount>    talent_list;   // 天赋列表
	BoundArray<uint8_t, kMaxDescriptionLen> description;   // 天赋组描述


protected:
	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(init_open, icon_path, visible_name, talent_list, description);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(init_open, icon_path, visible_name, talent_list, description);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < talent_list.size(); ++ i)
		{
			TemplID talent_id = talent_list[i];
			if (talent_id <= 0)
				return false;
		}
		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_TALENT_GROUP_TEMPL_H_
