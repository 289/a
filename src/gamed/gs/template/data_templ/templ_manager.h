#ifndef GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_MANAGER_H_
#define GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_MANAGER_H_

#include <map>
#include <vector>

#include "shared/base/singleton.h"
#include "templ_types.h"


namespace shared {
namespace net {

	class Buffer;
	template <typename T>
	class TemplPacketCodec;

} // namespace net
} // namespace shared


namespace dataTempl {

class BaseDataTempl;
class ItemDataTempl;
class GlobalConfigTempl;

class DataTemplManager : public shared::Singleton<DataTemplManager>
{
	friend class shared::Singleton<DataTemplManager>;
public:
	static inline DataTemplManager* GetInstance() {
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool ReadFromBuffer(const shared::net::Buffer& buffer);
	bool WriteToFile(const char* file, std::vector<BaseDataTempl*>& vec_templ);

	const BaseDataTempl*       QueryBaseDataTempl(TemplID tpl_id) const;
	template<class T> const T* QueryDataTempl(TemplID tpl_id) const;
	template<class T> void     QueryDataTemplByType(std::vector<const T*>& ptr_vec) const;
	const GlobalConfigTempl*   QueryGlobalConfigTempl() const;
	void CollectAllItemTempl(std::vector<const ItemDataTempl*>& list) const;

	bool CheckAllTemplate() const;


protected:
	DataTemplManager();
	~DataTemplManager();

	void InsertToSpecificMap(TemplID templ_id, const BaseDataTempl* ptempl);
	bool CheckInstanceGroup() const;
    bool CheckWorldBossAward() const;
    bool CheckItemCoolDownGroup() const;
    bool CheckGeventGroup() const;
    bool CheckBattleGround() const;
    bool CheckInstanceTempl() const;

private:
    template<class T> int CheckItemCDGroup() const;
    int  CheckCoolDownGroup(TemplID cd_group_id) const;


private:
	shared::net::TemplPacketCodec<BaseDataTempl>* codec_;

	typedef std::vector<const BaseDataTempl*> BaseTemplVec;
	typedef std::map<TemplID, const BaseDataTempl*> IdToTemplMap;
	IdToTemplMap      id_query_map_;

	typedef int16_t PacketType;
	typedef std::map<PacketType, BaseTemplVec> SpecialMapByType;
	SpecialMapByType  specific_type_query_map_;

	const GlobalConfigTempl* global_config_templ_;
};

template<class T>
const T* DataTemplManager::QueryDataTempl(TemplID tpl_id) const
{
	const BaseDataTempl* pbase_ptr = QueryBaseDataTempl(tpl_id);
	if (pbase_ptr == NULL)
		return NULL;
	return dynamic_cast<const T*>(pbase_ptr);
}

template<class T>
void DataTemplManager::QueryDataTemplByType(std::vector<const T*>& ptr_vec) const
{
	SpecialMapByType::const_iterator it = specific_type_query_map_.find(T::TypeNumber());
	if (it == specific_type_query_map_.end())
	{
		ptr_vec.clear();
		return;
	}

	const BaseTemplVec& baseptr_vec = it->second;
	for (size_t i = 0; i < baseptr_vec.size(); ++i)
	{
		ptr_vec.push_back(dynamic_cast<const T*>(baseptr_vec[i]));
	}
}

template<class T>
int DataTemplManager::CheckItemCDGroup() const
{
    std::vector<const T*> tpl_vec;
	QueryDataTemplByType(tpl_vec);
	for (size_t i = 0; i < tpl_vec.size(); ++i)
	{
        if (tpl_vec[i]->cooldown_data.cd_group_id == 0)
            continue;
        if (CheckCoolDownGroup(tpl_vec[i]->cooldown_data.cd_group_id) != 0)
            return -1;
    }
    tpl_vec.clear();
    return 0;
}
 
#define s_pDataTempl dataTempl::DataTemplManager::GetInstance()

} // namespace gamed

#endif // GAMED_GS_TEMPLATE_DATATEMPL_TEMPL_MANAGER_H_
