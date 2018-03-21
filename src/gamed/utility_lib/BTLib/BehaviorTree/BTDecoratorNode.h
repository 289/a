#ifndef GAMED_UTILITY_LIB_BTLIB_BTDECORATORNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTDECORATORNODE_H_

#include "BTNode.h"


namespace BTLib {

/**
 * The BTDecoratorNode class.
 * Node for Filter nodes. This is a virtual class, must be overridden to create new filters.
 */
class BTDecoratorNode : public BTNode
{
public:
	BTDecoratorNode(BehaviorTree* tree, BTNode* parent);
	virtual ~BTDecoratorNode() { }

	// Function that codifies the behavior of this filter/decorator.
	virtual NodeStatus decorator() { return SUCCESS; }

	virtual void execute() { }
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTDECORATORNODE_H_
