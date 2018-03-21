#ifndef TASK_TASK_MANAGER_H_
#define TASK_TASK_MANAGER_H_

#include "base_task_templ.h"

namespace shared
{
namespace net
{
class Buffer;
template<typename T>
class TemplPacketCodec;
} // namespace net
} // namespace shared

namespace task
{

class TaskTempl;
class StorageTempl;
class MonsterPackageTempl;
class RatioTableTempl;

// 任务数据管理器
class TaskManager : public shared::Singleton<TaskManager>
{
	friend class shared::Singleton<TaskManager>;
public:
	static inline TaskManager* GetInstance() 
	{
		return &(get_mutable_instance());
	}

	bool ReadFromFile(const char* file);
	bool ReadFromBuffer(const shared::net::Buffer& buffer);
	bool WriteToFile(const char* file, std::vector<BaseTempl*>& vec_templ);

	template<class T> const T* QueryDataTempl(TemplType type, TemplID id) const;

	const TaskTempl* GetTask(TaskID id) const;
	const TaskTempl* GetTopTask(TaskID id) const;
	const TaskMap& GetAutoTask() const;
	const TaskMap& GetUITask() const;
	const TaskMap& GetMagTask() const;

	const StorageTempl* GetStorage(StorageID id) const;

	const MonsterPackageTempl* GetMonsterPackage(PackageID id) const; 
	bool IsInMonsterPackage(PackageID pid, MonsterID mid) const;

	const RatioTableTempl* GetRatioTable(int32_t id) const;
	int32_t GetRatioValue(int32_t id, int32_t level, int32_t value, bool ratio);
protected:
	TaskManager();
	~TaskManager();

	void LoadComplete();
	void UpdateTaskTempl();
	void UpdateStorageTempl();
	void UpdateTaskLevel();
private:
	shared::net::TemplPacketCodec<BaseTempl>* codec_;
	typedef std::map<TemplID, BaseTempl*> IdToTemplMap;
	typedef std::map<TemplType, IdToTemplMap> DataTemplMap;
	DataTemplMap templ_map_;

	TaskMap top_task_;
	TaskMap auto_task_;
	TaskMap ui_task_;
	TaskMap mag_task_;
	TaskMap group_task_;
};

template<class T>
const T* TaskManager::QueryDataTempl(TemplType type, TemplID id) const
{
	DataTemplMap::const_iterator mit = templ_map_.find(type);
	assert(mit != templ_map_.end());
	const IdToTemplMap& id_query_map = mit->second;
	IdToTemplMap::const_iterator it = id_query_map.find(id);
	if (it == id_query_map.end())
	{
		return NULL;
	}
	const BaseTempl* templ = it->second;
	assert(templ != NULL && templ->GetType() == type && templ->id == id);
	return dynamic_cast<const T*>(templ);
}

#define s_pTask TaskManager::GetInstance()

} // namespace task

#endif // TASK_TASK_MANAGER_H_
