#include "proc_runtime_cmd.h"

#include "common/protocol/gen/global/gs_runtime_cmd.pb.h"
#include "shared/base/strtoken.h"
#include "gs/netio/netio_if.h"
#include "gs/netio/network_evloop.h"

#include "cmd_session.h"


namespace gamed {

using namespace std;
using namespace common::protocol;

namespace {

typedef std::string (*CmdHandleFunc)(const char* cmd);

/**
 * @brief GS运行时控制命令
 * （1）注释解析：X1-i（代表参数1是整型），X2-d（代表参数2是浮点型），X3-s（代表参数3是字符串）
 *
 */

//
// 参数：没有参数
//
std::string CmdHelp(const char* cmd)
{
	std::string tmpstr;
	tmpstr  = "available commands:\n";
	tmpstr += "\taddlink X1-ip X2-port\n";
	tmpstr += "\taddmaster X1-ip X2-port\n";
	return tmpstr;
}

//
// 参数：X1-s（ip）, X2-i（port）
//
std::string CmdAddLink(const char* cmd)
{
	std::string tmpstr("addlink - param error! Please enter \"help\" for more information");
	shared::StrToken token;
	const char* delim = " ";

	std::vector<std::string> param_list;
	if (token.GetTokenData<string>(cmd, delim, param_list) == 0)
	{
		if (param_list.size() == 3)
		{
			std::string ip = param_list[1];
			int port       = atoi(param_list[2].c_str());
			int ret        = NetIO::AddNewLink(ip.c_str(), port);
			if (ret == NetworkEvLoop::ANC_SUCCESS)
			{
				tmpstr = "addlink: Executed successfully";
			}
			else if (ret == NetworkEvLoop::ANC_ERR_PARAM)
			{
				tmpstr = "addlink: param error! ip or port invalid";
			}
			else if (ret == NetworkEvLoop::ANC_ERR_CONNECTED)
			{
				tmpstr = "addlink: link already connected!";
			}
		}
	}
	
	return tmpstr;
}

//
// 参数：X1-s（ip）, X2-i（port）
//
std::string CmdAddMaster(const char* cmd)
{
	std::string tmpstr("addmaster - param error! Please enter \"help\" for more information");
	shared::StrToken token;
	const char* delim = " ";

	std::vector<std::string> param_list;
	if (token.GetTokenData<string>(cmd, delim, param_list) == 0)
	{
		if (param_list.size() == 3)
		{
			std::string ip = param_list[1];
			int port       = atoi(param_list[2].c_str());
			int ret        = NetIO::AddNewMaster(ip.c_str(), port);
			if (ret == NetworkEvLoop::ANC_SUCCESS)
			{
				tmpstr = "addlink: Executed successfully";
			}
			else if (ret == NetworkEvLoop::ANC_ERR_PARAM)
			{
				tmpstr = "addlink: param error! ip or port invalid";
			}
			else if (ret == NetworkEvLoop::ANC_ERR_CONNECTED)
			{
				tmpstr = "addlink: link already connected!";
			}
		}
	}

	return tmpstr;
}

struct cmd_node
{
	const char*   cmd;
	CmdHandleFunc func;
};

static cmd_node cmd_list[] =
{
	{"help",        CmdHelp          },
	{"addlink",     CmdAddLink       },
	{"addmaster",   CmdAddMaster     },

// end
	{NULL,          0                }
};

bool CompareCmdName(const std::string& cmdstr, const std::string& name)
{
	// 小于
	if (cmdstr.size() < name.size())
		return false;

	// 等于
	if (cmdstr.size() == name.size())
	{
		if (cmdstr == name)
			return true;
	}

	// 大于
	if (cmdstr.size() > name.size())
	{
		size_t found = cmdstr.find(name);
		if (found == 0)
		{
			if (cmdstr[name.size()] == ' ')
				return true;
		}
	}

	return false;
}

} // Anonymous


/**
 * @brief ProcRuntimeCmd 
 *
 * @param cmd
 * @param session
 */
void ProcRuntimeCmd(const std::string& cmd, shared::net::ProtobufSession* session)
{
	std::string tmpstr = "command not found!";
	for (size_t i = 0; cmd_list[i].cmd != NULL; ++i)
	{
		if (CompareCmdName(cmd, cmd_list[i].cmd))
		{
			tmpstr = cmd_list[i].func(cmd.c_str());
			break;
		}
	}

	global::GSExecuteMessage proto;
	proto.set_message(tmpstr.c_str(), tmpstr.size());
	session->SendProtocol(proto);
}

} // namespace gamed
