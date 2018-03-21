#ifndef GAMED_GS_GLOBAL_DROP_TABLE_H_
#define GAMED_GS_GLOBAL_DROP_TABLE_H_

#include <stdint.h>
#include <vector>


namespace gamed {

class DropTable
{
public:
	struct ItemEntry
	{
		int32_t item_id;
		int32_t item_count;
	};

	static bool DropItem(int32_t tid, std::vector<ItemEntry>& item_vec);
};

} // namespace gamed

#endif // GAMED_GS_GLOBAL_DROP_TABLE_H_
