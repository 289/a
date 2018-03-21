#include "achieve_wrapper.h"
#include "achieve_templ.h"
#include "achieve_data.h"
#include "achieve_interface.h"
#include "achieve_expr.h"

namespace achieve
{

using namespace std;

AchieveWrapper::AchieveWrapper(Player* player, const AchieveTempl* templ)
    : player_(player), templ_(templ)
{
}

int32_t AchieveWrapper::CheckFinish() const
{
    // 检查前置成就是否达成
    const AchievePremise& premise = templ_->premise;
    FinishAchieve* finish = player_->GetFinishAchieve();
    if (premise.premise != 0 && finish->GetEntry(premise.premise) == NULL)
    {
        return ERR_ACHIEVE_ACHIEVE;
    }
    // 检查成就条件是否满足
    int32_t count = templ_->first_child == 0 ? CheckSelfCond() : CheckChildFinish();
    return count >= premise.needed_cond ? ERR_ACHIEVE_SUCC : ERR_ACHIEVE_COND;
}

int32_t AchieveWrapper::CheckSelfCond(vector<bool>* cond_vec) const
{
    int32_t finish_cond = 0;
    AchieveCondVec::const_iterator cit = templ_->premise.cond.begin();
    for (; cit != templ_->premise.cond.end(); ++cit)
    {
        bool cond_finish = true;
        AchieveExpr expr(cit->data, player_);
        int32_t result = expr.Calculate();
        if (cit->op == ACHIEVE_OP_GREATER_EQUAL && result >= cit->value)
        {                
            ++finish_cond;
        }
        else if (cit->op == ACHIEVE_OP_LESS_EQUAL && result <= cit->value)
        {
            ++finish_cond;
        }
        else
        {
            cond_finish = false;
        }
        if (cond_vec != NULL)
        {
            cond_vec->push_back(cond_finish);
        }
    }
    return finish_cond;
}

int32_t AchieveWrapper::CheckChildFinish(vector<bool>* cond_vec) const
{
    int32_t finish_cond = 0;
    FinishAchieve* finish = player_->GetFinishAchieve();
    const AchieveTempl* tmp = templ_->first_child_templ;
    while (tmp)
    {
        bool cond_finish = true;
        if (finish->GetEntry(tmp->id) != NULL)
        {
            ++finish_cond;
        }
        else
        {
            cond_finish = false;
        }
        if (cond_vec != NULL)
        {
            cond_vec->push_back(cond_finish);
        }
        tmp = tmp->next_sibling_templ;
    }
    return finish_cond;
}

int32_t AchieveWrapper::CanDeliverAward() const
{
    const ItemInfo& item = templ_->award.item;
    return player_->CanDeliverItem(item.type, 1) ? ERR_ACHIEVE_SUCC : ERR_ACHIEVE_ITEM;
}

void AchieveWrapper::AchieveFinish()
{
    FinishAchieve* finish = player_->GetFinishAchieve();
    AchieveEntry& entry = finish->achieve_list[templ_->id];
    entry.templ = templ_;
    ActiveAchieve* active = player_->GetActiveAchieve();
    if (active != NULL)
    {
        active->achieve_list.erase(templ_->id);
    }
    player_->DeliverAchievePoint(templ_->point);
}

void AchieveWrapper::DeliverAward()
{
    FinishAchieve* finish = player_->GetFinishAchieve();
    AchieveEntry& entry = finish->achieve_list[templ_->id];
    entry.SetAward();

    const ItemInfo& item = templ_->award.item;
    if (item.id != 0 && item.count != 0)
    {
        player_->DeliverItem(item.id, item.count);
    }
    player_->DeliverTitle(templ_->award.title_id);
}

} // namespace achieve
