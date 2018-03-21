#ifndef GAMED_UTILITY_LIB_BTLIB_BTSELECNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTSELECNODE_H_

#include "BTNode.h"


namespace BTLib {

/**
 * BTSelecNode class.
 * This is the Selector Node class. Executes its children from left to right, 
 * bailing out if one of them suceeds.
 */
class BTSelecNode : public BTNode
{
public:
	BTSelecNode(BehaviorTree* tree, BTNode* parent);
	virtual ~BTSelecNode();

	virtual void init();
	virtual void execute();
	virtual void update(NodeStatus result);
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTSELECNODE_H_
