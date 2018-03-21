#ifndef GAMED_GS_TEMPLATE_DATATEMPL_WORLD_BOSS_AWARD_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_WORLD_BOSS_AWARD_H_

#include "base_datatempl.h"


namespace dataTempl {

/**
 * @brief 世界BOSS奖励
 */
class WorldBossAwardTempl : public BaseDataTempl
{
	DECLARE_DATATEMPLATE(WorldBossAwardTempl, TEMPL_TYPE_WORLDBOSS_AWARD);
public:
    static const int kMaxAwardItemCount = 5;  // 邮件最大只能发5个物品
    static const int kMaxAwardRanking   = 16; // 最大分几级

    struct ItemInfo
    {
        int32_t tid;   // 物品模板id
        int32_t count; // 物品的数量，不能大于物品的堆叠上限（在编辑器里标注）
        NESTED_DEFINE(tid, count);
    };

    struct Entry
    {
        int32_t value;  // 具体排名等级数值（排名前几的是这一等级）
        BoundArray<ItemInfo, kMaxAwardItemCount> item_list; // 奖励物品列表
        NESTED_DEFINE(value, item_list);
    };

	inline void set_templ_id(TemplID id) { templ_id = id; }


public:
    BoundArray<Entry, kMaxAwardRanking> ranking;


protected:
	virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(ranking);
    }
	
    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(ranking);
    }
	
	virtual bool OnCheckDataValidity() const
    {
        for (size_t i = 0; i < ranking.size(); ++i)
        {
            const Entry& ent = ranking[i];
            if (ent.value <= 0)
                return false;

            if (i != 0)
            {
                // 必须是逐步递增的
                if (ent.value <= ranking[i-1].value)
                    return false;
            }

            if (ent.item_list.size() <= 0)
                return false;

            for (size_t j = 0; j < ent.item_list.size(); ++j)
            {
                if (ent.item_list[j].tid <= 0 || ent.item_list[j].count <= 0)
                    return false;
            }
        }

        return true;
    }
};

} // namespace dataTempl


#endif // GAMED_GS_TEMPLATE_DATATEMPL_WORLD_BOSS_AWARD_H_
