#include "Nop.h"


namespace BTLib {

Nop::Nop(BehaviorTree* tree, BTNode* parent)
	: Action(tree, parent)
{
}

Nop::~Nop()
{
}

void Nop::init() 
{ 
}

NodeStatus Nop::stepInto() 
{ 
	// Do nothing
	return SUCCESS; 
}

} // namespace BTLib
