#include "drop_table.h"

#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/monster_drop_templ.h"
#include "randomgen.h"


namespace gamed {

using namespace dataTempl;

static const size_t  kMaxProbSize     = 256;

bool DropTable::DropItem(int32_t tid, std::vector<ItemEntry>& item_vec)
{
	const MonsterDropTempl* ptempl = s_pDataTempl->QueryDataTempl<MonsterDropTempl>(tid);
	if (ptempl == NULL)
		return false;

	// copy prob
	ASSERT(kMaxProbSize >= ptempl->drop_table.size());
	int32_t prob_value[kMaxProbSize] = {0};
	int32_t prob_index[kMaxProbSize] = {0};
	for (size_t i = 0; i < ptempl->drop_table.size(); ++i)
	{
		prob_value[i] = ptempl->drop_table[i].drop_prob;
		prob_index[i] = i;
		ASSERT(prob_value[i] >= 0);
	}

	// rand drop
	size_t prob_size = ptempl->drop_table.size();
	for (int count = 0; count < ptempl->drop_times; ++count)
	{
		int index = mrand::RandSelect(prob_value, prob_size);

		// get item
		int real_index = prob_index[index];
		ItemEntry ent;
		ent.item_id    = ptempl->drop_table[real_index].item_id;
		ent.item_count = ptempl->drop_table[real_index].item_count;
		item_vec.push_back(ent);

		// unique drop
		if (ptempl->drop_table[index].unique_drop)
		{
			for (size_t j = index; j < prob_size - 1; ++j)
			{
				prob_value[j] = prob_value[j+1];
				prob_index[j] = prob_index[j+1];
			}
			prob_size -= 1;
			prob_value[prob_size] = 0;
			prob_index[prob_size] = 0;

			// end
			if (prob_size <= 0)
				break;

			// normalization
            mrand::Normalization(prob_value, prob_size);
		}
	}

	return true;
}

} // namespace gamed
