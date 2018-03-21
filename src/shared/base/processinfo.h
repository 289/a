#ifndef SHARED_BASE_PROCESSINFO_H_
#define SHARED_BASE_PROCESSINFO_H_

#include <vector>

#include "shared/base/types.h"
#include "shared/base/timestamp.h"

namespace shared {

namespace ProcessInfo
{
	pid_t		pid();
	uid_t		uid();
	uid_t		euid();
	Timestamp	start_time();
    std::string	pid_string();
    std::string	user_name();
    std::string host_name();
    std::string procname();

	/// read /proc/self/status
    std::string	proc_status();

	/// read /proc/self/stat
    std::string proc_stat();

	int			opened_files();
	int			max_open_files();

	int			num_threads();
	std::vector<pid_t> threads();
}

}

#endif  // SHARED_BASE_PROCESSINFO_H_
