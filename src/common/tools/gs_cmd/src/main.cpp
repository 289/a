#include <stdio.h>
#include <signal.h>

#include "shared/net/manager/network.h"
#include "shared/net/eventloop.h"

#include "loop_srv.h"
#include "gs_client.h"
#include "proc_cmd_input.h"


using namespace shared;
using namespace shared::net;
using namespace gsCmd;

#define INPUT_BUFFER_SIZE 1024

static bool done = false;

bool init_loop_srv(const char* ip_addr, int port)
{
	if (!s_pLoopService->Init())
		return false;

	s_pLoopService->StartLoop();

	NetWork::Client(s_pGSClient, ip_addr, port);
	s_pGSClient->EnableRetry(false);
	return true;
}

void stop_loop_srv()
{
	s_pLoopService->StopLoop();
}

void int_handler(int no)
{
	printf("\nBye!\n");
	done = true;
	s_pProcCmd->CountDown();
	s_pProcCmd->ServerProcFinish();
	printf("Press ENTER to quit ...\n");
}

void usage(const char* name)
{
	printf("Usage: %s [-v] [-h] <configurefile> <section_num>\n", name);
}

/**
 * @brief main 
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		usage(argv[0]);
		exit(-1);
	}

	// signal
	signal(SIGINT, int_handler);

	// option
	int opt;
	while ((opt = getopt(argc, argv, "hv")) != EOF)
	{
		switch (opt)
		{
			case 'v':
				printf("%s\n", "??????? version");
				exit(0);
			default:
				usage(argv[0]);
				exit(0);
		}
	}

	if (optind >= argc)
	{
		usage(argv[0]);
		exit(-1);
	}

	// init network
	const char* ip_addr = argv[optind];
	int32_t port = atoi(argv[optind + 1]);
	if (!init_loop_srv(ip_addr, port))
	{
		printf("网络初始化失败！\n");
		return -1;
	}

	// wait latch
	s_pProcCmd->CountDownWait();

	// cmd input
	char* line = NULL;
	size_t len = 0;
	ssize_t read;
	while (!done)
	{
		printf("\nplease enter cmd: ");
		read = getline(&line, &len, stdin);
		if (!done && read > 0)
		{
			if (s_pProcCmd->ProcAndSendCmd(line, read))
			{
				s_pProcCmd->WaitServerProc();
			}
		}
		else if (read < 0)
		{
			printf("stdin输入错误\n");
			done = true;
		}
	}

	// stop loop
	stop_loop_srv();

	// release
	if (line) 
	{
		free(line);
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
