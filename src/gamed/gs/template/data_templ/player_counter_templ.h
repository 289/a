#ifndef GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_COUNTER_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_COUNTER_H_

#include "base_datatempl.h"


namespace dataTempl {

/*
 * @brief: 玩家计数器（提供给策划用的计数用，大部分是任务使用）
 *         需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 */
class PlayerCounterTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(PlayerCounterTempl, TEMPL_TYPE_PLAYER_COUNTER);
public:
    static const int kMaxCounterNameLen = UTF8_LEN(16); 

    enum CounterValidity
    {
        IS_VALID = 0, // 计数器生效
        IS_INVALID,   // 计数器失效
    };

	inline void set_templ_id(TemplID id) { templ_id = id; }

public:
    BoundArray<uint8_t, kMaxCounterNameLen> counter_name; // 计数器名字，用于任务或活动显示
    int32_t    initial_value; // 计数器的初始化值，默认值：0
    int8_t     validity;      // 计数器是否有效，对应CounterValidity枚举，默认值：IS_VALID

protected:
	virtual void OnMarshal() 
    { 
        MARSHAL_TEMPLVALUE(counter_name, initial_value, validity);
    }

    virtual void OnUnmarshal() 
    { 
        UNMARSHAL_TEMPLVALUE(counter_name, initial_value, validity);
    }

	virtual bool OnCheckDataValidity() const
	{
        if (validity != IS_VALID && validity != IS_INVALID)
            return false;

		return true;
	}
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_PLAYER_COUNTER_H_
