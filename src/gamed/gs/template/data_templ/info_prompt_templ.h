#ifndef GAMED_GS_TEMPLATE_DATATEMPL_INFO_PROMPT_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_INFO_PROMPT_H_

#include "base_datatempl.h"


namespace dataTempl {

/*
 * @brief: 消息提示模板
 * @brief: 需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *        （1）本模板只用于客户端
 */
class InfoPromptTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(InfoPromptTempl, TEMPL_TYPE_INFO_PROMPT);
public:
	static const int kMaxInfoLen = UTF8_LEN(128);

// 0
    int16_t event; // 发生的事件
    int32_t mask;  // 提示消息显示在哪个区域
    BoundString<kMaxInfoLen> content; // 提示消息的内容


protected:
	virtual void OnMarshal()
	{
        MARSHAL_TEMPLVALUE(event, mask, content);
	}

	virtual void OnUnmarshal()
	{
        UNMARSHAL_TEMPLVALUE(event, mask, content);
	}

	virtual bool OnCheckDataValidity() const
	{
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_INFO_PROMPT_H_
