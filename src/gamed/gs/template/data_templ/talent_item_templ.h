#ifndef __GAMED_GS_TEMPLATE_DATA_TEMPL_TALENT_ITEM_TEMPL_H__
#define __GAMED_GS_TEMPLATE_DATA_TEMPL_TALENT_ITEM_TEMPL_H__

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @class TalentItemTempl
 * @brief 天赋物品模板
 */
class TalentItemTempl : public ItemDataTempl
{
	DECLARE_ITEM_TEMPLATE(TalentItemTempl, TEMPL_TYPE_TALENT_ITEM);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }

    enum ITEM_FUNC_TYPE
    {
        IFT_INVALID,
        IFT_OPEN_TALENT_GROUP,
        IFT_OPEN_TALENT,
    };

    uint8_t item_func_type;  // 1:开启天赋组；2:开启天赋；
    TemplID talent_group_id; // 开启哪一个天赋组
    TemplID talent_id;       // 开启哪一个天赋


protected:
	virtual void OnMarshal()
	{
		MARSHAL_TEMPLVALUE(item_func_type, talent_group_id, talent_id);
	}

	virtual void OnUnmarshal()
	{
		UNMARSHAL_TEMPLVALUE(item_func_type, talent_group_id, talent_id);
	}

	virtual bool OnCheckDataValidity() const
	{
        if (item_func_type == IFT_OPEN_TALENT_GROUP)
        {
            if (talent_group_id <= 0)
                return false;
        }
        else if (item_func_type == IFT_OPEN_TALENT_GROUP)
        {
            if (talent_group_id <= 0 || talent_id <= 0)
                return false;
        }

		return true;
	}
};

}; // namespace dataTempl

#endif
