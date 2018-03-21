#include <stdio.h>
#include <time.h>
#include <string.h>
#include <hash_map>

#include "point.h"
#include "map_astar.h"
#include "move_map.h"
#include "pf_centre.h"
#include "path_finder.h"
#include "expander.h"
#include "clock.h"
#include "shared/net/buffer.h"

namespace shared {
namespace net {
class Buffer;
};
};

int main()
{
	//初始化寻路模块
	if (!pathfinder::InitGlobalTransTable("global_trans_table.gbd"))
		return -1;

	//加载通路图
	/*
	if (!pathfinder::LoadMoveMap(3, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(900, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(901, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(902, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(6002, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	*/
	if (!pathfinder::LoadMoveMap(12, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(14, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;
	if (!pathfinder::LoadMoveMap(2006, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;


	Clock clock("main");
	clock.Start();

	//测试寻路
	pathfinder::PF2DVECTOR __source;
	pathfinder::PF2DVECTOR __dest;
	std::vector<pathfinder::PF2DVECTOR> __path;

#if 1

	//同地图同区域寻路

	__source = pathfinder::PF2DVECTOR(-8.5f, -1.0f);
	__dest   = pathfinder::PF2DVECTOR(-11.0f, 10.5f);

	//__source = pathfinder::PF2DVECTOR(-11.25000, -10.750000);
	//__dest   = pathfinder::PF2DVECTOR(6.3400002, -6.8000002);

	/*
	__source = pathfinder::PF2DVECTOR(7.750000f, -3.750000f);
	__dest   = pathfinder::PF2DVECTOR(9.750000f, -4.250000f);
	*/

	if (pathfinder::SearchPath(14, __source, __dest, 0.5f, __path))
	{
		fprintf(stdout, "main::search successfull!!! SUCC!!!\n");
		fprintf(stdout, "__source(%f,%f), __dest(%f,%f)\n", __source.x, __source.z, __dest.x, __dest.z);
		fprintf(stdout, "__path=");
		for (size_t i = 0; i < __path.size(); ++ i)
		{
			if (pathfinder::IsWalkable(14, __path[i]))
				fprintf(stdout, "(%f,%f)", __path[i].x, __path[i].z);
			else
				assert(false);
		}
		fprintf(stdout, "\n\n");
	}
	else
	{
		fprintf(stdout, "main::search failed!!! ERROR!!!\n");
		fprintf(stdout, "__source(%f,%f), __dest(%f,%f)\n", __source.x, __source.z, __dest.x, __dest.z);
	}

#else

	//跨区域或跨地图寻路

	__source = pathfinder::PF2DVECTOR(49.456932, -8.0275545);
	__dest   = pathfinder::PF2DVECTOR(9.7191305, -6.2795515);

	pathfinder::WayFinder way_finder;
	if (!way_finder.Start(14, __source, 14, __dest, 0.5f))
	{
		fprintf(stdout, "main::Start failed!!!, __source(%f,%f), __dest(%f,%f)\n", __source.x, __source.z, __dest.x, __dest.z);
		pathfinder::Release();
		return -1;
	}

	fprintf(stdout, "main::start search. __source(%f,%f), __dest(%f,%f)\n\n", __source.x, __source.z, __dest.x, __dest.z);
	for (;;)
	{
		pathfinder::PF2DVECTOR cur_source, cur_dest;
		way_finder.GetCurStartPos(cur_source);
		way_finder.GetCurEndPos(cur_dest);

		if (way_finder.NextPath(__path))
		{
			fprintf(stdout, "main::search successfull!!!, __source(%f,%f), __dest(%f,%f)\n", cur_source.x, cur_source.z, cur_dest.x, cur_dest.z);
			//fprintf(stdout, ", __path=(");
			//for (size_t i = 0; i < __path.size(); ++ i)
			//{
			//	fprintf(stdout, "(%f,%f)", __path[i].x, __path[i].z);
			//}
			//fprintf(stdout, ")\n\n");
		}
		else
		{
			break;
		}

		__path.clear();
	};

#endif
	clock.End();
	clock.ElapseTime();

	/*
	//test npc-chase
	pathfinder::PF2DVECTOR start(-14.4399996, 14.1999998);
	pathfinder::PF2DVECTOR target(-13.7694702, 12.7850027);
	float range = 0.56f;

	pathfinder::NpcChase chase;
	chase.Start(1, start, target, range);

	fprintf(stdout, "Chase, start(%f,%f), target(%f,%f), range=%f)\n", start.x, start.z, target.x, target.z, range);

	for (;;)
	{
		chase.MoveOneStep(0.5f);

		pathfinder::PF2DVECTOR curpos = chase.GetCurPos();
		fprintf(stdout, "(%f,%f),", curpos.x, curpos.z);

		if (chase.GetToGoal())
		{
			break;
		}
	}
	fprintf(stdout, "\n");
	*/

	//test npc-ramble
	/*
	pathfinder::PF2DVECTOR start(1974.0f, 267.0f);
	pathfinder::PF2DVECTOR centre(1974.0f, 267.0f);
	float range = 1.0f;

	pathfinder::NpcRamble ramble;
	ramble.Start(3, start, centre, 0.5f, range);

	pathfinder::PF2DVECTOR goal = ramble.GetGoalPos();

	fprintf(stdout, "Ramble, start(%f,%f), Range((%f,%f),range=%f), goal(%f,%f), path=\n", start.x, start.z, centre.x, centre.z, range, goal.x, goal.z);

	for (;;)
	{
		ramble.MoveOneStep();
		pathfinder::PF2DVECTOR cur_pos = ramble.GetCurPos();
		fprintf(stdout, "(%f, %f)\n", cur_pos.x, cur_pos.z);

		if (ramble.GetToGoal())
		{
			break;
		}
	}
	*/

	///
	/// 测试需找指定点附近的可达点
	///
	pathfinder::PF2DVECTOR __pos;
	__pos = pathfinder::PF2DVECTOR (-8.0893087, 6.8979726);
	pathfinder::PF2DVECTOR __newpos;

	pathfinder::Expander expander;
	if (!pathfinder::FindNearbyWalkablePos(14, __pos, __newpos))
	{
		fprintf(stdout, "expander failed\n");
	}

	//释放寻路模块
	pathfinder::Release();
	return 0;
}
