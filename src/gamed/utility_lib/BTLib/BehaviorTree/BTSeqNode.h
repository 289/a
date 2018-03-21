#ifndef GAMED_UTILITY_LIB_BTLIB_BTSEQNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTSEQNODE_H_

#include "BTNode.h"


namespace BTLib {

/**
 * BTSeqNode class.
 * This is the Sequence Node class. Executes its children from left to right, 
 * bailing out if one of them fails.
 */
class BTSeqNode : public BTNode
{
public:
	BTSeqNode(BehaviorTree* tree, BTNode* parent);
	virtual ~BTSeqNode();

	virtual void init();
	virtual void execute();
	virtual void update(NodeStatus result);
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTSEQNODE_H_
