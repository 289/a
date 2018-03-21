#include "player_visible_state.h"

#include "shared/net/packet/bytebuffer.h"


namespace gamed {

using namespace shared::net;

#define MARSHAL_PARAM(state_mask, param) \
	if (state_ & state_mask) \
	{ \
		buf << param; \
	}

#define UNMARSHAL_PARAM(state_mask, param) \
	if (state_ & state_mask) \
	{ \
		buf >> param; \
	}

///
/// PlayerVisibleState
///
PlayerVisibleState::PlayerVisibleState()
	: state_(0)
{
}

PlayerVisibleState::~PlayerVisibleState()
{
}

void PlayerVisibleState::Release()
{
	state_ = 0;
}

///
/// Marshal 和 Unmarshal 对应成员变量的顺序要一致
///
void PlayerVisibleState::Pack(ByteBuffer& buf)
{
	buf << state_;

	// 0
	MARSHAL_PARAM(PVS_COMBAT, combat_param_);
	MARSHAL_PARAM(PVS_TEAM, team_param_);
	MARSHAL_PARAM(PVS_DEAD, none_param_);
	MARSHAL_PARAM(PVS_BUFF, buff_param_);
	MARSHAL_PARAM(PVS_TITLE, title_param_);
	MARSHAL_PARAM(PVS_MOUNT, mount_param_);
}

void PlayerVisibleState::UnPack(ByteBuffer& buf)
{
	buf >> state_;

	// 0
	UNMARSHAL_PARAM(PVS_COMBAT, combat_param_);
	UNMARSHAL_PARAM(PVS_TEAM, team_param_);
	UNMARSHAL_PARAM(PVS_DEAD, none_param_);
	UNMARSHAL_PARAM(PVS_BUFF, buff_param_);
	UNMARSHAL_PARAM(PVS_TITLE, title_param_);
	UNMARSHAL_PARAM(PVS_MOUNT, mount_param_);
}


///
/// PVStateWrapper
///
#define REGISTER_PARAM(type, param) \
    { \
        std::pair<MaskToParamMap::iterator, bool> ret; \
        ret = mask_mapto_param_.insert(std::pair<PVState::PVS_MASK, PVState::pvs_base_param*>(PVState::type, &(pv_state_.param))); \
        assert(ret.second); (void)ret; \
    }

PVStateWrapper::PVStateWrapper(const PVState& pvs)
	: pv_state_(pvs)
{
	REGISTER_PARAM(PVS_COMBAT, combat_param_);
	REGISTER_PARAM(PVS_TEAM, team_param_);
	REGISTER_PARAM(PVS_DEAD, none_param_);
	REGISTER_PARAM(PVS_BUFF, buff_param_);
	REGISTER_PARAM(PVS_TITLE, title_param_);
	REGISTER_PARAM(PVS_MOUNT, mount_param_);
}

PVStateWrapper::~PVStateWrapper()
{
	pv_state_.Release();
}

} // namespace gamed
