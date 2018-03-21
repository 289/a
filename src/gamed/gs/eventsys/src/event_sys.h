#ifndef GAMED_GS_EVENTSYS_SRC_EVENT_SYS_H_
#define GAMED_GS_EVENTSYS_SRC_EVENT_SYS_H_

#include "shared/base/singleton.h"
#include "shared/base/conf.h"


namespace BTLib 
{
	class BehaviorTree;
} // namespace BTLib


namespace gamed {

class AreaEventManager;
class EvLuaEntity;

///
/// EventSys
///
class EventSys : public shared::Singleton<EventSys>
{
	friend class shared::Singleton<EventSys>;

	static const std::string function_dir;
	static const std::string param_dir;
	static const std::string area_ev_dir;

  // functions
public:
	static inline EventSys* GetInstance() {
		return &(get_mutable_instance());
	}
	static const std::string& AreaEvFuncPath();
	static const std::string& AreaEvParamPath();

	bool Init(const std::string& root, shared::Conf* gsconf);
	BTLib::BehaviorTree* CreateAreaEventBT(const std::string& bt_xml_text);

	EvLuaEntity* GetAreaEvLuaEntity();


protected:
	EventSys();
	~EventSys();
	
	inline const std::string& evscript_root() const;
	inline const std::string& area_func_path() const;
	inline const std::string& area_param_path() const;


private:
	void InitVariousPaths(const std::string& evs_root);
	bool InitEventManagers();
	bool GetAllLuaFiles(const std::string& dir, std::vector<std::string>& vec);
	bool HasEnding(const std::string& fullstring, const std::string& ending);
	bool LoadAllLuaFiles(shared::Conf* gsconf);
	bool PreloadLuaFilesFromConfig(const std::string& files, 
		                           const std::string& dir, 
								   std::vector<std::string>& preload_vec);
	bool LoadAreaEvLuaFiles();


private:
	bool           is_inited_;
	std::string    evscript_root_;

	// area directory
	std::string    area_ev_root_;
	std::string    area_func_path_;
	std::string    area_param_path_;

	typedef std::vector<std::string> PreloadLuaFileVec;
	PreloadLuaFileVec  area_preload_vec_; // XXX: 注意要先push_back所有global_preload后，在push自己的preload文件

	// manager
	AreaEventManager*  parea_ev_man_;
};

///
/// inline func
///
inline const std::string& EventSys::evscript_root() const
{
	return evscript_root_;
}

inline const std::string& EventSys::area_func_path() const
{
	return area_func_path_;
}

inline const std::string& EventSys::area_param_path() const
{
	return area_param_path_;
}

#define s_pEventSys gamed::EventSys::GetInstance()

} // namespace gamed

#endif // GAMED_GS_EVENTSYS_SRC_EVENT_SYS_H_
