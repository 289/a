#ifndef GAMED_GS_TEMPLATE_DATATEMPL_PET_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_PET_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

class PetTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(PetTempl, TEMPL_TYPE_PET);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	static const int kMaxVisibleNameLen     = UTF8_LEN(16);     // 最多支持16个汉字
	static const int kMaxIconRsrcPathLen    = 512;              // 图标资源路径的最大长度
	static const int kMaxWindowRsrcPathLen  = 512;              // 图鉴资源路径的最大长度
	static const int kMaxGFXRsrcPathLen     = 512;              // 光效资源路径的最大长度
	static const int kMaxRankCount          = 10;               // 宠物成长阶段暂定为10个
	static const int kMaxRankBriefLen       = UTF8_LEN(64);     // 位阶简要描述的最大长度
	static const int kMaxRankDetailLen      = UTF8_LEN(128);    // 位阶详细描述的最大长度
	static const int kMaxMajorSkillLen      = UTF8_LEN(128);    // 主宠技能描述的最大长度

	struct RankEntry
	{
		int16_t blevel_limit;                                            // 最小血脉等级限制(即处于该头衔需要的最小血脉等级)
		SkillID skill_id;                                                // 本品阶所调用的技能
		int32_t skill_consume;                                           // 本品阶施放技能消耗多少能量值
		BoundArray<uint8_t, kMaxMajorSkillLen>      skill_description;    // 主宠技能描述
        BoundArray<uint8_t, kMaxVisibleNameLen>     visible_name;         // 游戏内显示的宠物名称
        BoundArray<uint8_t, kMaxIconRsrcPathLen>    icon_resource_path;   // 图标资源路径
        BoundArray<uint8_t, kMaxWindowRsrcPathLen>  window_resource_path; // 图鉴资源路径
        BoundArray<uint8_t, kMaxRankBriefLen>       brief_description;    // 位阶简要描述,宠物界面显示
        BoundArray<uint8_t, kMaxRankDetailLen>      detail_description;   // 位阶详细描述,宠物界面显示
        BoundArray<uint8_t, kMaxGFXRsrcPathLen>     shifa_path;         // 施法光效资源路径
        BoundArray<uint8_t, kMaxGFXRsrcPathLen>     chuxianshang_path;  // 出现上光效资源路径
        BoundArray<uint8_t, kMaxGFXRsrcPathLen>     chuxianxia_path;    // 出现下光效资源路径
        BoundArray<uint8_t, kMaxGFXRsrcPathLen>     fangshexian_path;   // 放射线光效资源路径
        BoundArray<uint8_t, kMaxGFXRsrcPathLen>     shutiao_path;       // 竖条光效资源路径
        int32_t shutiao_num;

        NESTED_DEFINE(blevel_limit, skill_id, skill_consume, skill_description, visible_name, icon_resource_path, window_resource_path, brief_description, detail_description, shifa_path, chuxianshang_path, chuxianxia_path, fangshexian_path, shutiao_path, shutiao_num);
	};
	BoundArray<RankEntry, kMaxRankCount> ranks;

	struct PropEntry
	{
		int32_t init_prop; //属性初始值
		int32_t lvlup_inc; //升级提升数值

		NESTED_DEFINE(init_prop, lvlup_inc);
	};
	BoundArray<PropEntry, MAX_PROP_NUM> properties;

	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(ranks, properties);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(ranks, properties);
	}

	virtual bool OnCheckDataValidity() const
	{
		for (size_t i = 0; i < ranks.size(); ++ i)
		{
			const RankEntry& entry = ranks[i];
			if (entry.blevel_limit <= 0 ||
				entry.skill_id <= 0 ||
				entry.skill_consume <= 0)
				return false;
		}

		for (size_t i = 0; i < properties.size(); ++ i)
		{
			const PropEntry& entry = properties[i];
			if (entry.init_prop < 0 || entry.lvlup_inc < 0)
				return false;
		}

        return true;
	}
};

};

#endif
