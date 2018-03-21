#ifndef GAMED_GS_EVENTSYS_AREA_EVENT_FUNC_TABLE_H_
#define GAMED_GS_EVENTSYS_AREA_EVENT_FUNC_TABLE_H_

#include "shared/lua/lua_engine.h"

namespace gamed {
namespace areaEv {

///
/// 作为BT叶子节点所调用的函数，这些函数都必须是线程安全的！
/// 即不保存任何状态及修改任何全局变量
///

///
/// Condition
///
// ---- thread safe ----
int CheckPlayerLevel(lua_State* state);
int CheckPlayerClass(lua_State* state);


///
/// Action
///
// ---- thread safe ----
int DeliverItem(lua_State* state);


extern luabind::CFunction area_event_func_table[];
extern int func_table_size;

} // namespace areaEv
} // namespace gamed

#endif // GAMED_GS_EVENTSYS_AREA_EVENT_FUNC_TABLE_H_
