#include "object.h"

#include "shared/base/base_define.h"

#include "gs/global/dbgprt.h"
#include "gs/global/gmatrix.h"
#include "gs/scene/world.h"


namespace gamed {

Object::Object()
{
}

Object::~Object()
{
}

WorldObject::WorldObject()
	: world_id_(-1),
	  world_tag_(-1),
	  xid_(-1, XID::TYPE_INVALID),
	  plane_(NULL),
	  is_actived_(false)
{
}

WorldObject::~WorldObject()
{
	
}

void WorldObject::Lock() 
{ 
	mutex_.lock(); 
}

void WorldObject::Unlock()
{
	mutex_.unlock(); 
}

bool WorldObject::IsLocked()
{
	return mutex_.IsLockedByThisThread();
}

void WorldObject::Release()
{
	world_id_   = -1;
	world_tag_  = -1;
	pos_.x      = 0;
	pos_.y      = 0;
	dir_        = 0;
	xid_.id     = -1;
	xid_.type   = XID::TYPE_INVALID;
	plane_      = NULL;
	is_actived_ = false;
}

bool WorldObject::IsActived() const
{
	return is_actived_;
}

void WorldObject::SetActive()
{
	is_actived_ = true;
}

void WorldObject::ClrActive()
{
	is_actived_ = false;
}

const XID& WorldObject::world_xid() const
{
	return plane_->world_xid();
}

int WorldObject::DispatchMessage(const MSG& msg)
{
	return OnDispatchMessage(msg);
}

void WorldObject::DoHeartbeat()
{
	OnHeartbeat();
}

int WorldObject::MessageHandler(const MSG& msg)
{
	switch (msg.message)
	{
		case GS_MSG_OBJ_HEARTBEAT:
			{
				DoHeartbeat();
			}
			break;

		default:
			return -1;
	}

	return 0;
}

void WorldObject::SendMsg(int message, const XID& target, int64_t param)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), param, NULL, 0, pos());
	Gmatrix::SendObjectMsg(msg);
}
	
void WorldObject::SendMsg(int message, const XID& target, const void* buf, size_t len)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), 0, buf, len, pos());
	Gmatrix::SendObjectMsg(msg);
}

void WorldObject::SendMsg(int message, const XID& target, int64_t param, const void* buf, size_t len)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), param, buf, len, pos());
	Gmatrix::SendObjectMsg(msg);
}

void WorldObject::SendPlaneMsg(int message, int64_t param)
{
	if (world_plane() != NULL && world_plane()->world_xid().IsWorld())
	{
		SendPlaneMsg(message, world_plane()->world_xid(), param);
	}
}

void WorldObject::SendPlaneMsg(int message, const void* buf, size_t len)
{
	if (world_plane() != NULL && world_plane()->world_xid().IsWorld())
	{
		SendPlaneMsg(message, world_plane()->world_xid(), buf, len);
	}
}

void WorldObject::SendPlaneMsg(int message, int64_t param, const void* buf, size_t len)
{
	if (world_plane() != NULL && world_plane()->world_xid().IsWorld())
	{
		SendPlaneMsg(message, world_plane()->world_xid(), param, buf, len);
	}
}

void WorldObject::SendPlaneMsg(int message, const XID& target, int64_t param)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), param, NULL, 0, pos());
	Gmatrix::SendWorldMsg(msg);
}

void WorldObject::SendPlaneMsg(int message, const XID& target, const void* buf, size_t len)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), 0, buf, len, pos());
	Gmatrix::SendWorldMsg(msg);
}

void WorldObject::SendPlaneMsg(int message, const XID& target, int64_t param, const void* buf, size_t len)
{
	MSG msg;
	BuildMessage(msg, message, target, object_xid(), param, buf, len, pos());
	Gmatrix::SendWorldMsg(msg);
}

} // namespace gamed
