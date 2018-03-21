#ifndef ACHIEVE_ACHIEVE_WRAPPER_H_
#define ACHIEVE_ACHIEVE_WRAPPER_H_

#include "achieve_types.h"

namespace achieve
{

class AchieveTempl;

class AchieveWrapper
{
public:
    AchieveWrapper(Player* player, const AchieveTempl* templ);

    // 通用函数
    int32_t CheckFinish() const;
    int32_t CanDeliverAward() const;
    void AchieveFinish();

    // 仅服务器使用
    void DeliverAward();

    // 成就系统内部使用
    int32_t CheckSelfCond(std::vector<bool>* cond_vec = NULL) const;
    int32_t CheckChildFinish(std::vector<bool>* cond_vec = NULL) const;
private:
    Player* player_;
    const AchieveTempl* templ_;
};

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_WRAPPER_H_
