#ifndef GAMED_CLIENT_PROTO_PLAYER_VISIBLE_STATE_H_
#define GAMED_CLIENT_PROTO_PLAYER_VISIBLE_STATE_H_

#include <vector>
#include "shared/base/noncopyable.h"
#include "shared/base/copyable.h"
#include "shared/net/packet/packet_util.h"


namespace shared 
{
	namespace net 
	{
		class ByteBuffer;
	} // net
} // shared


namespace gamed {

class PVStateWrapper;
class PlayerVisibleState : public shared::copyable
{
	friend class PVStateWrapper;
	typedef int32_t CombatID;
	typedef int32_t TeamID;
	typedef int32_t BuffID;
	typedef int32_t TitleID;
	typedef int32_t MountID;

public:
	enum PVS_MASK
	{
		PVS_COMBAT     = 0x0001,
		PVS_TEAM       = 0x0002,
		PVS_DEAD       = 0x0004,
		PVS_BUFF       = 0x0008,
        PVS_TITLE      = 0x0010,
        PVS_MOUNT      = 0x0020,
	};

    ///
    /// buff相关定义
    ///
    enum BuffType
    {
        BT_SYS_BUFF = 0,  // 由系统发的buff，比如登陆的无敌
        BT_WORLD_BUFF,    // 大世界buff，由策划配置的buff
    };

    enum SysBuffType
    {
        SBT_INVINCIBLE = 1, // 无敌
    };

    struct BuffInfo
    {
        int8_t  type; // 对应BuffType
        int32_t id;   // 如果是系统发的buff对应SysBuffType枚举，大世界则是对应的效果id
        int32_t timeout; // 超时时间，单位秒，如果是0表示无限
        NESTED_DEFINE(type, id, timeout);
    };


public:
	PlayerVisibleState();
	~PlayerVisibleState();

	void Release();
	void Pack(shared::net::ByteBuffer& buf);
	void UnPack(shared::net::ByteBuffer& buf);
	
	// combat state ---- pvs_combat
	inline void SetCombatFlag(CombatID combat_id);
	inline void ClrCombatFlag();
	inline bool IsInCombat() const;

	// team state ---- pvs_team
	inline void SetTeamFlag(TeamID team_id);
	inline void ClrTeamFlag();
	inline bool IsInTeam() const;

	// dead state ---- pvs_none_param
	inline void SetDeadFlag();
	inline void ClrDeadFlag();
	inline bool IsDead() const;

	// invincible state
	inline void SetBuffList(const std::vector<BuffInfo>& buff_vec);

    // title state ---- pvs_title
    inline void SetTitleFlag(int32_t title_id);

    // mount state ---- pvs_mount
    inline void SetMountFlag(int32_t mount_id);

public:
	struct pvs_base_param
	{ 
		virtual ~pvs_base_param() { }
	};

	struct pvs_none_param : public pvs_base_param
	{
		NESTED_DEFINE();
	};

	struct pvs_combat : public pvs_base_param
	{
		CombatID combat_id;
		NESTED_DEFINE(combat_id);
	};

	struct pvs_team : public pvs_base_param
	{
		TeamID team_id;
		NESTED_DEFINE(team_id);
	};

	struct pvs_buff : public pvs_base_param
	{
		std::vector<BuffInfo> buff_vec;
		NESTED_DEFINE(buff_vec);
	};

    struct pvs_title : public pvs_base_param
    {
        TitleID title_id;
        NESTED_DEFINE(title_id);
    };

    struct pvs_mount : public pvs_base_param
    {
        MountID mount_id;
        NESTED_DEFINE(mount_id);
    };


private:
	uint64_t       state_;
	pvs_none_param none_param_; // 不受顺序影响，可以多次使用

	// 状态的参数要按照一定的顺序，才能保证Marshal、Unmarshal是正确的
	// 定义的参数需要在cpp里REGISTER_PARAM注册，把Mask和对应的Param关联在一起，
	// 因为客户端需要使用Wrapper操作
	// 0
	pvs_combat     combat_param_;
	pvs_team       team_param_;
	pvs_buff       buff_param_;
    pvs_title      title_param_;
    pvs_mount      mount_param_;
};

///
/// inline function
///
inline void PlayerVisibleState::SetCombatFlag(CombatID combat_id)
{
	state_ |= PVS_COMBAT;
	combat_param_.combat_id = combat_id;
}

inline void PlayerVisibleState::ClrCombatFlag()
{
	state_ &= ~PVS_COMBAT;
	combat_param_.combat_id = 0;
}

inline bool PlayerVisibleState::IsInCombat() const
{
	return state_ & PVS_COMBAT;
}

inline void PlayerVisibleState::SetTeamFlag(TeamID team_id)
{
	state_ |= PVS_TEAM;
	team_param_.team_id = team_id;
}

inline void PlayerVisibleState::ClrTeamFlag()
{
	state_ &= ~PVS_TEAM;
	team_param_.team_id = 0;
}

inline bool PlayerVisibleState::IsInTeam() const
{
	return state_ & PVS_TEAM;
}

inline void PlayerVisibleState::SetDeadFlag()
{
	state_ |= PVS_DEAD;
}

inline void PlayerVisibleState::ClrDeadFlag()
{
	state_ &= ~PVS_DEAD;
}

inline bool PlayerVisibleState::IsDead() const
{
	return state_ & PVS_DEAD;
}

inline void PlayerVisibleState::SetBuffList(const std::vector<BuffInfo>& buff_vec)
{
	if (buff_vec.empty())
		state_ &= ~PVS_BUFF;
	else
		state_ |= PVS_BUFF;

	buff_param_.buff_vec = buff_vec;
}

inline void PlayerVisibleState::SetTitleFlag(int32_t title_id)
{
    if (title_id > 0)
    {
        state_ |= PVS_TITLE;
        title_param_.title_id = title_id;
    }
    else
    {
        state_ &= ~PVS_TITLE;
        title_param_.title_id = 0;
    }
}

inline void PlayerVisibleState::SetMountFlag(int32_t mount_id)
{
    if (mount_id > 0)
    {
        state_ |= PVS_MOUNT;
        mount_param_.mount_id = mount_id;
    }
    else
    {
        state_ &= ~PVS_MOUNT;
        mount_param_.mount_id = 0;
    }
}


///
/// PVStateWrapper
///
class PVStateWrapper : shared::noncopyable
{
	typedef PlayerVisibleState PVState;
public:
	PVStateWrapper(const PVState& pvs);
	~PVStateWrapper();

	template <typename T>
	bool ForStateParam(PVState::PVS_MASK type, T& param);


private:
	typedef std::map<PVState::PVS_MASK, PVState::pvs_base_param*> MaskToParamMap;
	MaskToParamMap mask_mapto_param_;

	PlayerVisibleState pv_state_;
};

///
/// template function
///
template <typename T>
bool PVStateWrapper::ForStateParam(PVState::PVS_MASK type, T& param)
{
	if (type & pv_state_.state_)
	{
		MaskToParamMap::const_iterator it = mask_mapto_param_.find(type);
		if (it == mask_mapto_param_.end())
		{
			return false;
		}

		try
		{
			T* tmp = dynamic_cast<T*>(it->second);
			if (tmp == NULL)
			{
				return false;
			}
			param  = *tmp;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	return false;
}

} // namespace gamed

#endif // GAMED_CLIENT_PROTO_PLAYER_VISIBLE_STATE_H_
