#include <stdio.h>
#include <stdlib.h>
#include "shared/net/packet/codec_templ.h"
#include "task_manager.h"
#include "task_templ.h"
#include "storage_templ.h"
#include "monster_package_templ.h"
#include "ratio_table_templ.h"

namespace task
{

using namespace shared;
using namespace shared::net;
using namespace std;

#define TASKTEMPLATE_VERSION 0x00000001

TaskManager::TaskManager()
	: codec_(NULL)
{
	codec_ = new TemplPacketCodec<BaseTempl>(TASKTEMPLATE_VERSION,
											 BaseTemplCreater::CreatePacket,
											 BaseTemplCreater::IsValidType);
	assert(codec_ != NULL);
}

TaskManager::~TaskManager()
{
	DataTemplMap::const_iterator mit = templ_map_.begin();
	for (; mit != templ_map_.end(); ++mit)
	{
		const IdToTemplMap& id_query_map = mit->second;
		IdToTemplMap::const_iterator it = id_query_map.begin();
		for (; it != id_query_map.end(); ++it)
		{
			if (it->second != NULL)
			{
				delete it->second;
			}
		}
	}
	templ_map_.clear();

	top_task_.clear();
	auto_task_.clear();
	ui_task_.clear();
	mag_task_.clear();

	delete codec_;
	codec_ = NULL;
}

bool TaskManager::ReadFromFile(const char* file)
{
	FILE* pfile       = NULL;
	int32_t file_size = 0;
	int result        = 0;

	if ((pfile = fopen(file, "rb")) == NULL)
		return false;

	// 获取文件大小
	fseek(pfile, 0L, SEEK_END);
	file_size = ftell(pfile);
	rewind(pfile);

	// 分配Buffer
	Buffer buffer;
	buffer.EnsureWritableBytes(file_size);

	// 拷贝整个文件
	result = fread(buffer.BeginWrite(), 1, file_size, pfile);
	if (result != file_size)
	{
		fclose(pfile);
		return false;
	}
	buffer.HasWritten(result);
	
	// 解析
	if (!ReadFromBuffer(buffer))
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

static bool TopTaskFunc(const TaskTempl* task)
{
	return task->parent == 0;
}

static bool AutoTaskFunc(const TaskTempl* task)
{
	return task->auto_deliver();
}

static bool UITaskFunc(const TaskTempl* task)
{
	return !task->hide_task() && task->ui_task();
}

static bool MagTaskFunc(const TaskTempl* task)
{
	return !task->hide_task() && task->mag_task();
}

typedef bool (*TaskTypeFunc) (const TaskTempl* task);
static void AddToTaskMap(const TaskTempl* task, TaskMap& task_map, TaskTypeFunc func)
{
	if (!func(task))
	{
		return;
	}
	task_map[task->id] = task;
}

bool TaskManager::ReadFromBuffer(const Buffer& buffer)
{
	vector<BaseTempl*> tempvec;
	if (TPC_SUCCESS != codec_->ParseBuffer(&buffer, tempvec))
	{
		return false;
	}

	for (size_t i = 0; i < tempvec.size(); ++i)
	{
		BaseTempl* templ = tempvec[i];
		assert(templ->CheckDataValidity());
		TemplType templ_type = templ->GetType();
		templ_map_[templ_type][templ->id] = templ;
		if (templ_type == TEMPL_TYPE_TASK)
		{
			// 添加到不同类型的Map中方便针对不同情况选取任务
			TaskTempl* task = static_cast<TaskTempl*>(templ);
			AddToTaskMap(task, top_task_, TopTaskFunc);
			AddToTaskMap(task, auto_task_, AutoTaskFunc);
			AddToTaskMap(task, ui_task_, UITaskFunc);
			AddToTaskMap(task, mag_task_, MagTaskFunc);
		}
	}
	tempvec.clear();

	LoadComplete();
	return true;
}

bool TaskManager::WriteToFile(const char* file, vector<BaseTempl*>& vec_templ)
{
	for (size_t i = 0; i < vec_templ.size(); ++i)
	{
		if (!vec_templ[i]->CheckDataValidity()) 
		{
			return false;
		}
	}

	Buffer buffer;
	if (TPC_SUCCESS != codec_->AssemblingEmptyBuffer(&buffer, vec_templ))
	{
		return false;
	}
	
	FILE* pfile = NULL;
	if ((pfile = fopen(file, "wb+")) == NULL)
		return false;

	size_t result = fwrite(buffer.peek(), 1, buffer.ReadableBytes(), pfile);
	if (result != buffer.ReadableBytes())
	{
		fclose(pfile);
		return false;
	}

	fclose(pfile);
	return true;
}

const StorageTempl* TaskManager::GetStorage(StorageID id) const
{
	return QueryDataTempl<StorageTempl>(TEMPL_TYPE_STORAGE, id);
}

const MonsterPackageTempl* TaskManager::GetMonsterPackage(PackageID id) const
{
	return QueryDataTempl<MonsterPackageTempl>(TEMPL_TYPE_MONSTER_PACKAGE, id);
}

bool TaskManager::IsInMonsterPackage(PackageID pid, MonsterID mid) const
{
	const MonsterPackageTempl* templ = GetMonsterPackage(pid);
	return templ->monster_list.find(mid) != templ->monster_list.end();
}

const RatioTableTempl* TaskManager::GetRatioTable(int32_t id) const
{
	return QueryDataTempl<RatioTableTempl>(TEMPL_TYPE_RATIO_TABLE, id);
}

int32_t TaskManager::GetRatioValue(int32_t id, int32_t level, int32_t value, bool ratio)
{
	if (!ratio)
	{
		return value;
	}

	const RatioTableTempl* templ = GetRatioTable(id);
	return value * templ->GetValue(level);
}

const TaskTempl* TaskManager::GetTask(TaskID id) const
{
	return QueryDataTempl<TaskTempl>(TEMPL_TYPE_TASK, id);
}

const TaskTempl* TaskManager::GetTopTask(TaskID id) const
{
	TaskMap::const_iterator it = top_task_.find(id);
	return it == top_task_.end() ? NULL : it->second;
}

const TaskMap& TaskManager::GetAutoTask() const
{
	return auto_task_;
}

const TaskMap& TaskManager::GetUITask() const
{
	return ui_task_;
}

const TaskMap& TaskManager::GetMagTask() const
{
	return mag_task_;
}

void TaskManager::LoadComplete()
{
	UpdateTaskTempl();
	UpdateStorageTempl();
	UpdateTaskLevel();
}

void TaskManager::UpdateTaskTempl()
{
	IdToTemplMap& id_query_map = templ_map_[TEMPL_TYPE_TASK];
	IdToTemplMap::iterator it = id_query_map.begin();
	for (; it != id_query_map.end(); ++it)
	{
		TaskTempl* task = static_cast<TaskTempl*>(it->second);
		// 数据模版中存储的是UTC时间，各个服务器根据自己所在地区转换成对应的本地时
		task->premise.segment.segment.UTCToLocal();
		// 初始化树节点指针
		task->parent_templ = GetTask(task->parent);
		task->next_sibling_templ = GetTask(task->next_sibling);
		task->first_child_templ = GetTask(task->first_child);
		task->sid = 0;
	}
}

void TaskManager::UpdateStorageTempl()
{
	IdToTemplMap& id_query_map = templ_map_[TEMPL_TYPE_STORAGE];
	IdToTemplMap::iterator it = id_query_map.begin();
	for (; it != id_query_map.end(); ++it)
	{
		StorageTempl* group = static_cast<StorageTempl*>(it->second);
		const TaskInfoVec& task_list = group->task_list;
		TaskInfoVec::const_iterator tit = task_list.begin();
		for (; tit != task_list.end(); ++tit)
		{
			const TaskInfo& info = *tit;
			TaskTempl* task = const_cast<TaskTempl*>(GetTask(info.id));
			task->sid = group->id;
			group_task_[info.id] = task;
		}
	}

	// TODO 用户预先将任务按照品质存好
	// 方便以后玩家使用道具刷新库任务时可以指定必定出现某一品质的任务
}

void TaskManager::UpdateTaskLevel()
{
#ifdef CLIENT_SIDE
	IdToTemplMap& id_query_map = templ_map_[TEMPL_TYPE_TASK];
	IdToTemplMap::iterator it = id_query_map.begin();
	for (; it != id_query_map.end(); ++it)
	{
		TaskTempl* task = static_cast<TaskTempl*>(it->second);
		const TaskTempl* parent_task = task;

		do
		{
			char buff[1024] = {0};
			string hyphen = parent_task == task ? "" : ":";
			snprintf(buff, sizeof(buff), "%d%s", parent_task->id, hyphen.c_str());
			task->level = buff + task->level;
		}
		while ((parent_task = parent_task->parent_templ) != NULL);
	}
#endif
}

} // namespace task
