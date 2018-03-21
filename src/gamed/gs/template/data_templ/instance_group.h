#ifndef GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_GROUP_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_GROUP_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 */
class InstanceGroup : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(InstanceGroup, TEMPL_TYPE_INSTANCE_GROUP);
public:
	static const int kMaxInsNameLen     = UTF8_LEN(8);
	static const int kMaxInsBriefLen    = UTF8_LEN(256);
	static const int kInsStartUpCondLen = UTF8_LEN(256);
	static const int kMaxDisplayPicPath = 512;
	static const int kMaxInsDropItem    = 64;

	inline void set_templ_id(TemplID id) { templ_id = id; }

	enum InsLevelDef
	{
		IL_NORMAL = 0, // 普通
		IL_HERO,       // 英雄   
		IL_CHALLENGE,  // 挑战
		IL_HELL,       // 地狱
		IL_GOD,        // 超神
		IL_MAX_COUNT
	};

	struct InsInfo
	{
		int32_t ins_level;     // 对应枚举InsLevelDef
		int32_t ins_templ_id;  // 对应的副本模板id
		int32_t combat_value;  // 副本推荐战斗力
		BoundArray<uint8_t, kInsStartUpCondLen> startup_cond;    // 副本开启条件
		BoundArray<int32_t, kMaxInsDropItem>    drop_item_array; // 副本掉落物品，只用于显示
		NESTED_DEFINE(ins_level, ins_templ_id, combat_value, startup_cond, drop_item_array);
	};

	typedef BoundArray<InsInfo, IL_MAX_COUNT> InsInfoArray;


public:
// 0
	BoundArray<uint8_t, kMaxInsNameLen>     ins_name;   // 副本显示名字
	BoundArray<uint8_t, kMaxInsBriefLen>    ins_brief;  // 副本简介
	BoundArray<uint8_t, kMaxDisplayPicPath> ins_display_pic; // 副本大厅中显示图片，默认值：空 表示使用默认图片
	int8_t    display_not_cond; // 没有可进入副本时是否显示，默认值：0表示不显示，1表示显示
	BoundArray<InsInfo, IL_MAX_COUNT> ins_array; // 副本组，只填入有效值

// 5


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(ins_name, ins_brief, ins_display_pic, display_not_cond, ins_array);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(ins_name, ins_brief, ins_display_pic, display_not_cond, ins_array);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (display_not_cond != 0 && display_not_cond != 1)
			return false;

		if (!CheckInstanceArray(ins_array))
			return false;

		return true;
	}

private:
	bool CheckInstanceArray(const InsInfoArray& ins_array) const
	{
		for (size_t i = 0; i < ins_array.size(); ++i)
		{
			// 检查枚举
			if (ins_array[i].ins_level < IL_NORMAL || 
				ins_array[i].ins_level >= IL_MAX_COUNT)
			{
				return false;
			}

			for (size_t j = 0; j < ins_array.size(); ++j)
			{
				// 同一级别不能填两个副本
				if (i != j && ins_array[i].ins_level == ins_array[j].ins_level)
				{
					return false;
				}
			}
		}

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_INSTANCE_GROUP_H_
