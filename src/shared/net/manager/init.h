#ifndef INIT_H
#define INIT_H

#include "shared/base/conf.h"

namespace init
{

extern shared::Conf g_conf;
inline shared::Conf* GetConf()
{
	return &g_conf;
}

bool InitConf(const char *pfilename);
bool AppendConf(const char *pfilename);
void SetConf(const shared::Conf& rhs);

} // namespace init

#endif // INIT_H
