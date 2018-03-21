#ifndef SHARED_LOGSYS_LOGFILE_H_
#define SHARED_LOGSYS_LOGFILE_H_

#include "shared/base/types.h" //using std::string
#include "shared/base/noncopyable.h"
#include "shared/base/mutex.h"

namespace shared {

class LogFile : noncopyable
{
public:
	LogFile(const std::string& basename,
			size_t rollSize,
			bool threadSafe = true,
			int flushInterval = 3);

	LogFile(const std::string& path,
			const std::string& basename,
			size_t rollSize,
			bool threadSafe = true,
			int flushInterval = 3);

	~LogFile();

	void	Append(const char* logline, int len);
	void	Flush();

private:
	void	AppendUnlocked(const char* logline, int len);
	void	RollFile();

	static std::string get_logfile_name(const std::string& basename, time_t* now);

	const std::string path_;
	const std::string basename_;
	const size_t	  rollsize_;
	const int		  flush_interval_;

	int				  count_;

	MutexLock*		  mutex_;
	time_t			  start_of_period_;
	time_t			  last_roll_;
	time_t			  last_flush_;
	class File;
	File*			  file_;

	const static int kCheckTimeRoll_	= 1024;
	const static int kRollPerSeconds_	= 60*60*24;
};

} // namespace shared

#endif // SHARED_LOGSYS_LOGFILE_H_
