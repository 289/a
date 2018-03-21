#ifndef GAMED_UTILITY_LIB_BTLIB_NOP_H_
#define GAMED_UTILITY_LIB_BTLIB_NOP_H_

#include "Action.h"


namespace BTLib {

class Nop : public Action
{
public:
	Nop(BehaviorTree* tree, BTNode* parent);
	virtual ~Nop();

    virtual void init();
	virtual NodeStatus stepInto();
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_NOP_H_
