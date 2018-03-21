#ifndef GAMED_UTILITY_LIB_BTLIB_BTLINKNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_BTLINKNODE_H_

#include "BTNode.h"


namespace BTLib {

/**
 * BTLinkNode class.
 * This is the Root Node class. It is the top of the tree and it can only have one child.
 */
class BTLinkNode : public BTNode
{
public:
	BTLinkNode(BehaviorTree* tree, BTNode* parent);
	virtual ~BTLinkNode();

	virtual void execute();
	virtual void update(NodeStatus result);

	inline int getNumChildren() const;
};

///
/// inline func
///
inline int BTLinkNode::getNumChildren() const
{
	return children_.size();
}

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_BTLINKNODE_H_
