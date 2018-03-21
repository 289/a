#ifndef ACHIEVE_ACHIEVE_TEMPL_H_
#define ACHIEVE_ACHIEVE_TEMPL_H_

#include "base_achieve_templ.h"
#include "achieve_premise.h"
#include "achieve_award.h"

namespace achieve
{

class AchieveTempl : public BaseTempl
{
	DECLARE_SYS_TEMPL(AchieveTempl, TEMPL_TYPE_ACHIEVE);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;

    inline bool unfinish_show() const;
public:
    // 基本信息
    int16_t flag;
    int32_t point;
    std::string name;
    std::string desc;
    std::string icon;

    // 成就达成条件
    AchievePremise premise;

    // 成就奖励
    AchieveAward award;

	// 层次结构
	AchieveID parent;
	AchieveID next_sibling;
	AchieveID first_child;

	// 以下部分不存盘，任务数据加载到内存后设置
	const AchieveTempl* parent_templ;
	const AchieveTempl* next_sibling_templ;
	const AchieveTempl* first_child_templ;
};

inline bool AchieveTempl::unfinish_show() const
{
    return flag & FLAG_UNFINISH_SHOW;
}

} // namespace achieve

#endif // ACHIEVE_ACHIEVE_TEMPL_H_
