#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>

#include "pugixml/include/pugixml.hpp"

#include "BTLibrary.h"
#include "btlib_link.h"


using namespace BTLib;
using namespace pugi;
using namespace std;

struct behaviortree_predicate
{
	bool operator()(xml_node node) const
	{
		return strcmp(node.name(), "BehaviorTree") == 0;
	}
};

class Param : public BTParam
{
public:
	int kk;
	int yy;
};

class Param2 : public BTParam
{
public:
	std::string kk;
};


int main()
{
	BTLibraryLink liblink;
	liblink.Init();

	LoadBehaviorTree loader;
	loader.Init(BTLibraryLink::GetLeafFactory());

	BehaviorTree*    ptree;
	/*
	bool ret = loader.LoadFile("bt_test.xml", &ptree);
	if (ret)
	{
		printf("successful!\n");
	}
	*/

	xml_document doc;
	xml_parse_result result = doc.load_file("101.xml");
	if (!result) 
	{
		printf("xml load_file error!\n");
		return -1;
	}

	/*
	xml_node element = doc.find_node(behaviortree_predicate());
	while (element)
	{
		xml_text text = element.text();
		string str    = element.text().as_string();
		ostringstream os;
		element.print(os);
		std::string str11 = os.str();
		//printf("%s\n", str11.c_str());

		ptree = loader.LoadBuffer(str11.c_str(), str11.length());
		if (ptree)
		{
			ptree->printTree();
			delete ptree;
		}

		element = element.next_sibling();
		element = element.find_node(behaviortree_predicate());
	}
	*/

	xml_node script = doc.child("Script");
	for (xml_node event = script.child("Event"); event; event = event.next_sibling("Event"))
	{
		xml_node element = event.child("BehaviorTree");
		ostringstream os;
		element.print(os);
		std::string str11 = os.str();
		printf("%s", str11.c_str());

		ptree = loader.LoadBuffer(str11.c_str(), str11.length());
		if (ptree)
		{
			ptree->printTree();
			printf("\n");
			delete ptree;
		}
	}

	Param yc;
	yc.kk = 1;
	yc.yy = 123;
	ptree->setBlackboard(yc);

	Param2 yc2;
	if (ptree->getBlackboard(yc2))
	{
		printf("%s, %d \n", yc2.kk.c_str(), 11);//yc.yy);
	}

	ptree->resetBlackboard();

	return 0;
}
