#include "player.h"

#include "gs/global/game_util.h"

#include "subsys_if.h"
#include "player_sender.h"


namespace gamed {

int Player::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_OBJ_ENTER_VIEW:
			{
				if (msg.source != world_xid())
				{
					return 0;
				}

				CHECK_CONTENT_PARAM(msg, msg_obj_enter_view);
				msg_obj_enter_view& param = *(msg_obj_enter_view*)msg.content;
				if (param.object.type == XID::TYPE_PLAYER)
				{
					ASSERT(param.object != object_xid());
					// AOI通知则一定会进else player set，这样是为了和Leave View成对出现
					int32_t linkid, sid;
					unpack_linkid_sid(param.param, linkid, sid);
					ElsePlayerEnterView(param.object.id, linkid, sid);
				}
				else if (param.object.IsNpc() || param.object.IsMatter())
				{
					// Do nothing!
				}
				else
				{
					ASSERT(false);
				}
			}
			break;

		case GS_MSG_OBJ_LEAVE_VIEW:
			{
				if (msg.source != world_xid())
				{
					return 0; 
				}

				CHECK_CONTENT_PARAM(msg, msg_obj_leave_view);
				msg_obj_leave_view& param = *(msg_obj_leave_view*)msg.content;
				if (param.object.type == XID::TYPE_PLAYER)
				{
					ElsePlayerLeaveView(param.object.id);
				}
				else if (param.object.IsNpc() || param.object.IsMatter())
				{
					XID who = param.object;
					sender_->ObjectLeaveView(param.object.id);
				}
				else
				{
					ASSERT(false);
				}
			}
			break;
		
		case GS_MSG_ENTER_RULES_AREA:
			{
				int area_elem_id = msg.param;
				ASSERT(area_elem_id != 0);
				
				// refresh
				RefreshAreaRulesInfo(area_elem_id, true);
				sender_->EnterRuleArea(area_elem_id);
			}
			break;

		case GS_MSG_LEAVE_RULES_AREA:
			{
				int area_elem_id = msg.param;
				ASSERT(area_elem_id != 0);

				// refresh
				RefreshAreaRulesInfo(area_elem_id, false);
				sender_->LeaveRuleArea(area_elem_id);
			}
			break;

		case GS_MSG_ERROR_MESSAGE:
			{
				sender_->ErrorMessage(msg.param);
			}
			break;

		case GS_MSG_QUERY_EXTPROP:
			{   
				CHECK_CONTENT_PARAM(msg, msg_query_extprop);
				msg_query_extprop* param = (msg_query_extprop*)(msg.content);
				sender_->ElsePlayerQueryExtProp(param->requester, param->link_id, param->sid_in_link);
			}
			break;

		case GS_MSG_QUERY_EQUIPCRC:
			{   
				CHECK_CONTENT_PARAM(msg, msg_query_equipcrc);
				msg_query_equipcrc* param = (msg_query_equipcrc*)(msg.content);
				sender_->ElsePlayerQueryEquipCRC(param->requester, param->link_id, param->sid_in_link);
			}
			break;

		case GS_MSG_QUERY_EQUIPMENT:
			{   
				CHECK_CONTENT_PARAM(msg, msg_query_equipment);
				msg_query_equipment* param = (msg_query_equipment*)(msg.content);
				sender_->ElsePlayerQueryEquipment(param->requester, param->link_id, param->sid_in_link);
			}
			break;

		case GS_MSG_GET_STATIC_ROLE_INFO:
			{
				CHECK_CONTENT_PARAM(msg, msg_get_static_role_info);
				msg_get_static_role_info* param = (msg_get_static_role_info*)(msg.content);
				sender_->ElsePlayerQueryStaticRoleInfo(param->requester, param->link_id, param->sid_in_link);
			}
			break;

		case GS_MSG_WORLD_CLOSING:
			{
				if (msg.source != world_xid())
				{
					ASSERT(false);
					return 0;
				}

				HandleWorldClose(msg);
			}
			break;

		case GS_MSG_SHOP_SHOPPING_FAILED:
			{
				CHECK_CONTENT_PARAM(msg, msg_shop_shopping_failed);
				msg_shop_shopping_failed* param = (msg_shop_shopping_failed*)(msg.content);
				sender_->ShopShoppingFailed(param->err_code, param->goods_id, param->goods_count, param->goods_remains);
			}
			break;
            
        case GS_MSG_ENHANCE_FAILED:
            {
				CHECK_CONTENT_PARAM(msg, msg_enhance_failed);
				msg_enhance_failed* param = (msg_enhance_failed*)(msg.content);
				sender_->EnhanceReply(param->err_code, -1, 0, 0);
            }
            break;

	    default:
		    if (Unit::MessageHandler(msg) != 0 && subsys_if_->DispatchMsg(msg) != 0 && pimp_->OnMessageHandler(msg) != 0)
			{
				ASSERT(false && "无法处理未知类型的inter-message");
				return -1;
			}
			return 0;
	}

	return 0;
}

} // namespace gamed
