#include "ConditionSuccessful.h"


namespace BTLib {

ConditionSuccessful::ConditionSuccessful(BehaviorTree* tree, BTNode* parent)
	: Condition(tree, parent)
{
}

ConditionSuccessful::~ConditionSuccessful()
{
}

void ConditionSuccessful::init() 
{ 
}

NodeStatus ConditionSuccessful::stepInto() 
{ 
	// Do nothing
	return SUCCESS; 
}

} // namespace BTLib

