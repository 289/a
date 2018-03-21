#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_ENHANCE_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_ENHANCE_CONFIG_H_

namespace dataTempl {

/**
 * 附魔全局信息配置表
 */
class EnhanceConfig
{
public:
    int32_t open_price; // 开启附魔位的基础价格

    NESTED_DEFINE(open_price);

    bool CheckDataValidity() const
    {
        return open_price > 0;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_ENHANCE_CONFIG_H_
