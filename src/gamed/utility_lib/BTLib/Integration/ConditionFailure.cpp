#include "ConditionFailure.h"


namespace BTLib {

ConditionFailure::ConditionFailure(BehaviorTree* tree, BTNode* parent)
	: Condition(tree, parent)
{
}

ConditionFailure::~ConditionFailure()
{
}

void ConditionFailure::init() 
{ 
}

NodeStatus ConditionFailure::stepInto() 
{ 
	// Do nothing
	return FAILURE; 
}

} // namespace BTLib

