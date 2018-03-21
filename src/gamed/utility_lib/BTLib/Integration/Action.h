#ifndef GAMED_UTILITY_LIB_BTLIB_ACTION_H_
#define GAMED_UTILITY_LIB_BTLIB_ACTION_H_

#include "BehaviorTree/ClassLeafNode.h"


namespace BTLib {

class Action : public ClassLeafNode
{
public:
	Action(BehaviorTree* tree, BTNode* parent)
		: ClassLeafNode(BTLEAF_ACTION, tree, parent)
	{ }

	virtual ~Action() { }

	virtual void init() { }
	virtual NodeStatus stepInto() = 0;

private:
	/**
	 * @brief step 
	 *    （1）XXX：子类不能覆盖step函数
	 */
	virtual NodeStatus step() 
	{
		stepInto();
		return SUCCESS;
	}
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_ACTION_H_
