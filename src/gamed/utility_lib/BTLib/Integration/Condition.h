#ifndef GAMED_UTILITY_LIB_BTLIB_CONDITION_H_
#define GAMED_UTILITY_LIB_BTLIB_CONDITION_H_

#include "BehaviorTree/ClassLeafNode.h"

namespace BTLib {

class Condition : public ClassLeafNode
{
public:
	Condition(BehaviorTree* tree, BTNode* parent)
		: ClassLeafNode(BTLEAF_ACTION, tree, parent)
	{ }

	virtual ~Condition() { }

	virtual void init() { }
	virtual NodeStatus stepInto() = 0;

private:
	/**
	 * @brief step 
	 *    （1）XXX：子类不能覆盖step函数
	 */
	virtual NodeStatus step() { return stepInto(); }
};

} // namespace BTLib


#endif // GAMED_UTILITY_LIB_BTLIB_CONDITION_H_
