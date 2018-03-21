#ifndef GAMED_GS_PLAYER_PLAYER_STATE_H_
#define GAMED_GS_PLAYER_PLAYER_STATE_H_

#include <stdint.h>


namespace gamed {

class Player;
class PlayerMsgDispatcher;

/**
 * @brief PlayerState
 */
class PlayerState
{
public:
	typedef uint32_t MaskType;

	// 状态的定义修改后（包括调整顺序），注意需要修改InterMsgFilter里的相关静态变量:
	// s_special_filter_，player_msg_state_table等
	enum
	{
		STATE_NORMAL,       // 正常状态
		STATE_OFFLINE,      // 离线状态
		STATE_DEAD,         // 死亡状态
		STATE_COMBAT,       // 战斗状态
		STATE_COUNT,        // 真正状态的数目
	};

	PlayerState();
	~PlayerState();

	/**
	 * @brief Release 
	 *     player下线会调用这个函数，重置state
	 */
	inline void Release();

	/* GET FUNCTION */
	inline MaskType state() const;
	inline bool CanSave() const;
	inline bool CanSwitch() const;
	inline bool CanCombat() const;
	inline bool CanGather() const;
	inline bool IsNormal() const;
	inline bool IsOffline() const;
	inline bool IsDead() const;
	inline bool IsCombat() const;

	/* SET FUNCTION */
	inline void SetNormal();
	inline void SetOffline();
	inline void SetDead();
	inline void SetCombat();


private:
	MaskType    state_;
};

///
/// inline func
///
inline void PlayerState::Release()
{
	state_ = STATE_NORMAL;
}

inline PlayerState::MaskType PlayerState::state() const
{
	return state_;
}

inline bool PlayerState::CanSave() const
{
	return state_ == STATE_NORMAL ||
		   state_ == STATE_DEAD ||
		   state_ == STATE_COMBAT;
}

inline bool PlayerState::CanSwitch() const
{
	return state_ == STATE_NORMAL;
}

inline bool PlayerState::CanCombat() const
{
	return state_ == STATE_NORMAL;
}

inline bool PlayerState::CanGather() const
{
	return state_ == STATE_NORMAL;
}

inline bool PlayerState::IsNormal() const
{
	return state_ == STATE_NORMAL;
}

inline bool PlayerState::IsOffline() const
{
	return state_ == STATE_OFFLINE;
}

inline bool PlayerState::IsDead() const
{
	return state_ == STATE_DEAD;
}

inline bool PlayerState::IsCombat() const
{
	return state_ == STATE_COMBAT;
}

inline void PlayerState::SetNormal()
{
	if (IsOffline()) 
		return;

	state_ = STATE_NORMAL;
}

inline void PlayerState::SetDead()
{
	if (IsOffline()) 
		return;

	state_ = STATE_DEAD;
}

inline void PlayerState::SetOffline()
{
	state_ = STATE_OFFLINE;
}

inline void PlayerState::SetCombat()
{
	if (IsOffline()) 
		return;

	state_ = STATE_COMBAT;
}

} // namespace gamed

#endif // GAMED_GS_PLAYER_PLAYER_STATE_H_
