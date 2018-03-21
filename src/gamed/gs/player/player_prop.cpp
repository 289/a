#include "player_prop.h"
#include "player.h"

#include "gs/global/gmatrix.h"
#include "gs/template/data_templ/player_class_templ.h"


namespace gamed {

SHARED_STATIC_ASSERT(PROP_INDEX_HIGHEST <= PROP_INDEX_MAX);

/*****************************PropertyPolicy*******************************/
/*****************************PropertyPolicy*******************************/
/*****************************PropertyPolicy*******************************/
/*****************************PropertyPolicy*******************************/
void PropertyPolicy::UpdatePlayerProp(Player* player)
{
	//缓存更新前的属性
	ExtendProp old_props[PROP_INDEX_HIGHEST] = {0};
	memcpy(old_props, player->cur_prop_, sizeof(ExtendProp) * PROP_INDEX_HIGHEST);

	//先更新扩展属性
	UpdateExtendProp(player);

	//再更新基础属性
	UpdateBasicProp(player, old_props);
}

void PropertyPolicy::UpdateExtendProp(Player* player)
{
	ExtendProp dest[PROP_INDEX_HIGHEST] = {0};
	memcpy(dest, player->base_prop_, sizeof(player->base_prop_));

	ExtendProp enh_scale[PROP_INDEX_HIGHEST] = {0};
	for (size_t i = 0; i < PROP_INDEX_HIGHEST; ++ i)
	{
		enh_scale[i] += player->enh_scale_[i];
		enh_scale[i] += player->equip_scale_[i];
	}

	for (size_t i = 0; i < PROP_INDEX_HIGHEST; ++ i)
	{
		dest[i] *= 0.0001 * (10000 + enh_scale[i]);
	}

	for (size_t i = 0; i < PROP_INDEX_HIGHEST; ++ i)
	{
        dest[i] += player->enh_point_[i];
		dest[i] += player->equip_point_[i];
	}

	memset(player->cur_prop_, 0, sizeof(player->cur_prop_));
	memcpy(player->cur_prop_, dest, sizeof(dest));
}

void PropertyPolicy::UpdateBasicProp(Player* player, const ExtendProp* old_props)
{
	gmatrixdef::PlayerClassDefaultTempl cls_default = Gmatrix::GetPlayerClsTemplCfg(player->role_class());

	char hp_sync_rule = cls_default.templ_ptr->properties[0].sync_rule;
	char mp_sync_rule = cls_default.templ_ptr->properties[1].sync_rule;
	char ep_sync_rule = cls_default.templ_ptr->properties[2].sync_rule;

	int32_t new_hp = PropRuler::UpdateProperty(hp_sync_rule, player->GetHP(), old_props[PROP_INDEX_MAX_HP], player->GetMaxHP());
	int32_t new_mp = PropRuler::UpdateProperty(mp_sync_rule, player->GetMP(), old_props[PROP_INDEX_MAX_MP], player->GetMaxMP());
	int32_t new_ep = PropRuler::UpdateProperty(ep_sync_rule, player->GetEP(), old_props[PROP_INDEX_MAX_EP], player->GetMaxEP());

	player->SetHP(new_hp);
	player->SetMP(new_mp);
	player->SetEP(new_ep);
}

void PropertyPolicy::CalPlayerCombatProp(Player* player, std::vector<int32_t>& dest)
{
    
	//为战斗中的玩家计算属性
	//战斗玩家属性包括玩家基本属性和装备产生的属性
	//ExtendProp props[PROP_INDEX_MAX] = {0};
	//for (size_t i = 0; i < PROP_INDEX_MAX; ++ i)
	//{
		//props[i] += player->base_prop_[i];
		//props[i] += player->equip_point_[i];
	//}

	//dest.clear();
	//dest.resize(PROP_INDEX_MAX);
	//for (size_t i = 0; i < PROP_INDEX_MAX; ++ i)
	//{
		//dest[i] = props[i] * 0.0001 * (10000 + player->equip_scale_[i]);
	//}

	dest.clear();
	dest.resize(PROP_INDEX_HIGHEST);
    for (size_t i = 0; i < PROP_INDEX_HIGHEST; ++ i)
    {
        dest[i] = player->cur_prop_[i];
    }
}

int32_t PropertyPolicy::GetHPGen(Player* player)
{
	/**
	 * @brief 血量恢复公式
	 * @brief hp_gen = max_hp * ((360/(level+60))/100);  --->>> 原始回血公式
	 * @brief hp_gen = max_hp * (18 / (5*level+300));    --->>> 简化后的公式
	 */
    //int32_t level = player->level();
    //int32_t maxhp = player->GetMaxHP();
    //return maxhp * ((float)(18.0f) / (5*level + 300));
	//为测试临时修改
	//return player->GetMaxHP();


    /**
     * @brief 血量恢复公式
     * @brief hp_gen = max_hp * (6 / (2 * level + 9));
     */
    int32_t level = player->level();
    int32_t maxhp = player->GetMaxHP();
    return maxhp * ((float)(6.0f) / (2 * level + 9));
}

/**************************PropRuler*************************/
/**************************PropRuler*************************/
/**************************PropRuler*************************/
/**************************PropRuler*************************/
int32_t PropRuler::UpdateProperty(char ruler, int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t rst = 0;
	switch (ruler)
	{
#define CASE_UPDATE_PLAYER_PROP(num, curval, old_maxval, new_maxval) \
		case num: \
		{ \
			rst = UpdatePropByRule_##num(curval, old_maxval, new_maxval); \
		} \
		break;

		CASE_UPDATE_PLAYER_PROP(0, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(1, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(2, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(3, curval, old_maxval, new_maxval);
		CASE_UPDATE_PLAYER_PROP(4, curval, old_maxval, new_maxval);
		default:
		{
			ASSERT(false);
		}
		break;
#undef CASE_UPDATE_PLAYER_PROP
	};

	return rst;

}

int32_t PropRuler::UpdatePropByRule_0(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	return new_maxval;
}

int32_t PropRuler::UpdatePropByRule_1(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	int new_prop = curval + offset;

	if (new_prop < 0) new_prop = 0;
	if (new_prop > new_maxval) new_prop = new_maxval;
	return new_prop;
}

int32_t PropRuler::UpdatePropByRule_2(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	int new_prop = curval + offset;

	if (new_prop <= 0) new_prop = 1;
	if (new_prop > new_maxval) new_prop = new_maxval;
	return new_prop;
}

int32_t PropRuler::UpdatePropByRule_3(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	if (curval > new_maxval) return new_maxval;
	return curval;
}

int32_t PropRuler::UpdatePropByRule_4(int32_t curval, int32_t old_maxval, int32_t new_maxval)
{
	int32_t offset = new_maxval - old_maxval;
	if (offset == 0)
	{
		return curval;
	}

	if (offset > 0)
	{
		//最大值变大
		int32_t new_prop = curval + offset;
		if (new_prop > new_maxval) new_prop = new_maxval;
		return new_prop;
	}
	else
	{
		//最大值变小
		if (curval > new_maxval) return new_maxval;
		return curval;
	}
}

} // namespace gamed
