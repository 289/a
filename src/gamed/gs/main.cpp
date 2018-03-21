#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <google/protobuf/stubs/common.h>

#include "gs/global/global_init.h"


using namespace gamed;


static bool done = false;

void int_handler(int no)
{
	printf("\nBye!\n");
	done = true;
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
	/// gameserver initialization
	///
	const char * conf_file   = "gs.conf";
	const char * gmconf_file = "gmserver.conf";
	const char * alias_conf  = "gsalias.conf";
    int          gs_id       = 0;
	if (argc >1) conf_file   = argv[1];
	if (argc >2) gmconf_file = argv[2];
	if (argc >3) alias_conf  = argv[3];
    if (argc >4) gs_id       = atoi(argv[4]);

	int ret = GlobalInit::InitGameServer(conf_file, gmconf_file, alias_conf, gs_id);
	if (ret)
	{
		printf("初始化失败，错误号：%d\n", ret);
		exit(ret);
	}

	///
	/// main thread wait
	///
	while (!done)
	{
		sleep(60);
	}

	///
	/// stop gameserver
	///
	ret = GlobalInit::StopGameServer();
	if (ret)
	{
		printf("GameServer停止失败，错误号：%d\n", ret);
		exit(ret);
	}

	///
	/// exit program
	/// 
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
