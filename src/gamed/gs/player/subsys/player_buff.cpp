#include "player_buff.h"

#include "gs/global/dbgprt.h"
#include "gs/global/glogger.h"
#include "gs/player/subsys_if.h"
#include "gs/player/player_sender.h"


namespace gamed {

namespace {

    enum MutexType
    {
        MT_NONE = 0, // do nothing
        MT_DEL_NEW,  // 删除新进的filter
        MT_DEL_OLD,  // 删除已存在的filter
    };

    MutexType check_buff_mutex(const Filter* old, const Filter* obj)
    {
        // 互斥组id为-1时表示都不互斥
        if (old->GetMutexGroup() == obj->GetMutexGroup() &&
            old->GetMutexGroup() != -1)
        {
            if (old->GetPriority() <= obj->GetPriority()) 
            {
                return MT_DEL_OLD;
            }
            else 
            {
                return MT_DEL_NEW;
            }
        }

        return MT_NONE;
    }

} // Anonymous

PlayerBuff::PlayerBuff(Player& player)
    : PlayerSubSystem(SUB_SYS_TYPE_BUFF, player),
      update_flag_(false)
{
	SAVE_LOAD_REGISTER(common::PlayerBuffData, PlayerBuff::SaveToDB, PlayerBuff::LoadFromDB);
}

PlayerBuff::~PlayerBuff()
{
    for (size_t i = 0; i < list_wait_.size(); ++i)
    {
        DELETE_SET_NULL(list_wait_[i].second.pfilter);
    }
    list_wait_.clear();

    FilterMultimap::iterator it = filter_map_.begin();
    for (; it != filter_map_.end(); ++ it)
    {
        DELETE_SET_NULL(it->second);
    }
    filter_map_.clear();

    ASSERT(retain_filter_.size() == 0);
    update_flag_ = false;
}

bool PlayerBuff::LoadFromDB(const common::PlayerBuffData& data)
{
	for (size_t i = 0; i < data.buff_list.size(); ++i)
	{
		int32_t filter_type = data.buff_list[i].filter_type;
		Filter* filter      = s_pfilterMan->CreateFilter(filter_type);
		if (!filter)
		{
            GLog::log("PlayerBuff::LoadFromDB - filter类型没有找到type:%d", filter_type);
			continue;
		}

		if (filter->Load(data.buff_list[i]))
        {
		    filter->SetPlayer(&player_);
		    RawAddFilter(filter);
        }
        else
        {
            GLog::log("PlayerBuff::LoadFromDB - filter Load解析数据失败type:%d", filter_type);
            DELETE_SET_NULL(filter);
            continue;
        }
	}

	return true;
}

bool PlayerBuff::SaveToDB(common::PlayerBuffData* pData)
{
    // 先执行change队列
    DoChange();

    std::vector<Filter*> tmplist;
	FilterMultimap::iterator it = filter_map_.begin();
	for (; it != filter_map_.end(); ++ it)
	{
		Filter* filter = it->second;
		if (!filter->IsNeedSave())
            continue;

        common::FilterInfo info;
		if (!filter->Save(&info))
        {
            GLog::log("玩家%ld Filter存盘失败filtertype:%d effectid:%d", player_.role_id(), 
                    filter->GetType(), filter->GetEffectID());
            return false; // 这里返回false，玩家数据就无法存盘了
        }

        pData->buff_list.push_back(info);
    }
    return true;
}

void PlayerBuff::OnHeartbeat(time_t cur_time)
{
    // 执行change队列
    DoChange();

    // 需要在DoChange之后调用
    HandleInactiveFilter();

    // 执行心跳
    FilterMultimap::iterator it = filter_map_.begin();
    for (; it != filter_map_.end(); ++it)
    {
        it->second->Heartbeat();
    }

    //测试buff光效是否需要更新
    if (update_flag_)
	{
		std::vector<PlayerVisibleState::BuffInfo> buff_vec;
		CollectBuffData(buff_vec);
        player_.visible_state().SetBuffList(buff_vec);
		player_.sender()->UpdateVisibleBuff(buff_vec);

		//清除更新标记
        update_flag_ = false;
	}
}

void PlayerBuff::HandleInactiveFilter()
{
    RetainStart();
    FilterMultimap::iterator it = filter_map_.begin();
    for (; it != filter_map_.end(); ++it)
    {
        if (!it->second->IsActived())
        {
            // 不能直接DoDelete，因为会破坏迭代器
            retain_filter_.push_back(it->second);
        }
    }
    RetainEnd();
}

void PlayerBuff::RetainStart()
{
    ASSERT(retain_filter_.size() == 0);
}

void PlayerBuff::RetainEnd()
{
    for (size_t i = 0; i < retain_filter_.size(); ++i)
    {
        DoDelete(retain_filter_[i]);
    }
    retain_filter_.clear();
}

void PlayerBuff::RegisterMsgHandler()
{
	REGISTER_MSG_HANDLER(GS_MSG_ADD_FILTER, PlayerBuff::MSGHandler_AddFilter);
	REGISTER_MSG_HANDLER(GS_MSG_DEL_FILTER, PlayerBuff::MSGHandler_DelFilter);
}

int PlayerBuff::MSGHandler_AddFilter(const MSG& msg)
{
    DoChange();
    return 0;
}

int PlayerBuff::MSGHandler_DelFilter(const MSG& msg)
{
    DoChange();
    return 0;
}

void PlayerBuff::AddFilter(Filter* obj)
{
    WaitingPair entry(FOT_ADD, OpParam(obj, obj->GetType()));
    list_wait_.push_back(entry);
}

void PlayerBuff::DelFilter(Filter* obj)
{
    obj->SetActived(false);
    WaitingPair entry(FOT_DEL, OpParam(obj, obj->GetType()));
    list_wait_.push_back(entry);
}

void PlayerBuff::DelFilter(int32_t type)
{
    ASSERT(type > FILTER_TYPE_INVALID && type < FILTER_TYPE_MAX);
    WaitingPair entry(FOT_DEL_BY_TYPE, OpParam(NULL, type));
    list_wait_.push_back(entry);
}

void PlayerBuff::DelFilterByID(int32_t effectid)
{
    if (effectid <= 0)
    {
        __PRINTF("Error: PlayerBuff::DelFilterByID effectid=%d", effectid);
        return;
    }
    WaitingPair entry(FOT_DEL_BY_EID, OpParam(NULL, effectid));
    list_wait_.push_back(entry);
}

void PlayerBuff::DelSpecFilter(int32_t filter_mask)
{
    ASSERT(filter_mask != 0);
    WaitingPair entry(FOT_DEL_BY_MASK, OpParam(NULL, filter_mask));
    list_wait_.push_back(entry);
}

bool PlayerBuff::IsFilterExistByID(int32_t effectid)
{
    FilterMultimap::const_iterator it = filter_map_.begin();
    for (; it != filter_map_.end(); ++it)
    {
        const Filter* pfilter = it->second;
        if (pfilter->IsActived() && pfilter->GetEffectID() == effectid)
        {
            return true;
        }
    }
    return false;
}

void PlayerBuff::DoChange()
{
    for (size_t i = 0; i < list_wait_.size(); ++i)
    {
        Filter* paramFilter = list_wait_[i].second.pfilter;
        if (paramFilter != NULL && !paramFilter->IsInited())
        {
            DoDelete(paramFilter);
            continue;
        }

        switch (list_wait_[i].first)
        {
            case FOT_ADD:
                {
                    DoInsert(paramFilter);
                }
                break;

            case FOT_DEL:
                {
                    DoDelete(paramFilter);
                }
                break;

            case FOT_DEL_BY_TYPE:
                {
                    RetainStart();
                    int32_t type = list_wait_[i].second.param;
                    FilterMultimapPair ret = filter_map_.equal_range(type);
                    for (FilterMultimap::iterator it = ret.first; it != ret.second; ++it)
                    {
                        if (it->second->IsActived())
                        {
                            it->second->SetActived(false);
                            // 不能直接DoDelete，因为会破坏迭代器
                            retain_filter_.push_back(it->second);
                        }
                    }
                    RetainEnd();
                }
                break;

            case FOT_DEL_BY_MASK:
                {
                    RetainStart();
                    int32_t mask = list_wait_[i].second.param;
                    FilterMultimap::iterator it = filter_map_.begin();
                    for (; it != filter_map_.end(); ++it)
                    {
                        if ( (it->second->GetMask() & mask) &&
                             (it->second->IsActived()) )
                        {
                            it->second->SetActived(false);
                            // 不能直接DoDelete，因为会破坏迭代器
                            retain_filter_.push_back(it->second);
                        }
                    }
                    RetainEnd();
                }
                break;

            case FOT_DEL_BY_EID:
                {
                    RetainStart();
                    int32_t effectid = list_wait_[i].second.param;
                    FilterMultimap::iterator it = filter_map_.begin();
                    for (; it != filter_map_.end(); ++it)
                    {
                        if ( (it->second->GetEffectID() == effectid) &&
                             (it->second->IsActived()) )
                        {
                            it->second->SetActived(false);
                            // 不能直接DoDelete，因为会破坏迭代器
                            retain_filter_.push_back(it->second);
                        }
                    }
                    RetainEnd();
                }
                break;

            default:
                ASSERT(false);
                break;
        }
    }
    list_wait_.clear();
}

void PlayerBuff::DoInsert(Filter* obj)
{
    int32_t type = obj->GetType();
	int32_t mask = obj->GetMask();

    FilterMultimapPair ret = filter_map_.equal_range(type);
    // 存在相同ID的Filter
    if (mask & FILTER_MASK_UNIQUE)
    {
        //Filter是排他的,删除已存在的
        for (FilterMultimap::iterator it = ret.first; it != ret.second; ++it)
        {
            DelFilter(it->second);
        }
    }
    else if (mask & FILTER_MASK_PRIORITY_UNIQUE)
    {
        // 有同类型的buff时
        if (ret.first != ret.second)
        {
            // 根据优先级唯一：相同互斥组里高优先级顶替低优先级，相同优先级的后来顶替先前
            for (FilterMultimap::iterator it = ret.first; it != ret.second; ++it)
            {
                Filter* old = it->second;
                if (old->IsActived())
                {
                    MutexType type = check_buff_mutex(old, obj);
                    if (type == MT_DEL_OLD)
                    {
                        DelFilter(it->second);
                    }
                    else if (type == MT_DEL_NEW)
                    {
                        DELETE_SET_NULL(obj);
                        return;
                    }
                }
            }
        }
    }
    else if (mask & FILTER_MASK_WEAK)
    {
        //Filter是弱的,即有相同的Filter存在的情况下不再次加入
        //但是需要检查是否是active的,如果有则不再加入,否则需要加入
        for (FilterMultimap::iterator it = ret.first; it != ret.second; ++it)
        {
            Filter* old = it->second;
            if (old->IsActived())
            {
                DELETE_SET_NULL(obj);
                return;
            }
        }
    }

    RawAddFilter(obj);
}

void PlayerBuff::DoDelete(Filter* obj)
{
    RawDelFilter(obj);
}

void PlayerBuff::RawAddFilter(Filter* obj)
{
    int32_t type = obj->GetType();
	filter_map_.insert(std::pair<int, Filter*>(type, obj));
	obj->Attach();
    AlterVisibleState();
}

void PlayerBuff::RawDelFilter(Filter* obj)
{
    bool found = false;
    ASSERT(!obj->IsActived());
    int32_t type = obj->GetType();
    FilterMultimapPair ret = filter_map_.equal_range(type);
    //ASSERT(ret.first != filter_map_.end());
    for (FilterMultimap::iterator it = ret.first; it != ret.second; ++it)
    {
        if (it->second == obj)
        {
            obj->Detach();
            filter_map_.erase(it);
            DELETE_SET_NULL(obj);
            AlterVisibleState();
            found = true;
            break;
        }
    }

    if (!found)
    {
        DELETE_SET_NULL(obj);
    }
}

void PlayerBuff::AlterVisibleState()
{
    update_flag_ = true;
}

void PlayerBuff::CollectBuffData(std::vector<PlayerVisibleState::BuffInfo>& buff_vec) const
{
    buff_vec.clear();
    FilterMultimap::const_iterator it = filter_map_.begin();
    for (; it != filter_map_.end(); ++it)
    {
        const Filter* pfilter = it->second;
        if (!pfilter->IsActived())
            continue;

        PlayerVisibleState::BuffInfo info;
        int32_t effect_id = pfilter->GetEffectID();
        info.timeout      = pfilter->GetTimeout();
        if (effect_id == 0) // SYS_BUFF
        {
            info.type = PlayerVisibleState::BT_SYS_BUFF;
            if (!GetSysBuffID(pfilter->GetType(), info.id))
                continue;
        }
        else // WORLD_BUFF
        {
            info.type = PlayerVisibleState::BT_WORLD_BUFF;
            info.id   = effect_id;
        }
        buff_vec.push_back(info);
    }
}

bool PlayerBuff::GetSysBuffID(int32_t buff_type, int32_t& id) const
{
    switch (buff_type)
    {
        case FILTER_TYPE_INVINCIBLE:
            id = PlayerVisibleState::SBT_INVINCIBLE;
            break;

        default:
            LOG_WARN << "this buff not update to client-side? buff_type:" << buff_type;
            return false;
    }
    return true;
}

} // namespace gamed
