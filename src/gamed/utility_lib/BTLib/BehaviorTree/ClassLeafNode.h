#ifndef GAMED_UTILITY_LIB_BTLIB_CLASSLEAFNODE_H_
#define GAMED_UTILITY_LIB_BTLIB_CLASSLEAFNODE_H_

#include <string>

#include "BehaviorTree/BTLeafNode.h"


namespace BTLib {

enum BTLeafType
{
	BTLEAF_UNDEF     = 0,
	BTLEAF_CONDITION = 1,
	BTLEAF_ACTION    = 2
};

class ClassLeafNode;

/* Callback for leaf node classes creation. */
template<typename T>
ClassLeafNode* createLeafNode(BehaviorTree* tree, BTNode* parent)
{
	return new T(tree, parent);
}

/**
 * The core ClassLeafNode class.
 * This is a virtual class, and must be used as the base class for leaf node classes.
 * Actions and condition classes must inherit from this class, overriding the method step()
 * to perform the actual behavior of the node.
 */
class ClassLeafNode : public BTLeafNode
{
public:
	ClassLeafNode(BTLeafType type, BehaviorTree* tree, BTNode* parent);
	virtual ~ClassLeafNode();

	/**
	 * @brief execute 
	 *    （1）XXX 注意：子类不能实现这个虚函数
	 */
	virtual void execute();

	/**
	 * This method must be overridden by the leaf node classes, so the actual behavior of the 
	 * action or condition is triggered.
	 * @return The result of the execution.
	 */
	virtual NodeStatus step() = 0;

	/**
	 * Sets the leaf type (action / condition)
	 * @param a_leafType Type of the leaf node.
	 */
	inline void setLeafType(BTLeafType leafType) { leafType_ = leafType; }

	/**
	 * Sets the execute script (lua script)
	 * @param scriptName lua script name.
	 */
	inline void setScriptName(const std::string& script_name) { scriptName_ = script_name; }
	inline void setScriptFunc(const std::string& script_func) { scriptFunc_ = script_func; }


protected:
	// RUNTIME DATA //
	BTLeafType    leafType_;
	std::string   scriptName_;
	std::string   scriptFunc_;
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_CLASSLEAFNODE_H_
