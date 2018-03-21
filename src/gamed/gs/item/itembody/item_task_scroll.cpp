#include "item_task_scroll.h"

#include "shared/security/randomgen.h"
#include "game_module/task/include/task.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/task_scroll_templ.h"
#include "gs/template/data_templ/cooldown_group.h"
#include "gs/global/randomgen.h"
#include "gs/player/player.h"
#include "gs/player/player_sender.h"


namespace gamed {

void ItemTaskScroll::Initialize(const dataTempl::ItemDataTempl* tpl)
{
	ASSERT(tpl);
	ptpl_ = dynamic_cast<const dataTempl::TaskScrollTempl*>(tpl);

	consume_on_use_     = ptpl_->consume_on_use;
	consume_item_id_    = ptpl_->consume_item_id;
	consume_item_count_ = ptpl_->consume_item_count;
    cd_group_id_        = ptpl_->cooldown_data.cd_group_id;
    cd_time_            = ptpl_->cooldown_data.cd_time;

    if (cd_group_id_ > 0)
    {
        const CoolDownGroupTempl* pCDG = s_pDataTempl->QueryDataTempl<CoolDownGroupTempl>(cd_group_id_);
        if (pCDG) {
            cd_group_time_ = pCDG->cd_group_time;
        }
        else {
            cd_group_id_ = 0;
            LOG_ERROR << "task_scroll_item: " << ptpl_->templ_id << " cd_group " << cd_group_id_ << " not found!";
        }
    }

    tasks_.resize(ptpl_->task_list.size());
	for (size_t i = 0; i < ptpl_->task_list.size(); ++ i)
	{
		tasks_[i].task_id     = ptpl_->task_list[i].task_id;
		tasks_[i].probability = ptpl_->task_list[i].probability;
        if (i > 0) {
			tasks_[i].probability += tasks_[i-1].probability;
        }
	}
}

bool ItemTaskScroll::TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	Player* pplayer = dynamic_cast<Player*>(obj);
	if (consume_item_id_ > 0 && consume_item_count_ > 0 && !pplayer->CheckItem(consume_item_id_, consume_item_count_))
	{
		return false;
	}
        
    if (!pplayer->TestItemCoolDown(cd_group_id_, parent->Type()))
        return false;

	return true;
}

Item::ITEM_USE ItemTaskScroll::OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const
{
	Player* pplayer = dynamic_cast<Player*>(obj);
    
    // 检查地图限制
    if (ptpl_->specify_map.map_id > 0)
    {
        struct rect rt;
        rt.left   = ptpl_->specify_map.min_coord.x;
        rt.top    = ptpl_->specify_map.min_coord.y;
        rt.right  = ptpl_->specify_map.max_coord.x;
        rt.bottom = ptpl_->specify_map.max_coord.y;
        if (ptpl_->specify_map.map_id == pplayer->world_id())
        {
            if (rt.IsOut(pplayer->pos().x, pplayer->pos().y))
            {
                pplayer->sender()->ErrorMessage(G2C::ERR_OUT_OF_SPECIFY_MAP_AREA);
                return Item::ITEM_USE_FAILURE;
            }
        }
        else
        {
            pplayer->sender()->ErrorMessage(G2C::ERR_OUT_OF_SPECIFY_MAP_AREA);
            return Item::ITEM_USE_FAILURE;
        }
    }
    else
    {
        if (ptpl_->forbid_map_type == dataTempl::TaskScrollTempl::FMT_NONNORMAL)
        {
            if (!IS_NORMAL_MAP(pplayer->world_id()))
            {
                pplayer->sender()->ErrorMessage(G2C::ERR_THIS_MAP_FORBID);
                return Item::ITEM_USE_FAILURE;
            }
        }
    }

    // 先设置冷却，成功与否都要设置冷却
    if (cd_group_id_ > 0)
    {
        pplayer->SetCoolDown(cd_group_id_, cd_group_time_);
    }
    if (cd_time_ > 0)
    {
        pplayer->SetCoolDown(parent->Type(), cd_time_);
    }

	// 发任务
	int32_t task_id = 0;
	int32_t rand_num = mrand::Rand(1, 10000);
	for (size_t i = 0; i < tasks_.size(); ++ i)
	{
		if (tasks_[i].probability < rand_num)
			continue;

		task_id = tasks_[i].task_id;
		break;
	}

	// 随机任务失败
	if (task_id <= 0)
	{
		if (consume_item_id_ > 0 && consume_item_count_ > 0)
		{
			//随机失败也会扣除道具
			pplayer->TakeOutItem(consume_item_id_, consume_item_count_);
		}
		pplayer->sender()->UseItemError(G2C::UIEC_RAND_TASK, parent->Type());
        // 随机失败也要扣除物品
		return Item::ITEM_USE_CONSUME;
	}

	// 能否接受任务
	if (pplayer->CanDeliverTask(task_id) != task::ERR_TASK_SUCC)
	{
		pplayer->sender()->UseItemError(G2C::UIEC_DELIVER_TASK, parent->Type());
		return Item::ITEM_USE_FAILURE;
	}

	// 扣除道具
	if (consume_item_id_ > 0 && consume_item_count_ > 0 && !pplayer->TakeOutItem(consume_item_id_, consume_item_count_))
	{
		assert(false);
		return Item::ITEM_USE_FAILURE;
	}
    
	// 发任务
	if (pplayer->DeliverTask(task_id) != task::ERR_TASK_SUCC)
	{
		assert(false);
		return Item::ITEM_USE_FAILURE;
	}

	return consume_on_use_ ? Item::ITEM_USE_CONSUME : Item::ITEM_USE_RETAIN;
}

};
