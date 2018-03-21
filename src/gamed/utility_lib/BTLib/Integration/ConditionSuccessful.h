#ifndef GAMED_UTILITY_LIB_BTLIB_CONDITIONSUCCESSFUL_H_
#define GAMED_UTILITY_LIB_BTLIB_CONDITIONSUCCESSFUL_H_

#include "Condition.h"


namespace BTLib {

class ConditionSuccessful : public Condition
{
public:
	ConditionSuccessful(BehaviorTree* tree, BTNode* parent);
	virtual ~ConditionSuccessful();

    virtual void init();
	virtual NodeStatus stepInto();
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_CONDITIONSUCCESSFUL_H_
