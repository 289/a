#include "filter.h"

#include "shared/net/packet/bytebuffer.h"
#include "common/obj_data/player_attr_declar.h"
#include "gs/global/game_util.h"
#include "gs/global/glogger.h"
#include "gs/player/filter/invincible_filter.h"
#include "gs/player/filter/immune_landmine_filter.h"
#include "gs/player/filter/transform_filter.h"
#include "gs/player/filter/property_filter.h"


namespace gamed {

///
/// Filter
///
Filter::Filter(int32_t type)
    : is_inited_(false),
      filter_type_(type),
      timeout_(-1),
      effect_id_(0),
      filter_mask_(0),
      is_actived_(false),
      is_reject_save_(false),
      pplayer_(NULL)
{
    SetFilterInit(true);
}

Filter::Filter(Player* player, int32_t type, int32_t mask, int32_t effectid)
    : is_inited_(false),
      filter_type_(type),
      timeout_(-1),
      effect_id_(effectid),
	  filter_mask_(mask),
	  is_actived_(false),
      is_reject_save_(false),
	  pplayer_(player)
{
    SetFilterInit(true);
}

Filter::~Filter()
{
    is_inited_      = false;
    filter_type_    = -1;
    timeout_        = -1;
    effect_id_      = 0;
    filter_mask_    = 0;
    is_actived_     = false;
    is_reject_save_ = false;
    pplayer_        = NULL;
}

bool Filter::Load(const common::FilterInfo& info)
{
    filter_type_ = info.filter_type;
    timeout_     = info.timeout;
    effect_id_   = info.effect_id;
    
    if (!OnLoad(info.detail))
    {
        GLog::log("ERR - Filter::Load() failure type:%d", filter_type_);
        return false;
    }
    return true;
}

bool Filter::Save(common::FilterInfo* pinfo)
{
    if (!IsNeedSave())
        return false;
    
    pinfo->filter_type = filter_type_;
    pinfo->timeout     = timeout_;
    pinfo->effect_id   = effect_id_;

    if (!OnSave(&pinfo->detail))
    {
        GLog::log("ERR - Filter::Save() failure type:%d", filter_type_);
        return false;
    }
    return true;
}

void Filter::Attach()
{
    if (!is_inited_)
        return;

    // Attach肯定是第一次active
    ASSERT(!is_actived_);
    OnAttach();
    SetActived(true);
}

void Filter::Detach()
{
    if (!is_inited_)
        return;

    // 已经被管理类设成false
    ASSERT(!is_actived_);
    OnDetach();
}

void Filter::Heartbeat()
{
    if (is_actived_)
    {
        if (HasTimeout() && --timeout_ <= 0)
        {
            SetActived(false); // 在下个心跳删除
        }
        else
        {
            OnHeartbeat();
        }
    }
}

bool Filter::IsNeedSave() const 
{ 
    if ( IsActived() &&
         !is_reject_save_ &&
         (filter_mask_ & FILTER_MASK_SAVE_DB_DATA) )
    {
        return true;
    }

    return false; 
}


///
/// FilterMan
///
#define REGISTER_FILTER(class_name) \
        { \
            class_name* obj = new class_name(class_name::TypeNumber()); \
		    ASSERT(prototype_map_.insert(std::pair<int32_t, Filter*>(class_name::TypeNumber(), obj)).second) \
        }

FilterMan::FilterMan() 
{
    REGISTER_FILTER(InvincibleFilter);
	REGISTER_FILTER(ImmuneLandmineFilter);
    REGISTER_FILTER(TransformFilter);
    REGISTER_FILTER(PropertyFilter);
}

FilterMan::~FilterMan()
{
    FilterMap::iterator it = prototype_map_.begin();
    for (; it != prototype_map_.end(); ++ it)
    {
        DELETE_SET_NULL(it->second);
    }
    prototype_map_.clear();
}

Filter* FilterMan::CreateFilter(int32_t filter_type)
{
	FilterMap::iterator it = prototype_map_.find(filter_type);
	if (it == prototype_map_.end())
    {
        ASSERT(false);
		return NULL;
    }
	return it->second->Clone();
}

} // namespace gamed
