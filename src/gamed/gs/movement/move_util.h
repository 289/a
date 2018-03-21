#ifndef GAMED_GS_MOVEMENT_MOVE_UTIL_H_
#define GAMED_GS_MOVEMENT_MOVE_UTIL_H_

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"


namespace gamed {

#define MOVE_PRINTF(fmt, ...) \
    if (!Gmatrix::GetServerParam().forbid_move_print) \
    { \
        __PRINTF(fmt, __VA_ARGS__); \
    }

} // namespace gamed

#endif // GAMED_GS_MOVEMENT_MOVE_UTIL_H_
