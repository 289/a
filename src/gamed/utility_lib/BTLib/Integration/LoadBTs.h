#ifndef GAMED_UTILITY_LIB_BTLIB_LOADBTS_H_
#define GAMED_UTILITY_LIB_BTLIB_LOADBTS_H_

#include <string>


namespace pugi
{
	class xml_node;
	class xml_document;
}

namespace BTLib {

class BTNode;
class BehaviorTree;
class BTLeafFactory;

class LoadBehaviorTree
{
public:
	LoadBehaviorTree();
	~LoadBehaviorTree();

	bool Init(const BTLeafFactory* factory);
	BehaviorTree* LoadFile(const std::string& fromXMLFile);
	BehaviorTree* LoadBuffer(const void* contents, size_t size);


private:
	BehaviorTree* Load(pugi::xml_document& doc);

	void loadRecur(pugi::xml_node& a_element, 
	  		       BehaviorTree* tree, 
				   BTNode* parentElement, 
				   int deep);

	void insertTreeNode(BehaviorTree* a_tree, 
			            BTNode* a_parentElement, 
						int a_deep, 
						BTNode* a_new_node,
						int a_xml_nodeid);

private:
	bool  isInited_;
	const BTLeafFactory* leafFactory_;
};

} // namespace BTLib

#endif // GAMED_UTILITY_LIB_BTLIB_LOADBTS_H_
