#include <time.h>
#include <algorithm>
#include "task_interface.h"
#include "storage_util.h"
#include "storage_templ.h"
#include "task_templ.h"
#include "task_manager.h"
#include "task_data.h"
#include "util.h"

namespace task
{

using namespace std;

int32_t StorageUtil::GetUpdateInterval(int32_t now, const StorageEntry& entry)
{
	const StorageTempl* templ = entry.templ;
	time_t last_update = entry.time;    
	if (templ->interval != 0)
	{
        return last_update + templ->interval - now;
	}
	struct tm last_tm;
#ifdef PLATFORM_WINDOWS
	localtime_s(&last_tm, &last_update);
#else
	localtime_r(&last_update, &last_tm);    
#endif
    last_tm.tm_mday += 1;
    return mktime(&last_tm) - now;
}

bool StorageUtil::NeedUpdate(Player* player, const StorageEntry& entry)
{
	//const StorageTempl* templ = entry.templ;
	//if ((int32_t)entry.task_list.size() >= templ->max_num)
	//{
		//return false;
	//}
    return GetUpdateInterval(player->GetCurTime(), entry) <= 0;
}

static void Rand(int32_t quality, int32_t num, const TaskInfoVec& info, TaskIDVec& task)
{
    if (num <= 0)
    {
        return;
    }

	// 归一化所有候选任务
	vector<int32_t> prob;
	TaskIDVec candidate;
	TaskInfoVec::const_iterator it = info.begin();
	for (; it != info.end(); ++it)
	{
        if (it->value == 0 || find(task.begin(), task.end(), it->id) != task.end())
        {
            continue;
        }
        if (quality != QUALITY_MAIN && s_pTask->GetTask(it->id)->quality < quality)
        {
            continue;
        }
		prob.push_back(it->value);
		candidate.push_back(it->id);
	}

	// 随机选取任务
	int32_t index = 0;
	while (prob.size() > 0 && num > 0)
	{
		index = Util::Rand(prob);
		task.push_back(candidate[index]);
		prob.erase(prob.begin() + index);
		candidate.erase(candidate.begin() + index);
		--num;
	}
}

void StorageUtil::Update(Player* player, StorageEntry& entry)
{
	// 计算需要补充的任务数
	const StorageTempl* templ = entry.templ;
	TaskIDVec& task_vec = entry.task_list;
	int32_t num = templ->max_num - (int32_t)task_vec.size();
    Rand(QUALITY_MAIN, num, templ->task_list, task_vec);
    entry.time = player->GetCurTime();
}

void StorageUtil::Update(Player* player, StorageEntry& entry, int32_t quality)
{
	// 计算需要补充的任务数
	const StorageTempl* templ = entry.templ;
	TaskIDVec& task_vec = entry.task_list;
	int32_t num = templ->max_num - (int32_t)task_vec.size();
    // 生成1个指定品质的库任务
    Rand(quality, 1, templ->task_list, task_vec);
    Rand(QUALITY_MAIN, num - 1, templ->task_list, task_vec);
    entry.time = player->GetCurTime();
}

} // namespace task
