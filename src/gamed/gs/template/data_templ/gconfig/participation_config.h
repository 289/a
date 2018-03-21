#ifndef GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PARTICIPATION_CONFIG_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PARTICIPATION_CONFIG_H_

namespace dataTempl
{

struct ItemAward
{
    int32_t item_id;
    int32_t item_count;

    NESTED_DEFINE(item_id, item_count);

    bool CheckDataValidity() const
    {
        return item_id >= 0 && item_count >= 0;
    }
};

struct ParticipationAward
{
    static const int32_t kMaxItemNum = 3;

    int32_t participation;
    int32_t exp;
    int32_t money;
    int32_t score;
    BoundArray<ItemAward, kMaxItemNum> items;

    NESTED_DEFINE(participation, exp, money, score, items);

    bool CheckDataValidity() const
    {
        if (participation < 0 || exp < 0 || money < 0 || score < 0)
        {
            return false;
        }
        for (size_t i = 0; i < items.size(); ++i)
        {
            if (!items[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

class ParticipationConfig
{
public:
    static const int32_t kMaxAwardNum = 16;

    int32_t max_participation;
    BoundArray<ParticipationAward, kMaxAwardNum> awards;

    NESTED_DEFINE(max_participation, awards);

    bool CheckDataValidity() const
    {
        if (max_participation < 0)
            return false;
        for (size_t i = 0; i < awards.size(); ++i)
        {
            if (!awards[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // GAMED_GS_TEMPLATE_DATATEMPL_GCONFIG_PARTICIPATION_CONFIG_H_
