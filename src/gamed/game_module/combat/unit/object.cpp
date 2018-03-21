#include "object.h"
#include "combat_man.h"

namespace combat
{

void Object::SendMSG(const MSG& msg, size_t tick_latency)
{
	s_pCombatMan->SendMSG(msg, tick_latency);
}

void Object::SendMSG(int message, const XID& target, int64_t param, const void* buf, size_t len)
{
    MSG msg;
    BuildMessage(msg, message, target, xid_, param, buf, len);
    SendMSG(msg);
}

};
