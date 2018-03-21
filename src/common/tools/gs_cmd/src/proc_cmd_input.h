#ifndef COMMON_TOOLS_GSCMD_CMD_INPUT_H_
#define COMMON_TOOLS_GSCMD_CMD_INPUT_H_

#include <stdlib.h>
#include <string>

#include "shared/base/singleton.h"
#include "shared/base/countdownlatch.h"


namespace gsCmd {

using namespace shared;

class ProcessCommand : public Singleton<ProcessCommand>
{
public:
	static inline ProcessCommand* GetInstance() {
		return &(get_mutable_instance());
	}

	bool ProcAndSendCmd(const char* buf, size_t n);

	void CountDown();
	void CountDownWait();
	void SetCountDown(int count);

	void WaitServerProc();
	void ServerProcFinish();


protected:
	ProcessCommand();
	~ProcessCommand();

	std::string FormatCmdString(const char* buf, size_t n);


private:
	CountDownLatch latch_;
	int pipeFd_[2];
};


#define s_pProcCmd ProcessCommand::GetInstance()

} // namespace gsCmd

#endif // COMMON_TOOLS_GSCMD_CMD_INPUT_H_
