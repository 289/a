#ifndef GAMED_GS_TEMPLATE_DATATEMPL_COOLDOWN_GROUP_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_COOLDOWN_GROUP_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 需要在base_datatempl.cpp中添加INIT语句才能生效（Clone生效）
 *    1.冷却组模板
 */
class CoolDownGroupTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(CoolDownGroupTempl, TEMPL_TYPE_COOLDOWN_GROUP);
public:
	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
// 0
    int32_t cd_group_time; // 冷却组的冷却时间，单位毫秒，默认值：2000


protected:
	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(cd_group_time);
    }

	virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(cd_group_time);
    }

	virtual bool OnCheckDataValidity() const
    {
        if (cd_group_time <= 50 )
            return false;

        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_COOLDOWN_GROUP_H_
