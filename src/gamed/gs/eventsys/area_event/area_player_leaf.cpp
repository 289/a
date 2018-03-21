#include "area_player_leaf.h"

#include <string.h>

#include "shared/lua/lua_value.h"
#include "shared/logsys/logging.h"

#include "gs/global/math_types.h"
#include "gs/eventsys/src/event_sys.h"


namespace gamed {

using namespace BTLib;
using namespace luabind;

#define CHACK_AND_TAKE_BT_PARAM(ptree, param) \
	if (!ptree->getBlackboard(param)) \
	{ \
		LOG_ERROR << "area_player_leaf ptree->getBlackBoard() error!"; \
		return FAILURE; \
	} \
	if (!param.is_valid()) \
	{ \
		LOG_ERROR << "area_player_leaf param invalid!"; \
		return FAILURE; \
	}


///
/// AreaPlayerCondition
///
AreaPlayerCondition::AreaPlayerCondition(BehaviorTree* tree, BTNode* parent)
	: Condition(tree, parent)
{
}

AreaPlayerCondition::~AreaPlayerCondition()
{
}

void AreaPlayerCondition::init()
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%04d", getId());
	param_id_ = tree_->getTreeName() + "_" + buf;
}

NodeStatus AreaPlayerCondition::stepInto()
{
	AreaPlayerLeafParam tree_param;
	CHACK_AND_TAKE_BT_PARAM(tree_, tree_param);

	LuaValueArray args;
	// 注意一定要转成intptr_t类型,能安全跨64位系统
	args.push_back(LuaValue(reinterpret_cast<intptr_t>(tree_param.areaobj)));
	args.push_back(LuaValue(param_id_));
	args.push_back(LuaValue(tree_param.roleid));

	std::string ev_script = s_pEventSys->AreaEvFuncPath() + scriptName_;
	if(!tree_param.pengine->Call(ev_script.c_str(), scriptFunc_.c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error";
		return FAILURE;
	}

	int res = -1;
	tree_param.pengine->PopValue(res);
	if (res) 
	{
		return FAILURE;
	}

	return BTLib::SUCCESS;
}


///
/// AreaPlayerAction
///
AreaPlayerAction::AreaPlayerAction(BehaviorTree* tree, BTNode* parent)
	: Action(tree, parent)
{
}

AreaPlayerAction::~AreaPlayerAction()
{
}

void AreaPlayerAction::init()
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%04d", getId());
	param_id_ = tree_->getTreeName() + "_" + buf;
}

/**
 * @brief stepInto 
 *     （1）stepInfo返回FAILURE，并不会影响行为树的运行，因为在BTLib::Action里做了统一处理（只return SUCCESS）
 */
NodeStatus AreaPlayerAction::stepInto()
{
	AreaPlayerLeafParam tree_param;
	CHACK_AND_TAKE_BT_PARAM(tree_, tree_param);

	LuaValueArray args;
	// 注意一定要转成intptr_t类型,能安全跨64位系统
	args.push_back(LuaValue(reinterpret_cast<intptr_t>(tree_param.areaobj)));
	args.push_back(LuaValue(param_id_));
	args.push_back(LuaValue(tree_param.roleid));

	std::string ev_script = s_pEventSys->AreaEvFuncPath() + scriptName_;
	if(!tree_param.pengine->Call(ev_script.c_str(), scriptFunc_.c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error";
		return FAILURE;
	}

	int res = -1;
	tree_param.pengine->PopValue(res);
	if (res) 
	{
		LOG_ERROR << "area_ev_script execute error! tree:" << tree_->getTreeName() << " script:" << scriptName_ 
			<< " func:" << scriptFunc_ << " return:" << res;
		return FAILURE;
	}

	return BTLib::SUCCESS;
}

} // namespace gamed
