#include "service_npc_imp.h"

#include "shared/logsys/logging.h"
#include "gamed/client_proto/G2C_error.h"
#include "gs/obj/service/service_provider.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/service_npc_templ.h"

#include "npc_def.h"
#include "ainpc.h"


namespace gamed {

using namespace dataTempl;
using namespace npcdef;

ServiceNpcImp::ServiceNpcImp(Npc& npc)
	: NpcImp(npc),
	  service_list_(kMaxServiceProviderPerNpc)
{
}

ServiceNpcImp::~ServiceNpcImp()
{
}

bool ServiceNpcImp::OnInit()
{
	const ServiceNpcTempl* ptmpl = s_pDataTempl->QueryDataTempl<ServiceNpcTempl>(npc_.templ_id());
	if (ptmpl == NULL)
	{
		LOG_ERROR << "ServiceNpcImp::OnInit() failure! reason:template not found, "
			<< "or tamplate type invalid! templ_id" << npc_.templ_id();
		return false;
	}

	// 以下代码如果有错误不能直接return，因为要删除service_vec
	bool bRst = true;
	std::vector<serviceDef::ServiceTempl*> service_vec;
	if (!serviceDef::ParseBuffer(ptmpl->service_content, service_vec))
	{
		LOG_ERROR << "ServiceNpcImp::OnInit() error! elem_id:" << npc_.elem_id() 
			<< "templ_id:" << npc_.templ_id(); 
		bRst = false;
	}
	else // parse success
	{
		for (size_t i = 0; i < service_vec.size(); ++i)
		{
			int service_type = service_vec[i]->GetType();
			ServiceProvider* provider = ServiceManager::CreateProvider(service_type);
			if (provider != NULL) 
			{
				// if success
				if (provider->Init(&npc_, service_vec[i]))
				{
					if (service_list_.AddProvider(provider))
					{
						continue;
					}
				}

				LOG_ERROR << "ServiceNpcImp::OnInit() error! Provider Init() or AddProvider() error! elem_id:" 
					<< npc_.elem_id() << "templ_id:" << npc_.templ_id() << "service_type:" << service_type; 

				// error
				bRst = false;
				break;
			}
			else // error
			{
				bRst = false;
				break;
			}
		}
	}

	// delete all ServiceTempl
	for (size_t i = 0; i < service_vec.size(); ++i)
	{
		DELETE_SET_NULL(service_vec[i]);
	}
	return bRst;
}

void ServiceNpcImp::OnHeartbeat()
{
}

bool ServiceNpcImp::OnMapTeleport()
{
    return true;
}

int ServiceNpcImp::OnMessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_SERVICE_HELLO:
			{
				ASSERT(msg.source.IsPlayer());
				if (msg.pos.squared_distance(npc_.pos()) < kNpcMaxServiceDisSquare)
				{
					int player_faction = msg.param;
					if (!(player_faction & GetEnemyFaction()))
					{
						// trigger aicore
						npc_.aicore()->SomeoneGreeting(msg.source);
						npc_.SendMsg(GS_MSG_SERVICE_GREETING, msg.source);
					}
				}
			}
			break;

		case GS_MSG_SERVICE_REQUEST:
			{
				ASSERT(msg.source.IsPlayer());
				if (msg.pos.squared_distance(npc_.pos()) > kNpcMaxServiceDisSquare)
				{
					npc_.SendMsg(GS_MSG_ERROR_MESSAGE, msg.source, G2C::ERR_SERVICE_UNAVILABLE);
					break;
				}
				// 服务的请求到来
				ServiceProvider* provider = service_list_.GetProvider(msg.param);
				if (provider != NULL)
				{
					provider->PayService(msg.source, msg.content, msg.content_len);
				}
				else
				{
					npc_.SendMsg(GS_MSG_ERROR_MESSAGE, msg.source, G2C::ERR_SERVICE_UNAVILABLE);
				}
			}
			break;

		case GS_MSG_SERVICE_QUERY_CONTENT:
			{
				ASSERT(msg.source.IsPlayer());
				if (msg.pos.squared_distance(npc_.pos()) > kNpcMaxServiceDisSquare)
				{
					npc_.SendMsg(GS_MSG_ERROR_MESSAGE, msg.source, G2C::ERR_SERVICE_UNAVILABLE);
					break;
				}

				ServiceProvider* provider = service_list_.GetProvider(msg.param);
				if (provider != NULL)
				{
					const msg_service_query_content* buf = (const msg_service_query_content*)(msg.content);
					provider->GetServiceContent(msg.source, buf->link_sid, buf->client_sid);
				}
				else
				{
					npc_.SendMsg(GS_MSG_ERROR_MESSAGE, msg.source, G2C::ERR_SERVICE_UNAVILABLE);
				}
			}
			break;

		default:
			ASSERT(false);
			return -1;
	}

	return 0;
}

} // namespace gamed
