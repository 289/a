#ifndef GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_
#define GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_

#include "BTLib/BTLibrary.h"


class BTLibraryLink : public BTLib::BehaviorTreeLibrary
{
public:
	static BTLib::BTLeafFactory* GetLeafFactory();
	bool   Init();

protected:
	// This function registers all the user's leaf nodes.
	void   registerLeafNodes(BTLib::BTLeafFactory* factory);

private:
	static BTLib::BTLeafFactory leaf_factory_;
};

#endif // GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_
