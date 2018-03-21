#include <stdio.h>
#include <unistd.h>

#include "gs/scene/aoi.h"
#include "shared/base/atomic.h"


using namespace gamed;

enum
{
	PLAYER = 0,
	MONSTER,
	AREA,
};

void aoi_message_cb(AOI::AOI_Type type, AOI::ObjectInfo watcher, AOI::ObjectInfo marker, void* pcb_data)
{
	/*
	static bool is_delete = true;
	AOI* aoi_module = static_cast<AOI*>(pcb_data);
	if (is_delete)
	{
		aoi_module->AOI_Update(6, "d", A2DVECTOR(1, 4));
		is_delete = false;
	}
	*/
	assert(marker.type != AREA);

	if (type == AOI::OBJ_ENTER)
	{
		if (watcher.type == AREA)
			fprintf(stderr, "object%lld go into area%lld vision ++++\n", marker.id, watcher.id);
		else
			fprintf(stderr, "object%lld go into object%lld's field of vision ++\n", marker.id, watcher.id);
	}
	else if (type == AOI::OBJ_LEAVE)
	{
		if (watcher.type == AREA)
			fprintf(stderr, "object%lld leave area%lld vision ----\n", marker.id, watcher.id);
		else
			fprintf(stderr, "object%lld leave object%lld's field of vision --\n", marker.id, watcher.id);
	}
	else
	{
		fprintf(stderr, "error happen!\n");
	}
}

A2DVECTOR moveto(int64_t id, A2DVECTOR& cur_pos, const A2DVECTOR& step)
{
	A2DVECTOR tmp;
	tmp += cur_pos;
	tmp += step;
	printf("object%lld -> (%f,%f)\n", id, tmp.x, tmp.y);
	return tmp;
}

int64_t object1 = 1;
int64_t object2 = 2;
int64_t object3 = 301;
A2DVECTOR obj1_init_pos(2, 0);
A2DVECTOR obj2_init_pos(2, 15);

AtomicInt64 obj_x;

void* StartThread(void* obj)
{
	printf("thread start!\n");
	AOI* aoi_module = static_cast<AOI*>(obj);
	int64_t object_id = obj_x.increment_and_get();
	fprintf(stderr, "thread object_id .................%lld\n", object_id);
	while (true)
	{
		//aoi_module->AOI_Message(aoi_message_cb);
		aoi_module->InsertAnnulusObj(object_id, PLAYER, "wm", obj2_init_pos);
		usleep(50000 + object_id*1000);
		aoi_module->AOI_Update(object_id, "d", A2DVECTOR(1, 4));
		//usleep(150000);
		usleep(100000 + object_id*1000);
	}
	return NULL;
}


int main()
{
	//int64_t object1 = 1;
	//int64_t object2 = 2;

	//A2DVECTOR obj1_init_pos(0, 0);
	//A2DVECTOR obj2_init_pos(0, 9);
	A2DVECTOR step(0, -1);

	obj_x.get_and_set(10);
	AOI aoi_module;
	//aoi_module.InsertAnnulusObj(object1, MONSTER, "wm", obj1_init_pos);
	//aoi_module.InsertAnnulusObj(object2, PLAYER, "wm", obj2_init_pos);
	//aoi_module.AOI_Message(aoi_message_cb);
	/*
	pthread_t pthreadId;
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	pthread_create(&pthreadId, NULL, StartThread, &aoi_module);
	*/

	A2DVECTOR old_pos = obj2_init_pos;
	aoi_module.InsertAnnulusObj(object1, MONSTER, "wm", obj1_init_pos);
	aoi_module.InsertAnnulusObj(object2, PLAYER, "wm", obj2_init_pos);

	CGLib::Point2d p1(1, 1);
	CGLib::Point2d p2(3, 1); 
	CGLib::Point2d p3(3, 5);
	CGLib::Point2d p4(2, 4);
	CGLib::Point2d p5(1, 5);
	std::vector<CGLib::Point2d> tmp_vec;
	tmp_vec.push_back(p1);
	tmp_vec.push_back(p2);
	tmp_vec.push_back(p3);
	tmp_vec.push_back(p4);
	tmp_vec.push_back(p5);
	aoi_module.InsertPolygonArea(object3, AREA, tmp_vec);

	//static int is_deleted = 0;
	while(true)
	{
		A2DVECTOR target = moveto(object2, old_pos, step);
		/*
		if (target.y < -10)
			step.y = 1;
		else if (target.y > 10)
			step.y = -1;
			*/
		/*
		if (target.y > 13 && target.y < 22)
		{
			if (is_deleted == 0)
			{
				is_deleted = 1;
				aoi_module.AOI_Update(object2, "d", target);
			}
		}
		else
		*/
		//{
			//aoi_module.AOI_Update(object2, "wm", target);
		//}
		//aoi_module.AOI_Message(aoi_message_cb);

		/*
		aoi_module.InsertAnnulusObj(object1, MONSTER, "wm", obj1_init_pos);
		sleep(1);
		aoi_module.InsertAnnulusObj(object2, PLAYER, "wm", obj2_init_pos);

		//old_pos = target;
		sleep(1);

		A2DVECTOR target;
		aoi_module.AOI_Update(object1, "d", target);
		sleep(1);
		aoi_module.AOI_Update(object2, "d", target);
		sleep(1);

		printf("\n\n");
		*/
	
		//usleep(100000);
		aoi_module.AOI_Update(object2, "wm", target);
		aoi_module.AOI_Message(aoi_message_cb, &aoi_module);
		//usleep(100000);
		sleep(1);
		old_pos = target;
	}

	return 0;
}
