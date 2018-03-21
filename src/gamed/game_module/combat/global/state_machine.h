#ifndef __GAME_MODULE_COMBAT_GLOBAL_FSM_H__
#define __GAME_MODULE_COMBAT_GLOBAL_FSM_H__

#include "state.h"

#include <vector>
#include <map>

namespace combat
{

/**
 * @class ShiftNode
 * @brief 状态转换节点
 * @brief 源状态在事件驱动下过渡到目标状态
 */
struct ShiftNode
{
	STATUS cur;
	EVENT  event;
	STATUS next;
};


/**
 * @class FSM: finite-state-machine
 * @brief 有限状态机
 */
template <class T>
class FSM
{
private:
	typedef std::vector<XState<T> *> StateVec;
	StateVec state_stubs_;

	typedef std::map<STATUS/*cur*/, STATUS/*next*/> StatusPairMap;
	typedef std::vector<StatusPairMap> StateShiftVec;
	StateShiftVec state_shift_table_;

public:
	FSM() {}
	virtual ~FSM()
	{
		for (size_t i = 0; i < state_stubs_.size(); ++ i)
		{
			delete state_stubs_[i];
			state_stubs_[i] = NULL;
		}

		state_stubs_.clear();
		state_shift_table_.clear();
	}

    void Init(int max_status, int max_event)
    {
		assert(max_status > 0);
		assert(max_event > 0);
		state_stubs_.resize(max_status);
		state_shift_table_.resize(max_event);
    }

	bool RegisterState(XState<T>* state)
	{
		STATUS status = state->GetStatus();
		if (status < 0 || status >= (int)state_stubs_.size())
		{
			assert(false);
			return false;
		}

		if (state_stubs_[status] != NULL)
		{
			assert(false);
			return false;
		}

		state_stubs_[status] = state;
		return true;
	}

    XState<T>* QueryState(STATUS status)
    {
        return state_stubs_[status];
    }

	bool InitStateShiftTable(const ShiftNode* array, size_t size)
	{
		for (size_t i = 0; i < size; ++ i)
		{
			const ShiftNode& node = array[i];

			EVENT  event = node.event;
			STATUS cur   = node.cur;
			STATUS next  = node.next;

			if (event > (int)state_shift_table_.size())
			{
				assert(false);
				return false;
			}

			StatusPairMap& map = state_shift_table_[event];
			if (map.find(cur) != map.end())
			{
				assert(false);
				return false;
			}

			map[cur] = next;
		}
		return true;
	}

	bool Shift(XState<T>*& state, EVENT event, T* obj, int timeout) const
	{
		if (event < 0 || event >= (int)state_shift_table_.size())
		{
			assert(false);
			return false;
		}

		STATUS cur = state->GetStatus();

		const StatusPairMap& map = state_shift_table_[event];
		StatusPairMap::const_iterator it = map.find(cur);
		if (it == map.end())
		{
			assert(false);
			return false;
		}

		STATUS next = it->second;
        if (next == cur)
        {
            if (timeout > state->GetTimeOut())
            {
                state->SetTimeOut(timeout);
            }
            state->OnLeave(obj);
            state->OnEnter(obj);
            return true;
        }
		
        state->OnLeave(obj);
        delete state;
        state = NULL;

		XState<T>* __state = state_stubs_[next]->Clone();
		if (!__state)
		{
			return false;
		}

		state = __state;
        state->SetTimeOut(timeout);
		state->OnEnter(obj);
		return true;
	}
};


///
/// macros define
///
#define DECLARE_STATE(status, state_class_name, obj_class_name) \
		state_class_name(OPT* first, size_t n) : XState<obj_class_name>(status, first, n) \
		{ \
		} \
		virtual ~state_class_name() \
		{ \
		} \
		virtual XState<obj_class_name>* Clone() const \
		{ \
			return new state_class_name(*this); \
		} \


#define REGISTER_STATE(state, fsm) \
	{ \
		fsm.RegisterState(state); \
	}

};

#endif // __GAME_MODULE_COMBAT_GLOBAL_FSM_H__
