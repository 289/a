#ifndef GAMED_GS_PLAYER_SUBSYS_IF_H_
#define GAMED_GS_PLAYER_SUBSYS_IF_H_

#include "common/obj_data/player_data.h"

#include "player_subsys.h"


namespace gamed {

using namespace common;

#define FOREACH_SUBSYS_0(subsys_ptr, func_name) \
{ \
	std::vector<PlayerSubSystem*>& tmpvec = subsys_ptr->subsys_vec_; \
	for (size_t i = 0; i < tmpvec.size(); ++i) \
	{ \
		tmpvec[i]->func_name(); \
	} \
}

#define FOREACH_SUBSYS_1(subsys_ptr, func_name, param1) \
{ \
	std::vector<PlayerSubSystem*>& tmpvec = subsys_ptr->subsys_vec_; \
	for (size_t i = 0; i < tmpvec.size(); ++i) \
	{ \
		tmpvec[i]->func_name(param1); \
	} \
}

#define FOREACH_SUBSYS_2(subsys_ptr, func_name, param1, param2) \
{ \
	std::vector<PlayerSubSystem*>& tmpvec = subsys_ptr->subsys_vec_; \
	for (size_t i = 0; i < tmpvec.size(); ++i) \
	{ \
		tmpvec[i]->func_name(param1, param2); \
	} \
}


///
/// SubSysDispatch
///
class SubSysDispatch : noncopyable	
{
public:
	SubSysDispatch() { }
	virtual ~SubSysDispatch() { }

	virtual bool OnSaveToDB(ObjectAttrPacket* pattr) = 0;
	virtual bool OnLoadFromDB(const ObjectAttrPacket& attr) = 0;
};

template <typename T>
class SubSysDispatchT : public SubSysDispatch
{
public:
	typedef shared::bind::Callback<bool (T*)> SaveDispatchCB;
	typedef shared::bind::Callback<bool (const T&)> LoadDispatchCB;

	SubSysDispatchT(const SaveDispatchCB& save_cb, const LoadDispatchCB& load_cb)
		: save_cb_(save_cb),
		  load_cb_(load_cb)
	{ }

	virtual bool OnSaveToDB(ObjectAttrPacket* pattr)
	{
		T* concrete = dynamic_cast<T*>(pattr);
		ASSERT(concrete != NULL);
		return save_cb_(concrete);
	}

	virtual bool OnLoadFromDB(const ObjectAttrPacket& attr)
	{
		T* concrete = dynamic_cast<T*>(&const_cast<ObjectAttrPacket&>(attr));
		ASSERT(concrete != NULL);
		return load_cb_(*concrete);
	}

private:
	SaveDispatchCB    save_cb_;
	LoadDispatchCB    load_cb_;
};


///
/// SubSysInterface
///
class SubSysIf
{
	friend class Player;
public:
	static const int kMsgHasBeenHandled  = 0;
	static const int kMsgHandlerNotFound = -1001;

	SubSysIf(Player& player);
	~SubSysIf();

	static SubSysMask GetRequiredSubSys();
	static SubSysMask GetDynamicSubSys();

	bool InitSubSys();
	bool AddSubSys(SubSysType type);
	bool AddSubSys(PlayerSubSystem* pSubSys);
	PlayerSubSystem* CreateSubSys(SubSysType type);
	PlayerSubSystem* QuerySubSys(SubSysType type);
	void Release();

	bool SaveToDB(PlayerData* pdata);
	bool LoadFromDB(const PlayerData& data);
	int  DispatchMsg(const MSG& msg);

	template <typename T>
	void Register(const typename SubSysDispatchT<T>::SaveDispatchCB& save_cb,
			      const typename SubSysDispatchT<T>::LoadDispatchCB& load_cb);
	
	void RegisterMsgHandler(MSG::MsgType msg_type, const PlayerSubSystem::MsgDispatcherCB& disp_cb);


private:
	bool InitRuntimeSubSys(const SubSysMask& mask);
	void CreateMultiSubSys(const SubSysMask& mask);
	bool SavePlayerAttrToDB(ObjectAttrPacket* pattr);
	bool LoadPlayerAttrFromDB(ObjectAttrPacket* pattr);


private:
	struct SubSysFinder
	{
		SubSysType type;
		explicit SubSysFinder(SubSysType _type): type(_type) {}
		bool operator()(const PlayerSubSystem* pSubSys)
		{
			return pSubSys->GetType() == type;
		}
	};

	Player&        player_;

	typedef std::vector<PlayerSubSystem*> SubSysVec;
	SubSysVec      subsys_vec_;
	SubSysMask     subsys_mask_;

	typedef std::map<ObjectAttrPacket::AttrType, SubSysDispatch*> DispatchMap;
	DispatchMap    dispatch_map_; 

	typedef std::map<MSG::MsgType, PlayerSubSystem::MsgDispatcherCB> MsgDispatchMap;
	MsgDispatchMap msg_dispatch_map_;
};

template <typename T>
void SubSysIf::Register(const typename SubSysDispatchT<T>::SaveDispatchCB& save_cb,
		                const typename SubSysDispatchT<T>::LoadDispatchCB& load_cb)
{
	DispatchMap::const_iterator it = dispatch_map_.find(T::TypeNumber());
	if (it != dispatch_map_.end())
	{
		LOG_ERROR << "Error: Multiple registration - by the same playersubsys TYPE:" << T::TypeNumber();
		ASSERT(false);
		return;
	}

	SubSysDispatch* pdispatch      = new SubSysDispatchT<T>(save_cb, load_cb);
	dispatch_map_[T::TypeNumber()] = pdispatch;
}

} // namespace gamed

#endif // GAMED_GS_PLAYER_SUBSYS_IF_H_
