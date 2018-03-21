#ifndef GAMED_UTILITY_LIB_BTLIB_LEAFFACTORY_H_
#define GAMED_UTILITY_LIB_BTLIB_LEAFFACTORY_H_

#include <map>
#include <string>

#include "BehaviorTree/ClassLeafNode.h"


namespace BTLib {

/**
 * BTLeafFactory class. This class represents an associative array where
 * leaf classes pointers (ClassLeafNode) are associated with unique strings
 * Can return a pointer, taking the string as argument.
 */
class BTLeafFactory
{
public:
	// Callback for class creations
	typedef ClassLeafNode* (*CreateLeafCallback)(BehaviorTree* tree, BTNode* parent);

	BTLeafFactory();
	~BTLeafFactory();
	
	// **** thread unsafe ****
	void    Destroy();
	void    UnRegisterAll();

	// **** thread unsafe ****
	void    Register(const std::string& name, CreateLeafCallback creator);
	void    UnRegister(std::string& name);
	bool    IsRegistered(std::string& name) const;

	// **** thread unsafe ****
	CreateLeafCallback GetCreator(std::string& name) const;


private:
	void    registerDefaultLeafNodes();


private:
	typedef std::map<std::string, CreateLeafCallback> LeafCBMap;
	LeafCBMap    entries_;
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_LEAFFACTORY_H_
