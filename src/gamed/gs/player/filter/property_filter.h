#ifndef GAMED_GS_PLAYER_FILTER_PROPERTY_FILTER_H_
#define GAMED_GS_PLAYER_FILTER_PROPERTY_FILTER_H_

#include "game_module/skill/include/world_buff.h"
#include "gs/player/filter.h"
#include "gs/player/player.h"
#include "gs/player/skill_if.h"


namespace gamed {

/**
 * @class PropertyFilter - property filter world buff
 * @brief 修改玩家自身属性
 */
class PropertyFilter : public Filter
{
	enum 
    {
		MASK = FILTER_MASK_BUFF | FILTER_MASK_SAVE_DB_DATA | FILTER_MASK_PRIORITY_UNIQUE,
	};

    DECLARE_FILTER(PropertyFilter, FILTER_TYPE_PROPERTY, MASK);

public:
	PropertyFilter(Player* player, int32_t effectid, int32_t timeout = 0)
        : Filter(player, FILTER_TYPE_PROPERTY, MASK, effectid)
	{
        SetTimeout(timeout);

        if (CheckEffectID(effectid))
        {
            PlayerSkillIf skill_if(pplayer_);
            if (!skill::GetPropBuffInfo(effect_id_, &skill_if, buff_info_))
            {
                SetFilterInit(false);
                return;
            }

            SetTimeout(buff_info_.duration);
        }
	}

    virtual int32_t GetPriority() const   { return buff_info_.prior; }
    virtual int32_t GetMutexGroup() const { return buff_info_.mutex_gid; } // 获取互斥组

protected:
	virtual void OnAttach()
	{
        for (size_t i = 0; i < buff_info_.dmg_vec.size(); ++i)
        {
            int prop_index = buff_info_.dmg_vec[i].prop;
            if (!CheckPropIndex(prop_index))
            {
                GLog::log("PropertyFilter::OnAttach() error! prop index:%d effectid:%d", prop_index, effect_id_);
                continue;
            }
            pplayer_->IncPropPoint(prop_index, buff_info_.dmg_vec[i].value);
        }
	}

	virtual void OnDetach()
	{
        for (size_t i = 0; i < buff_info_.dmg_vec.size(); ++i)
        {
            int prop_index = buff_info_.dmg_vec[i].prop;
            if (!CheckPropIndex(prop_index))
            {
                GLog::log("PropertyFilter::OnDetach() error! prop index:%d effectid:%d", prop_index, effect_id_);
                continue;
            }
            pplayer_->DecPropPoint(prop_index, buff_info_.dmg_vec[i].value);
        }
	}

    // 存盘数据注意分版本，不要轻易修改
    virtual bool OnLoad(const std::string& detail) 
    { 
        if (CheckEffectID(effect_id_))
        {
            PlayerSkillIf skill_if(pplayer_);
            if (!skill::GetPropBuffInfo(effect_id_, &skill_if, buff_info_))
                return false;
        }
        else
        {
            if (timeout_ <= 0)
                return false;
        }

        return true; 
    }

    // 存盘数据注意分版本，不要轻易修改
	virtual bool OnSave(std::string* pdetail)
    { 
        return true; 
    }


private:
    bool CheckPropIndex(int index)
    {
        if (index >= 0 && index < PROP_INDEX_HIGHEST)
            return true;

        return false;
    }

private:
    skill::PropBuffInfo buff_info_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_FILTER_PROPERTY_FILTER_WB_H_
