#ifndef TASK_STORAGE_UTIL_H_
#define TASK_STORAGE_UTIL_H_

#include "task_types.h"

namespace task
{

class StorageEntry;

// 库任务辅助函数
class StorageUtil
{
public:
    static int32_t GetUpdateInterval(int32_t now, const StorageEntry& entry);
	static bool NeedUpdate(Player* player, const StorageEntry& entry);
	static void Update(Player* player, StorageEntry& entry);
	static void Update(Player* player, StorageEntry& entry, int32_t quality);
};

} // namespace task

#endif // TASK_STORAGE_UTIL_H_
