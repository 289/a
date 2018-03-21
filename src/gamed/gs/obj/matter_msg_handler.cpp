#include "matter.h"

#include "gs/global/game_util.h"
#include "gs/scene/world.h"


namespace gamed {

int Matter::MessageHandler(const MSG& msg)
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
					int32_t linkid, sid;
					unpack_linkid_sid(param.param, linkid, sid);
					PlayerEnterView(param.object.id, linkid, sid);
				}
				else if (param.object.IsMatter())
				{
					ASSERT(param.object != object_xid());
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
					PlayerLeaveView(param.object.id);
				}
				else if (param.object.IsMatter())
				{
					ASSERT(param.object != object_xid());
				}
			}
			break;

		case GS_MSG_WORLD_CLOSING:
			{
				if (msg.source != world_xid())
				{
					ASSERT(false);
					return 0;
				}

				HandleWorldClose();
			}
			break;

		case GS_MSG_RELATE_MAP_ELEM_CLOSED:
			{
				if (msg.source != world_xid())
				{
					ASSERT(false);
					return 0;
				}

				RecycleSelf();
			}
			break;

        case GS_MSG_OBJECT_TELEPORT:
            {
                if (msg.source != world_xid())
                    return 0;

                HandleObjectTeleport(msg);
            }
            break;

		default:
		    if (WorldObject::MessageHandler(msg) != 0 && pimp_->OnMessageHandler(msg) != 0)
			{
				ASSERT(false && "无法处理未知类型的inter-message");
				return -1;
			}
			return 0;
	}

	return 0;
}

} // namespace gamed
