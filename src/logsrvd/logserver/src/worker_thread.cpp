#include "worker_thread.h"

#include "shared/base/thread.h"
#include "shared/base/base_define.h"
#include "shared/logsys/logging.h"
#include "logsrvd/protocol/log_message.pb.h"

#include "glogger.h"


namespace GLog {

using namespace std;
using namespace shared;

void* WorkerThread::ThreadFunc(void* pdata)
{
	WorkerThread* pworker = static_cast<WorkerThread*>(pdata);
	pworker->Run();
	return NULL;
}

WorkerThread::WorkerThread()
	: is_done_(false),
	  thread_(NULL),
	  dispatcher_(BIND_MEM_CB(&WorkerThread::ProtoDefaultHandler, this))
{
}

WorkerThread::~WorkerThread()
{
	Stop();
}

bool WorkerThread::Init(const std::vector<LogFileInfo>& fileList)
{
	for (size_t i = 0; i < fileList.size(); ++i)
	{
		const LogFileInfo& info = fileList[i];
		GLogger* plogger = new GLogger(info.path, info.base_name, info.roll_size);
		if (!logger_map_.insert(make_pair(info.type, plogger)).second)
		{
			return false;
		}
	}

	StartupRegisterMsgHandler();

	thread_ = new Thread(WorkerThread::ThreadFunc, this);
	thread_->Start();
	return true;
}

void WorkerThread::Stop()
{
	if (!is_done_)
	{
		is_done_ = true;
		logsrv::LogServerStop* proto = new logsrv::LogServerStop();
		proto->set_reason(logsrv::LogServerStop::NORMAL_EXIT);
		log_msg_queue_.put(proto);

		thread_->Join();
		DELETE_SET_NULL(thread_);

		GLoggerMap::iterator it = logger_map_.begin();
		for (; it != logger_map_.end(); ++it)
		{
			DELETE_SET_NULL(it->second);
		}
		logger_map_.clear();
	}
}

void WorkerThread::PutProtocol(MessagePtr msg)
{
	log_msg_queue_.put(msg);
}

void WorkerThread::Run()
{
	std::deque<MessagePtr> tmpList;
	while (!is_done_)
	{
		// block queue
		log_msg_queue_.takeAll(tmpList);
		while (!tmpList.empty())
		{
			MessagePtr pmsg = tmpList.front();
			HandleGLogProtocol(*pmsg);
			SAFE_DELETE(pmsg);
			tmpList.pop_front();
		}
	}
}

void WorkerThread::HandleGLogProtocol(const MessageRef msg)
{
	dispatcher_.OnProtobufMessage(msg);
}

void WorkerThread::StartupRegisterMsgHandler()
{
	dispatcher_.Register<logsrv::LogMessage>(BIND_MEM_CB(&WorkerThread::HandleLogMessage, this));
	dispatcher_.Register<logsrv::LogServerStop>(BIND_MEM_CB(&WorkerThread::HandleLogServerStop, this));
}

void WorkerThread::ProtoDefaultHandler(const MessageRef msg)
{
	LOG_ERROR << "WorkerThread::ProtoDefaultHandler() what proto? msg.type=" << msg.GetTypeName();
}

void WorkerThread::HandleLogMessage(const logsrv::LogMessage& msg)
{
	GLogType type;
	GLogLevel level;
	if (!GetTypeAndLevel(msg, type, level))
	{
		LOG_ERROR << "WorkerThread::HandleLogMessage() type:" << msg.type()
			<< " level:" << msg.level();
		return;
	}

	logger_map_[type]->writeLog(level, msg.id(), msg.msg());
}

bool WorkerThread::GetTypeAndLevel(const logsrv::LogMessage& msg, GLogType& type, GLogLevel& level)
{
	switch (msg.type())
	{
		case logsrv::LogMessage::AU_TYPE:
			type = GLOG_TYPE_AU;
			break;

		case logsrv::LogMessage::GW_TYPE:
			type = GLOG_TYPE_GW;
			break;

		case logsrv::LogMessage::DB_TYPE:
			type = GLOG_TYPE_DB;
			break;

		case logsrv::LogMessage::GS_TYPE:
			type = GLOG_TYPE_GS;
			break;

		case logsrv::LogMessage::LINK_TYPE:
			type = GLOG_TYPE_LINK;
			break;

		case logsrv::LogMessage::MASTER_TYPE:
			type = GLOG_TYPE_MASTER;
			break;

		case logsrv::LogMessage::MATCH_TYPE:
			type = GLOG_TYPE_MATCH;
			break;

		default:
			return false;
	}

	switch (msg.level())
	{
		case logsrv::LogMessage::TRACE:
			level = GLOG_TRACE;
			break;

		case logsrv::LogMessage::DEBUG:
			level = GLOG_DEBUG;
			break;

		case logsrv::LogMessage::INFO:
			level = GLOG_INFO;
			break;

		case logsrv::LogMessage::WARN:
			level = GLOG_WARN;
			break;

		case logsrv::LogMessage::ERROR:
			level = GLOG_ERROR;
			break;

		case logsrv::LogMessage::FATAL:
			level = GLOG_FATAL;
			break;

		default:
			return false;
	}

	return true;
}

void WorkerThread::HandleLogServerStop(const logsrv::LogServerStop& proto)
{
	if (proto.reason() != logsrv::LogServerStop::NORMAL_EXIT)
	{
		LOG_ERROR << "worker thread abnormal exit! reason: " << (int)proto.reason();
	}
}

} // namespace GLog
