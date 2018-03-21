#ifndef _GAMED_GS_TEMPLATE_DATA_TEMPL_BOSS_CHALLENGE_TEMPL_H_
#define _GAMED_GS_TEMPLATE_DATA_TEMPL_BOSS_CHALLENGE_TEMPL_H_

#include "base_datatempl.h"

namespace dataTempl
{

/*
 * @brief：界面BOSS挑战模板
 * @brief：需要在base_datatempl.cpp中添加INIT_STATIC_INSTANCE才可以生效
 *
 */
class BossChallengeTempl : public BaseDataTempl
{
    DECLARE_DATATEMPLATE(BossChallengeTempl, TEMPL_TYPE_BOSS_CHALLENGE);
public:
	static const int32_t kMaxChallengeNameLen   = UTF8_LEN(4);      // 最多支持4个汉字
	static const int32_t kMaxBossNameLen        = UTF8_LEN(12);     // 最多支持12个汉字
	static const int32_t kMaxBossDescLen        = UTF8_LEN(128);    // 最多支持128个汉字
	static const int32_t kMaxResPathLen         = 512;              // 美术资源路径长度
    static const int32_t kMaxClearAwardNum      = 4;
    static const int32_t kMaxWinAwardNum        = 2;
    static const int32_t kMaxBossNum            = 6;

    struct ItemEntry
    {
        ItemEntry()
            : item_id(0), item_num(0)
        {
        }

        int32_t item_id;
        int32_t item_num;

        NESTED_DEFINE(item_id, item_num);

        bool CheckDataValidity() const
        {
            return item_id >= 0 && item_num >= 0;
        }
    };

    struct BossEntry
    {
        BossEntry()
            : recommend_combat_value(0), monster_id(0), monster_gid(0), battle_scene_id(0), win_award_money(0), win_award_score(0)
        {
        }

        BoundArray<uint8_t, kMaxBossNameLen> name;
        BoundArray<uint8_t, kMaxBossDescLen> desc;
        BoundArray<uint8_t, kMaxResPathLen> resource;
        int32_t recommend_combat_value;
        int32_t monster_id;
        int32_t monster_gid;
        int32_t battle_scene_id;
        int32_t win_award_money;
        int32_t win_award_score;
        BoundArray<ItemEntry, kMaxWinAwardNum> win_award_item;

        NESTED_DEFINE(name, desc, resource, recommend_combat_value, monster_id, monster_gid, battle_scene_id, win_award_money, win_award_score, win_award_item);

        bool CheckDataValidity() const
        {
            if (recommend_combat_value < 0 || monster_id < 0 || monster_gid < 0 || battle_scene_id < 0 || win_award_money < 0 || win_award_score < 0)
            {
                return false;
            }
            for (size_t i = 0; i < win_award_item.size(); ++i)
            {
                if (!win_award_item[i].CheckDataValidity())
                {
                    return false;
                }
            }
            return true;
        }
    };

    inline void set_templ_id(TemplID id) { templ_id = id; }

    BoundArray<uint8_t, kMaxChallengeNameLen> name;                 // 挑战组名称
    int32_t clear_award_money;                                      // 通关金币奖励
    int32_t clear_award_score;                                      // 通关学分奖励
    BoundArray<ItemEntry, kMaxClearAwardNum> clear_award_item;      // 通关物品奖励
    BoundArray<BossEntry, kMaxBossNum> boss_list;                   // 挑战组中的BOSS列表


    virtual void OnMarshal()
    {
        MARSHAL_TEMPLVALUE(name, clear_award_money, clear_award_score, clear_award_item, boss_list);
    }

    virtual void OnUnmarshal()
    {
        UNMARSHAL_TEMPLVALUE(name, clear_award_money, clear_award_score, clear_award_item, boss_list);
    }

    virtual bool OnCheckDataValidity() const
    {
        if (clear_award_money < 0 || clear_award_score < 0)
        {
            return false;
        }
        for (size_t i = 0; i < clear_award_item.size(); ++i)
        {
            if (!clear_award_item[i].CheckDataValidity())
            {
                return false;
            }
        }
        for (size_t i = 0; i < boss_list.size(); ++i)
        {
            if (!boss_list[i].CheckDataValidity())
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace dataTempl

#endif // _GAMED_GS_TEMPLATE_DATA_TEMPL_BOSS_CHALLENGE_TEMPL_H_
