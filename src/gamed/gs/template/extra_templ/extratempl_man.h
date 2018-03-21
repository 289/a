#ifndef GAMED_GS_TEMPLATE_EXTRATEMPL_EXTRATEMPL_MAN_H_
#define GAMED_GS_TEMPLATE_EXTRATEMPL_EXTRATEMPL_MAN_H_

#include <map>

#include "templ_types.h"
#include "base_extratempl.h"

// template header
#include "monster_group.h"
#include "battle_scene.h"
#include "mapelem_ctrl.h"
#include "client_map_global.h"


namespace shared {
namespace net {

	class Buffer;
	template <typename T>
	class TemplPacketCodec;

} // namespace net
} // namespace shared


namespace extraTempl {

class ExtraTemplManager : public shared::Singleton<ExtraTemplManager>
{
	friend class shared::Singleton<ExtraTemplManager>;
public:
	static inline ExtraTemplManager* GetInstance() {
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool ReadFromBuffer(const shared::net::Buffer& buffer);
	bool WriteToFile(const char* path, std::vector<BaseExtraTempl*>& vec_templ);

	template<class T> const T* QueryExtraTempl(TemplID tpl_id);
	template<class T> void     QueryExtraTemplByType(std::vector<const T*>& ptr_vec);


protected:
	ExtraTemplManager();
	~ExtraTemplManager();

	typedef int16_t PacketType;
	typedef std::map<TemplID, const BaseExtraTempl*> IdToTemplMap;
	typedef std::map<PacketType, IdToTemplMap> TypeQueryMap;

	void InsertToSpecificMap(TemplID templ_id, const BaseExtraTempl* ptempl);
	bool CheckTemplateAfterRWFile(const TypeQueryMap& type_map);
	bool CheckMonsterGroupData(const IdToTemplMap& id_map) const;
	void CopyToTypeMap(const std::vector<BaseExtraTempl*>& vec_templ, TypeQueryMap& type_map);


private:
	shared::net::TemplPacketCodec<BaseExtraTempl>* codec_;
	TypeQueryMap    type_query_map_;
};

template<class T>
const T* ExtraTemplManager::QueryExtraTempl(TemplID tpl_id)
{
	TypeQueryMap::const_iterator it = type_query_map_.find(T::TypeNumber());
	if(it == type_query_map_.end())
	{
		//assert(false && "未知的模板Type");
		return NULL;
	}

	IdToTemplMap::const_iterator it_tpl = it->second.find(tpl_id);
	if (it_tpl == it->second.end())
	{
		//assert(false && "没有找到对应id的模板tpl_id");
		return NULL;
	}

	return dynamic_cast<const T*>(it_tpl->second);
}

template<class T>
void ExtraTemplManager::QueryExtraTemplByType(std::vector<const T*>& ptr_vec)
{
	TypeQueryMap::const_iterator it = type_query_map_.find(T::TypeNumber());
	if (it == type_query_map_.end())
	{
		ptr_vec.clear();
		//assert(false && "未知的模板Type");
		return;
	}

	const IdToTemplMap& baseptr_map = it->second;
	IdToTemplMap::const_iterator it_tpl = baseptr_map.begin();
	for (; it_tpl != baseptr_map.end(); ++it_tpl)
	{
		ptr_vec.push_back(dynamic_cast<const T*>(it_tpl->second));
	}
}
 
#define s_pExtraTempl extraTempl::ExtraTemplManager::GetInstance()

} // namespace gamed

#endif // GAMED_GS_TEMPLATE_EXTRATEMPL_EXTRATEMPL_MAN_H_
