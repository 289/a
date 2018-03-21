#ifndef GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_
#define GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_

#include "utility_lib/BTLib/BTLibrary.h"


namespace gamed {

class AreaEvLibraryLink : public BTLib::BehaviorTreeLibrary
{
public:
	static BTLib::BTLeafFactory* GetLeafFactory();
	static BTLib::LoadBehaviorTree* GetBTLoader();
	bool   Init();

protected:
	// This function registers all the user's leaf nodes.
	void   registerLeafNodes(BTLib::BTLeafFactory* factory);

private:
	static BTLib::BTLeafFactory    leaf_factory_;
	static BTLib::LoadBehaviorTree bt_loader_;
};

} // namespace eventsys

#endif // GAMED_GS_EVENTSYS_AREA_BTLIB_LINK_H_
