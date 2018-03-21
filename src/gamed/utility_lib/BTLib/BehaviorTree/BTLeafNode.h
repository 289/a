#ifndef GAMED_UTILITY_LIB_BTLIB_BTLEAFNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTLEAFNODE_H_

#include "BTNode.h"


namespace BTLib {

/**
 * The BTLeafNode class.
 * Node for Leaf nodes. This is a virtual class, must be overridden.
 */
class BTLeafNode : public BTNode
{
public:
	BTLeafNode(BehaviorTree* tree, BTNode* parent)
		: BTNode(tree, parent)
	{ }

	virtual ~BTLeafNode() { }

	virtual void execute() { }

	/**
	 * This method must be overridden by the leaf node classes, so the actual behavior of the 
	 * action or condition is triggered.
	 * @return The result of the execution.
	 */
	virtual NodeStatus step() { return START; }
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTLEAFNODE_H_
