#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_EQUIP_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_EQUIP_TEMPL_H__

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief: 武器模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class EquipTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(EquipTempl, TEMPL_TYPE_EQUIP);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

// 0
	BoundArray<uint8_t, 512> model_file; // 模型资源文件
	int8_t init_slot_count;              // 初始槽位个数
	int8_t extend_slot_count;            // 扩展槽位个数
	int8_t equip_type;                   // 装备类型
	uint32_t equip_mask;                 // 装备位置(mask值)

// 5
	PlayerClsMask cls_limit;             // 职业限制(mask值)
	uint32_t lvl_limit;                  // 等级限制
	TemplID  refine_tpl_id;              // 精炼配置表ID
	BoundArray<int32_t, MAX_PROP_NUM> addon_list;  // 附加属性


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(model_file, init_slot_count, extend_slot_count, equip_type);
		MARSHAL_TEMPLVALUE(equip_mask, cls_limit, lvl_limit, refine_tpl_id, addon_list);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(model_file, init_slot_count, extend_slot_count, equip_type);
		UNMARSHAL_TEMPLVALUE(equip_mask, cls_limit, lvl_limit, refine_tpl_id, addon_list);
	}

	virtual bool OnCheckDataValidity() const
	{
		return pile_limit == 1 &&
		       equip_mask >= 0 &&
			   equip_type > 0 &&
			   lvl_limit >= 0 &&
			   refine_tpl_id >= 0 &&
			   init_slot_count >= 0 &&
			   extend_slot_count >= 0 &&
			   addon_list.size() == MAX_PROP_NUM;
	}
};

}; // namespace dataTempl

#endif // __GAMED_GS_TEMPLATE_DATA_TEMPL_EQUIP_TEMPL_H__
