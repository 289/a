#ifndef GAMED_GS_EVENTSYS_AREA_PLAYER_LEAF_H_
#define GAMED_GS_EVENTSYS_AREA_PLAYER_LEAF_H_

#include "shared/lua/lua_engine.h"
#include "shared/base/types.h"

#include "utility_lib/BTLib/BTLibrary.h"


namespace gamed {

class AreaObj;

///
/// Param
///
class AreaPlayerLeafParam : public BTLib::BTParam
{
public:
	AreaPlayerLeafParam()
		: pengine(NULL),
		  roleid(0),
		  areaobj(NULL)
	{ }

	virtual ~AreaPlayerLeafParam()
	{ }

	bool is_valid() const
	{
		if (pengine == NULL || roleid == 0 || areaobj == NULL)
			return false;
		return true;
	}

	luabind::LuaEngine* pengine;
	RoleID              roleid;
	const AreaObj*      areaobj;
};

///
/// AreaPlayerCondition
///
class AreaPlayerCondition : public BTLib::Condition
{
public:
	AreaPlayerCondition(BTLib::BehaviorTree* tree, BTLib::BTNode* parent);
	virtual ~AreaPlayerCondition();

	virtual void init();
	virtual BTLib::NodeStatus stepInto();

private:
	std::string param_id_;
};

///
/// AreaPlayerAction
///
class AreaPlayerAction : public BTLib::Action
{
public:
	AreaPlayerAction(BTLib::BehaviorTree* tree, BTLib::BTNode* parent);
	virtual ~AreaPlayerAction();

	virtual void init();
	virtual BTLib::NodeStatus stepInto();

private:
	std::string param_id_;
};


} // namespace gamed

#endif // GAMED_GS_EVENTSYS_AREA_PLAYER_LEAF_H_
