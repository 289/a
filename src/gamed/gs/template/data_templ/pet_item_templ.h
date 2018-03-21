#ifndef GAMED_GS_TEMPLATE_DATATEMPL_PET_ITEM_TEMPL_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_PET_ITEM_TEMPL_H_

#include "base_datatempl.h"


namespace dataTempl
{

/**
 * @class PetItemTempl
 * @brief 宠物物品模板，玩家得到该物品时，宠物栏获得一个宠物
 * @brief 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class PetItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(PetItemTempl, TEMPL_TYPE_PET_ITEM);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

	int32_t pet_id;     // 宠物模板ID
	int16_t pet_level;  // 宠物初始普通等级
	int16_t pet_blevel; // 宠物初始血脉等级


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(pet_id, pet_level, pet_blevel);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(pet_id, pet_level, pet_blevel);
	}

	virtual bool OnCheckDataValidity() const
	{
		if (pet_id <= 0)
			return false;
		if (pet_level <= 0 || pet_level > 255)
			return false;
		if (pet_blevel <= 0 || pet_blevel > 255)
			return false;
		return true;
	}
};

}; // namespace dataTempl


#endif // GAMED_GS_TEMPLATE_DATATEMPL_PET_ITEM_TEMPL_H_
