#ifndef GAMED_GS_EVENTSYS_SRC_AREA_EV_IF_H_
#define GAMED_GS_EVENTSYS_SRC_AREA_EV_IF_H_

#include <vector>

#include "shared/base/types.h"

#include "script_xml_op.h"


namespace BTLib
{
	class BehaviorTree;

} // namespace BTLib


namespace gamed {

class AreaObj;
/**
 * @brief AreaEventIf
 *    （1）作为AreaObj的成员变量，与AreaObj绑定在一起
 */
class AreaEventIf
{
public:
	AreaEventIf(AreaObj& areaobj);
	~AreaEventIf();

	bool Init(int evscript_id);
	void PlayerEnterArea(RoleID roleid);


private:
	void ForEachEventBT(const EvScriptXML::XMLInfo& info);
	void ReleaseAllBTs();


private:
	bool      is_inited_;
	AreaObj&  area_obj_;

	typedef std::vector<BTLib::BehaviorTree*> BehaviorTreePtrVec;
	BehaviorTreePtrVec player_enter_area_bts_;
};

} // namespace gamed

#endif // GAMED_GS_EVENTSYS_SRC_AREA_EV_IF_H_
