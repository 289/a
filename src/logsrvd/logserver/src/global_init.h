#ifndef LOGSRVD_LOGSERVER_SRC_GLOBAL_INIT_H_
#define LOGSRVD_LOGSERVER_SRC_GLOBAL_INIT_H_


namespace GLog {

class WorkerThread;

bool InitLogServer(const char* confFile);
bool StopLogServer();

WorkerThread* GetWorker();

} // namespace GLog

#endif // LOGSRVD_LOGSERVER_SRC_GLOBAL_INIT_H_
