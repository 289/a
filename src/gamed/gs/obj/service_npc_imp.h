#ifndef GAMED_GS_OBJ_SERVICE_NPC_IMP_H_
#define GAMED_GS_OBJ_SERVICE_NPC_IMP_H_

#include "gs/obj/service/service_man.h"

#include "npc.h"


namespace gamed {

/**
 * @brief ServiceNpcImp
 *    （1）不要直接访问Npc的成员变量（虽然是friend class）
 *         可以通过调用Npc的public及protected函数来实现
 */
class ServiceNpcImp : public NpcImp
{
public:
	ServiceNpcImp(Npc& npc);
	virtual ~ServiceNpcImp();

	virtual bool OnInit();
	virtual void OnHeartbeat();
	virtual int  OnMessageHandler(const MSG& msg);
    virtual bool OnMapTeleport();

private:
	ProviderList service_list_;
};

} // namespace gamed

#endif // GAMED_GS_OBJ_SERVICE_NPC_IMP_H_
