#ifndef TASK_MONSTER_PACKAGE_TEMPL_H_
#define TASK_MONSTER_PACKAGE_TEMPL_H_

#include "base_task_templ.h"

namespace task
{

class MonsterPackageTempl : public BaseTempl
{
	DECLARE_SYS_TEMPL(MonsterPackageTempl, TEMPL_TYPE_MONSTER_PACKAGE);
public:
	virtual void OnMarshal();
	virtual void OnUnmarshal();
	virtual bool OnCheckDataValidity() const;
public:
	std::string name;
	// 包含的怪物列表
	std::set<int32_t> monster_list;
};

} // namespace task

#endif // TASK_MONSTER_PACKAGE_TEMPL_H_
