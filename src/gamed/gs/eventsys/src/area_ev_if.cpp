#include "area_ev_if.h"

#include "shared/base/base_define.h"
#include "utility_lib/BTLib/BTLibrary.h"

#include "gs/global/math_types.h"
#include "gs/global/dbgprt.h"
#include "gs/eventsys/area_event/area_player_leaf.h"

#include "ev_lua_ent.h"
#include "event_sys.h"


namespace gamed {

using namespace BTLib;

namespace 
{
	const std::string xmlFileEnding  = ".xml";
	const std::string areaTypeScript = "AreaScript";

	enum EventType
	{
		PLAYER_ENTER_AREA = 0,
	};

	struct EventTypeNode
	{
		int event;
		const std::string name;
	};

	EventTypeNode evtype_table[] = 
	{
		{PLAYER_ENTER_AREA, "PlayerEnterArea"},
	};
	
} // namespace Anonymous


///
/// AreaEventIf
///
AreaEventIf::AreaEventIf(AreaObj& areaobj)
	: is_inited_(false),
	  area_obj_(areaobj)
{
}

AreaEventIf::~AreaEventIf()
{
	ReleaseAllBTs();
	is_inited_ = false;
}

void AreaEventIf::ReleaseAllBTs()
{
	for (size_t i = 0; i < player_enter_area_bts_.size(); ++i)
	{
		DELETE_SET_NULL(player_enter_area_bts_[i]);
	}
	player_enter_area_bts_.clear();
}

bool AreaEventIf::Init(int evscript_id)
{
	std::string xml_file = EventSys::AreaEvParamPath() + itos(evscript_id) + xmlFileEnding;

	if (!EvScriptXML::ParseScriptXML(xml_file, BIND_MEM_CB(&AreaEventIf::ForEachEventBT, this)))
	{
		__PRINTF("area event script load error! %d.xml", evscript_id);
		return false;
	}

	is_inited_ = true;
	return true;
}

void AreaEventIf::ForEachEventBT(const EvScriptXML::XMLInfo& info)
{
	ASSERT(areaTypeScript == info.script_type);
	std::string checkno = itos(info.script_id) + "_" + itos(info.event_id); 
	ASSERT(checkno == info.evbt_name);

	BehaviorTree* bt_ptr = s_pEventSys->CreateAreaEventBT(info.evbt_xml_text);
	ASSERT(bt_ptr != NULL);

	if (info.event_type == evtype_table[PLAYER_ENTER_AREA].name)
	{
		player_enter_area_bts_.push_back(bt_ptr);
	}
	else
	{
		ASSERT(false);
	}
}

void AreaEventIf::PlayerEnterArea(RoleID roleid)
{
	ASSERT(is_inited_);

	AreaPlayerLeafParam param;
	EvLuaEntity* lua_ent = s_pEventSys->GetAreaEvLuaEntity();
	// lock
	EvLuaEntityLockGuard guard(*lua_ent);
	param.pengine = guard.lua_engine();
	param.roleid  = roleid;
	param.areaobj = &area_obj_;
	for (size_t i = 0; i < player_enter_area_bts_.size(); ++i)
	{
		player_enter_area_bts_[i]->setBlackboard(param);
		player_enter_area_bts_[i]->execute();
	}
}

} // namespace gamed
