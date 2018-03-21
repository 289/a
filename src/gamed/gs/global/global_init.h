#ifndef GAMED_GS_GLOBAL_GLOBAL_INIT_H_
#define GAMED_GS_GLOBAL_GLOBAL_INIT_H_

#include "shared/base/conf.h"


namespace gamed {
namespace GlobalInit {

shared::Conf* GetGSConf();
shared::Conf* GetGMServerConf();
shared::Conf* GetAliasConf();

int InitGameServer(const char* conf_file, const char* gmconf_file, const char* alias_file, int gs_id);

int StopGameServer();

} // namespace GlobalInit
} // namespace gamed

#endif // GAMED_GS_GLOBAL_GLOBAL_INIT_H_
