#include "proc_cmd_input.h"

#include <stdio.h>
#include "common/protocol/gen/global/gs_runtime_cmd.pb.h"
#include "gs_client.h"


namespace gsCmd {

using namespace common::protocol;

ProcessCommand::ProcessCommand()
	: latch_(1)
{
	if (pipe(pipeFd_) < 0)
	{
		assert(false);
	}
}

ProcessCommand::~ProcessCommand()
{
}

std::string ProcessCommand::FormatCmdString(const char* buf, size_t n)
{
	std::string tmpstr;
	tmpstr.assign(buf, n);

	while (tmpstr.size() && tmpstr[0] == ' ')
	{
		tmpstr = tmpstr.substr(1);
	}

	while (tmpstr.size() && tmpstr[tmpstr.size() - 1] == '\n')
	{
		tmpstr = tmpstr.substr(0, tmpstr.size() - 1);
	}

	return tmpstr;
}

bool ProcessCommand::ProcAndSendCmd(const char* buf, size_t n)
{
	if (s_pGSClient->is_connected())
	{
		std::string tmpstr = FormatCmdString(buf, n);
		if (tmpstr.size() != 0)
		{
			global::GameServerCmd proto;
			proto.set_cmd(tmpstr.c_str(), tmpstr.size());
			s_pGSClient->Send(proto);
			return true;
		}
		else
		{
			printf("Has no command enter!\n");
		}
	}
	else
	{
		fprintf(stderr, "GS haven't connected!\n");
	}
	return false;
}

void ProcessCommand::CountDown()
{
	latch_.CountDown();
}
	
void ProcessCommand::CountDownWait()
{
	latch_.Wait();
}

void ProcessCommand::WaitServerProc()
{
	static const int kBufferSize = 64;
	char line[kBufferSize];
	read(pipeFd_[0], line, kBufferSize);
}

void ProcessCommand::ServerProcFinish()
{
	const char* value = "a";
	write(pipeFd_[1], value, strlen(value));
}

} // namespace gsCmd
