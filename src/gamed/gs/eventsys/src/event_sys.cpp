#include "event_sys.h"

#include <sys/types.h>
#include <dirent.h>

#include "shared/base/strtoken.h"
#include "shared/base/base_define.h"

#include "gs/global/dbgprt.h"
#include "gs/global/game_def.h"
#include "gs/eventsys/area_event/area_ev_manager.h"


namespace gamed {

using namespace std;

const std::string EventSys::function_dir    = "function/";
const std::string EventSys::param_dir       = "param/";
const std::string EventSys::area_ev_dir     = "area_event/";

///
/// static member func
///
const std::string& EventSys::AreaEvFuncPath()
{
	return s_pEventSys->area_func_path();
}

const std::string& EventSys::AreaEvParamPath()
{
	return s_pEventSys->area_param_path();
}

///
/// member func
///
EventSys::EventSys()
	: is_inited_(false),
	  parea_ev_man_(new AreaEventManager())
{
}

EventSys::~EventSys()
{
	DELETE_SET_NULL(parea_ev_man_);
}

bool EventSys::Init(const std::string& root, shared::Conf* gsconf)
{
	InitVariousPaths(root);

	if (!LoadAllLuaFiles(gsconf))
		return false;

	/*
	///
	/// preload files
	///
	shared::StrToken token;
	std::string files = "";
	const char* delim = ";,\r\n";
	std::vector<std::string> file_list;

	// 注意要先push_back所有global_preload后，在push自己的preload文件
	// global preload
	files = gsconf->find("EventScript", "GlobalPreload");
	if (token.GetTokenData<string>(files.c_str(), delim, file_list))
	{
		__PRINTF("解析[EventScript] - GlobalPreload文件列表错误.");
		return false;
	}
	for (size_t i = 0; i < file_list.size(); ++ i)
	{
		std::string scriptfile = evscript_root_ + file_list[i]; 
		area_preload_vec_.push_back(scriptfile);
	}
	file_list.clear();

	// area event preload
	files = gsconf->find("EventScript", "AreaEvPreload");
	if (token.GetTokenData<string>(files.c_str(), delim, file_list))
	{
		__PRINTF("解析[EventScript] - AreaEvPreload文件列表错误.");
		return false;
	}
	for (size_t i = 0; i < file_list.size(); ++ i)
	{
		std::string scriptfile = evscript_root_ + area_ev_dir + file_list[i]; 
		area_preload_vec_.push_back(scriptfile);
	}
	file_list.clear();
	*/

		
	// XXX: InitEventManagers应该放在本函数最末尾，等所有preload files都载入各个成员变量
	if (!InitEventManagers())
		return false;

	is_inited_ = true;
	return true;
}

bool EventSys::LoadAllLuaFiles(shared::Conf* gsconf)
{
	std::string files = "";

	///
	/// preload files from config
	///
	// 注意要先push_back所有global_preload后，在push自己的preload文件
	// global preload
	files = gsconf->find("EventScript", "GlobalPreload");
	if (!PreloadLuaFilesFromConfig(files, evscript_root_, area_preload_vec_))
		return false;

	// area preload
	files = gsconf->find("EventScript", "AreaEvPreload");
	if (!PreloadLuaFilesFromConfig(files, area_ev_root_, area_preload_vec_))
		return false;

	///
	/// param lua and function lua files
	///
	// load lua files
	if (!LoadAreaEvLuaFiles())
		return false;

	return true;
}

bool EventSys::PreloadLuaFilesFromConfig(const std::string& files, 
		                                 const std::string& dir, 
										 std::vector<std::string>& preload_vec)
{
	shared::StrToken token;
	const char* delim = ";,\r\n";
	std::vector<std::string> file_list;

	if (token.GetTokenData<string>(files.c_str(), delim, file_list))
	{
		__PRINTF("解析[EventScript] - Preload文件列表错误. files:%s", files.c_str());
		return false;
	}
	for (size_t i = 0; i < file_list.size(); ++ i)
	{
		std::string scriptfile = dir + file_list[i]; 
		preload_vec.push_back(scriptfile);
	}

	return true;
}

bool EventSys::LoadAreaEvLuaFiles()
{
	// 先载入param目录下的lua文件
	if (!GetAllLuaFiles(area_param_path_, area_preload_vec_))
		return false;

	// 再载入function目录下的
	if (!GetAllLuaFiles(area_func_path_, area_preload_vec_))
		return false;

	return true;
}

void EventSys::InitVariousPaths(const std::string& evs_root)
{
	evscript_root_   = evs_root;
	// area directory
	area_ev_root_    = evs_root + area_ev_dir;
	area_func_path_  = area_ev_root_ + function_dir;
	area_param_path_ = area_ev_root_ + param_dir;
}

bool EventSys::InitEventManagers()
{
	if (!parea_ev_man_->Init(kAreaEvLuaEntityCount, area_preload_vec_))
		return false;

	return true;
}

bool EventSys::HasEnding(const std::string& fullstring, const std::string& ending)
{
	if (fullstring.length() >= ending.length())
	{
		return (0 == fullstring.compare(fullstring.length() - ending.length(), 
					                    ending.length(), 
										ending));
	}

	return false;
}

bool EventSys::GetAllLuaFiles(const std::string& dir, std::vector<std::string>& vec)
{
	DIR* dp             = NULL;
	struct dirent* dirp = NULL;
	std::string ending  = ".lua";

	dp = opendir(dir.c_str());
	if (dp == NULL)
	{
		ASSERT(false && "不能打开对应的lua目录");
		return false;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		std::string name = dirp->d_name;
		if (dirp->d_type == DT_REG && HasEnding(name, ending))
		{
			vec.push_back(dir + name);
		}
	}

	closedir(dp);
	return true;
}

BTLib::BehaviorTree* EventSys::CreateAreaEventBT(const std::string& bt_xml_text)
{
	ASSERT(is_inited_);
	return parea_ev_man_->CreateBTFromXML(bt_xml_text);
}

EvLuaEntity* EventSys::GetAreaEvLuaEntity()
{
	return parea_ev_man_->GetLuaEntityRR();
}

} // namespace gamed
