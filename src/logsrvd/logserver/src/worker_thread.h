#ifndef LOGSRVD_LOGSERVER_SRC_WORKER_THREAD_H_
#define LOGSRVD_LOGSERVER_SRC_WORKER_THREAD_H_

#include <map>
#include <string>
#include <vector>

#include "shared/base/blocking_queue.h"
#include "shared/net/protobuf/dispatcher_proto.h"
#include "logsrvd/common/glog_def.h"


namespace shared 
{
	class Thread;
} // shared


namespace logsrv 
{
	class LogMessage;
	class LogServerStop;
} // logsrv


namespace GLog {

class GLogger;

class WorkerThread
{
	typedef google::protobuf::Message* MessagePtr;
	typedef google::protobuf::Message& MessageRef;
public:
	struct LogFileInfo
	{
		GLogType    type;
		std::string path;
		std::string base_name;
		size_t      roll_size;
	};

public:
	WorkerThread();
	~WorkerThread();

	bool  Init(const std::vector<LogFileInfo>& fileList);
	void  PutProtocol(MessagePtr msg);
	void  Stop();


protected:
	void  Run();
	void  HandleGLogProtocol(const MessageRef msg);
	void  StartupRegisterMsgHandler();
	bool  GetTypeAndLevel(const logsrv::LogMessage& msg, GLogType& type, GLogLevel& level);
	static void* ThreadFunc(void* pdata);

	void  ProtoDefaultHandler(const MessageRef);
	void  HandleLogMessage(const logsrv::LogMessage& proto);
	void  HandleLogServerStop(const logsrv::LogServerStop& proto);


private:
	bool            is_done_;
	shared::Thread* thread_;
	typedef std::map<GLogType, GLogger*> GLoggerMap;
	GLoggerMap      logger_map_;

	shared::BlockingQueue<MessagePtr> log_msg_queue_;
	shared::ProtobufDispatcher        dispatcher_;
};

} // namespace GLog

#endif // LOGSRVD_LOGSERVER_SRC_WORKER_THREAD_H_
