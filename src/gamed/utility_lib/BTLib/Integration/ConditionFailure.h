#ifndef GAMED_UTILITY_LIB_BTLIB_CONDITIONFAILURE_H_
#define GAMED_UTILITY_LIB_BTLIB_CONDITIONFAILURE_H_

#include "Condition.h"


namespace BTLib {

class ConditionFailure : public Condition
{
public:
	ConditionFailure(BehaviorTree* tree, BTNode* parent);
	virtual ~ConditionFailure();

    virtual void init();
	virtual NodeStatus stepInto();
};

} // namespace BTLib


#endif // GAMED_UTILITY_LIB_BTLIB_CONDITIONFAILURE_H_
