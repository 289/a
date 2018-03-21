#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "point.h"
#include "clock.h"
#include "path_track.h"
#include "path_finder.h"

using namespace pathfinder;
int main()
{
	Path path;
	path.push_back(PointI(1,1));
	path.push_back(PointI(2,1));
	path.push_back(PointI(3,1));
	path.push_back(PointI(3,2));
	path.push_back(PointI(4,3));
	path.push_back(PointI(6,3));
	path.push_back(PointI(6,4));
	path.push_back(PointI(6,5));
	path.push_back(PointI(6,6));
	path.push_back(PointI(6,7));

	//初始化寻路模块
	if (!pathfinder::InitGlobalTransTable("global_trans_table.gbd"))
		return -1;

	//加载通路图
	if (!pathfinder::LoadMoveMap(1, "/home/luoxinsheng/svnproject/server/trunk/game_src/gamed/game_module/pathfinder/tests/map/"))
		return -1;


	PF2DVECTOR __source(-62.0f, -30.0f);
	PF2DVECTOR __dest(-51.0f, 31.0f);

	Clock clock("test_path");
	clock.Start();

	for (size_t i = 0; i < 100; ++ i)
	{
		std::vector<pathfinder::PF2DVECTOR> __path;
		if (pathfinder::SearchPath(1, __source, __dest, 0.5f, __path))
		{
			fprintf(stdout, "main::search successfull!!! SUCC!!!\n");
			fprintf(stdout, "__source(%f,%f), __dest(%f,%f)\n", __source.x, __source.z, __dest.x, __dest.z);
			fprintf(stdout, "__path=");
			for (size_t i = 0; i < __path.size(); ++ i)
			{
				if (pathfinder::IsWalkable(1, __path[i]))
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
	}

	clock.End();
	clock.ElapseTime();

	pathfinder::Release();
	return 0;
}
