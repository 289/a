#include "global_init.h"

#include <signal.h>

#include "shared/base/strtoken.h"
#include "shared/logsys/logging.h"

#include "utility_lib/CGLib/cglib.h"
#include "runnable_pool.h"
#include "timer.h"
#include "randomgen.h"
#include "gmatrix.h"
#include "loop_srv.h"


namespace gamed {
namespace GlobalInit {

static const int kRunnablePoolThreadNum = 4;

// 线程池至少有两个线程，一个处理tick，另一个处理顺序MSG
SHARED_STATIC_ASSERT(kRunnablePoolThreadNum >= 2);

static shared::Conf g_gs_conf;
static shared::Conf g_gmsrv_conf;
static shared::Conf g_alias_conf;

shared::Conf* GetGSConf()
{
	return &g_gs_conf;
}

shared::Conf* GetGMServerConf()
{
	return &g_gmsrv_conf;
}

shared::Conf* GetAliasConf()
{
	return &g_alias_conf;
}

namespace 
{
	void sigbus_handler(int)
	{
		LOG_ERROR << "SIGBUS signal recv!";
		_exit(0);
	}

	int InitSignalProcMask()
	{
		// SIG_BLOCK
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGALRM);
		sigprocmask(SIG_BLOCK, &set, NULL);

		// SIGBUS
		struct sigaction act;
		act.sa_handler = sigbus_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		sigaction(SIGBUS, &act, 0);

		return 0;
	}

	bool InitLoopService(shared::Conf& conf, int gs_id)
    {
        // 跨服gs需要对端口做偏移
        int port_delta  = 0;
        std::string str = conf.find("ServerParamInfo", "cross_realm_svr");
		if (strcmp(str.c_str(), "true") == 0) {
            port_delta = gs_id % 100;
        }

        if (!s_pLoopService->Init(conf, port_delta))
            return false;

        s_pLoopService->StartLoop();
        return true;
    }

    void StopLoopService()
	{
		s_pLoopService->StopLoop();
	}

} // Anonymous


int InitGameServer(const char* conf_file, const char* gmconf_file, const char* alias_file, int gs_id)
{
	if (!g_gs_conf.Init(conf_file))
	{
		LOG_ERROR << "gs.conf 读入错误!";
		return -1;
	}
	if (!g_gmsrv_conf.Init(gmconf_file))
	{
		LOG_ERROR << "gmserver.conf 读入错误!";
		return -1;
	}
	if (!g_alias_conf.Init(alias_file))
	{
		LOG_ERROR << "gsalias.conf 读入错误!";
		return -1;
	}

    // get gs id
	gs_id = (gs_id > 0) ? gs_id : g_alias_conf.get_int_value("Identify", "ServerID");
	
	// init randomgen
	mrand::Init();

	// init cglib random function
	CGLib::init_random_func(mrand::Rand);

	// signal mask
	InitSignalProcMask();

	// init loop service for GLog, and so on
	shared::Conf tmpconf;
	if (!tmpconf.AppendConfFile(gmconf_file) || !tmpconf.AppendConfFile(alias_file))
	{
		LOG_ERROR << "LoopService的Conf文件初始化失败！";
		return -1;
	}
	if (!InitLoopService(tmpconf, gs_id))
	{
		LOG_ERROR << "LoopService初始化失败！";
		return -1;
	}

	// common task pool
	RunnablePool::CreatePool(kRunnablePoolThreadNum);

	// start timer
	g_timer->TimerThread();

	// init game, always the last step
	int rst = s_pGmatrix->Init(&g_gmsrv_conf, &g_gs_conf, &g_alias_conf, gs_id);
	return rst;
}

int StopGameServer()
{
	// stop threads - attention the order:
	// gmatrix stop first, and g_timer stop before runnablepool
	s_pGmatrix->StopProcess();
	g_timer->StopThread();
	RunnablePool::StopPool();

	// stop loop service
	StopLoopService();

	// release all resource memory
	s_pGmatrix->ReleaseAll();
	return 0;
}

} // namespace GlobalInit
} // namespace gamed
