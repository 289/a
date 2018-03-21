#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_CARD_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_CARD_CONFIG_H_

namespace dataTempl
{

class CardConfig
{
public:
    int32_t cash_consume_per_exp; // 使用人民币升级卡牌时，每一点经验对应的人民币价格

    NESTED_DEFINE(cash_consume_per_exp);

    bool CheckDataValidity() const
    {
        return cash_consume_per_exp >= 0;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_CARD_CONFIG_H_
