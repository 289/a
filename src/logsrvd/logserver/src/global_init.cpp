#include "global_init.h"

#include <dirent.h>
#include <errno.h>

#include "shared/base/base_define.h"
#include "shared/base/conf.h"
#include "logsrvd/common/glog_def.h"

#include "worker_thread.h"


namespace GLog {

using namespace std;

static shared::Conf  g_conf;
static WorkerThread* g_worker = NULL;
static string        g_logfile_path[GLog::NUM_LOG_TYPES];
static const char*   g_path_separator = "/";


static bool createDir(string s)
{
	size_t pre = 0;
	size_t pos = 0;
	int mdret  = -1;
	std::string dir;

	if (s[s.size() - 1] != '/')
	{
		// force trailing '/' so we can handle everything in loop
		s += '/';
	}

	while ((pos = s.find_first_of('/', pre)) != std::string::npos)
	{
		dir = s.substr(0, pos++);
		pre = pos;
		if (dir.size() == 0)
		{
			// if leading '/' first time is 0 length
			continue;
		}

		mdret = mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
		if (mdret == 0)
		{
			continue;
		}
		else if (mdret && errno != EEXIST)
		{
			printf("mkdir failure！ errno=%d\n", errno);
			return false;
		}
	}

	return true;
}

static bool initPath(const char* confFile)
{
	if (!g_conf.Init(confFile))
	{
		printf("conf init error!\n");
		return false;
	}

	string root = g_conf.find("Template", "Root") + g_path_separator;
	g_logfile_path[GLOG_TYPE_AU]     = root + g_conf.find("Template", "Auth");
	g_logfile_path[GLOG_TYPE_GW]     = root + g_conf.find("Template", "Gateway");
	g_logfile_path[GLOG_TYPE_DB]     = root + g_conf.find("Template", "Gamedb");
	g_logfile_path[GLOG_TYPE_GS]     = root + g_conf.find("Template", "GameServer");
	g_logfile_path[GLOG_TYPE_LINK]   = root + g_conf.find("Template", "Link");
	g_logfile_path[GLOG_TYPE_MASTER] = root + g_conf.find("Template", "Master");
	g_logfile_path[GLOG_TYPE_MATCH]  = root + g_conf.find("Template", "Match");

	for (int i = 0; i < NUM_LOG_TYPES; ++i)
	{
		if (!createDir(g_logfile_path[i]))
		{
			return false;
		}
	}

	return true;
}

bool InitLogServer(const char* confFile)
{
	if (!initPath(confFile))
	{
		return false;
	}

	std::vector<WorkerThread::LogFileInfo> logFileVec;
	for (int i = 0; i < NUM_LOG_TYPES; ++i)
	{
		WorkerThread::LogFileInfo info;
		info.type      = (GLogType)i;
		info.path      = g_logfile_path[i];
		info.base_name = LogFileBaseName[i];
		info.roll_size = LogFileRollSize[i];
		logFileVec.push_back(info);
	}

	g_worker = new WorkerThread();
	if (!g_worker->Init(logFileVec))
	{
		return false;
	}

	return true;
}

bool StopLogServer()
{
	g_worker->Stop();
	DELETE_SET_NULL(g_worker);
	return true;
}

WorkerThread* GetWorker()
{
	return g_worker;
}

} // namespace GLog

