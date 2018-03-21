#include "faction.h"

#include "shared/base/assertx.h"
#include "gs/template/data_templ/battleground_templ.h"


namespace gamed {

    SHARED_STATIC_ASSERT(BGFI_MAX == dataTempl::BattleGroundTempl::kMaxFactionCount);

} // namespace gamed
