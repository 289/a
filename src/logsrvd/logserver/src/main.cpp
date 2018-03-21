#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared/net/manager/init.h"
#include "shared/net/manager/network.h"
#include "shared/net/eventloop.h"

#include "logsrv_server.h"
#include "global_init.h"

using namespace shared::net;
using namespace GLog;


void int_handler(int no)
{
	printf("\nBye!\n");
	NetWork::QuitLoop();
}

void usage(const char* name)
{
	printf("Usage: %s [-v] [-h] [configurefile]\n", name);
}

/**
 * @brief main 
 *
 */
int main(int argc, char* argv[])
{
	///
	/// signal handle
	///
	signal(SIGINT, int_handler);

	/// 
	/// touch file system
	///
	if (system("/bin/touch foo"))
	{
		printf("文件系统不可写，无法进行后继的初始化操作\n");
		return -1;
	}

	///
	/// option parse
	///
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

	///
	/// access conf file
	///
	const char* conf_file = (argc > 1) ? argv[optind] : "server.conf";
	if (optind > argc || access(conf_file, R_OK) == -1)
	{
		usage(argv[0]);
		exit(-1);
	}

	///
	/// init conf file
	///
	if (!init::InitConf(conf_file))
	{
		printf("初始化conf文件失败！\n");
		return -1;
	}

	///
	/// global init
	///
	if (!InitLogServer(conf_file))
	{
		printf("InitLogServer() error！\n");
		return -1;
	}

	///
	/// network
	/// 
	EventLoop eventloop;
	NetWork::Init(&eventloop);

	LogsrvServer* pserver = LogsrvServer::GetInstance();
	NetWork::Server(pserver);

	NetWork::RunLoop();

	StopLogServer();
	pserver->Release();
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
