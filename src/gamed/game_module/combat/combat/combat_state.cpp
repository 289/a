#include "combat_state.h"
#include "combat.h"
#include "combat_scene_ai.h"

namespace combat
{

///
/// 战场处于不同状态下支持的操作
///
static OPT opts_closed[]            = {};
static OPT opts_open_ready[]	    = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE, XOPT_SUSPEND};
static OPT opts_running[]           = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE, XOPT_SUSPEND, XOPT_STOP, XOPT_WAIT_SELECT_SKILL, XOPT_WAIT_SELECT_PET};
static OPT opts_suspend[]           = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE, XOPT_SUSPEND};
static OPT opts_stopped[]           = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE};
static OPT opts_close_wait[]        = {XOPT_PLAYER_OPERATE};
static OPT opts_last_wait[]         = {};
static OPT opts_wait_select_skill[] = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE, XOPT_WAIT_SELECT_SKILL};
static OPT opts_wait_select_pet[]   = {XOPT_REGISTER_ATB, XOPT_UNREGISTER_ATB, XOPT_PLAYER_OPERATE, XOPT_SCRIPT_OPERATE, XOPT_WAIT_SELECT_PET};

///
/// 战场状态转移表
///
static ShiftNode combat_state_shift_table[] =
{
	/*cur-status*/                  /*event*/                 /*next-status*/
	{XSTATUS_CLOSED,                XEVENT_INIT,              XSTATUS_OPEN_READY},       // 战场创建时的初始化

    {XSTATUS_OPEN_READY,            XEVENT_TIMEOUT,           XSTATUS_RUNNING},          // 战场准备超时，正式开放战场
    {XSTATUS_OPEN_READY,            XEVENT_SUSPEND,           XSTATUS_SUSPEND},
    {XSTATUS_OPEN_READY,            XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},

	{XSTATUS_RUNNING,               XEVENT_STOP,              XSTATUS_STOPPED},          // 战斗被ATB暂停
	{XSTATUS_RUNNING,               XEVENT_SUSPEND,           XSTATUS_SUSPEND},          // 战斗被脚本暂停
	{XSTATUS_RUNNING,               XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},       // 战斗处于正常状态，战斗结束了，等待战斗对象把动作做完。
	{XSTATUS_RUNNING,               XEVENT_CLOSE,             XSTATUS_CLOSED},           // 战场创建时或进行中发生异常，即时关闭战场。
	{XSTATUS_RUNNING,               XEVENT_WAIT_SELECT_SKILL, XSTATUS_WAIT_SELECT_SKILL},
	{XSTATUS_RUNNING,               XEVENT_WAIT_SELECT_PET,   XSTATUS_WAIT_SELECT_PET},

	{XSTATUS_SUSPEND,               XEVENT_SUSPEND,           XSTATUS_SUSPEND},          // 战斗被脚本多次暂停
	{XSTATUS_SUSPEND,               XEVENT_RESUME,            XSTATUS_RUNNING},          // 脚本暂停超时, 并且脚本函数执行完毕，所以这里恢复战斗。
    //{XSTATUS_SUSPEND,               XEVENT_TIMEOUT,           XSTATUS_SUSPEND},          // 脚本暂停超时, 这里不恢复战斗，因为状态超时继续执行脚本，脚本可能再次暂停战斗，所以战斗恢复权交给战场去控制。
    {XSTATUS_SUSPEND,               XEVENT_TIMEOUT,           XSTATUS_RUNNING},          // 脚本暂停超时, 这里不恢复战斗，因为状态超时继续执行脚本，脚本可能再次暂停战斗，所以战斗恢复权交给战场去控制。
	{XSTATUS_SUSPEND,               XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},       // 战斗处于脚本暂停状态，战斗结束了，等待战斗对象把动作做完。

	{XSTATUS_STOPPED,               XEVENT_TIMEOUT,           XSTATUS_RUNNING},          // ATB暂停超时, 战斗恢复
	{XSTATUS_STOPPED,               XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},       // 战斗处于ATB暂停状态，战斗结束了，等待战斗对象把动作做完。这种情况只有在世界BOSS战斗中才会出现。

	{XSTATUS_CLOSE_WAIT,            XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},       // 重复关闭战场，是有可能的，因为战斗正常结束和异常结束的触发点是相互独立的。
	{XSTATUS_CLOSE_WAIT,            XEVENT_CLOSE,             XSTATUS_LAST_WAIT},        // 战场上已经没有人在动了，战斗结束，这里做延迟是为了客户端把可能的死亡效果播放完。

	{XSTATUS_LAST_WAIT,             XEVENT_TIMEOUT,           XSTATUS_CLOSED},           // 战斗结束了, 关闭战场。

	{XSTATUS_WAIT_SELECT_SKILL,     XEVENT_WAIT_SELECT_SKILL, XSTATUS_WAIT_SELECT_SKILL},
	{XSTATUS_WAIT_SELECT_SKILL,     XEVENT_RESUME,            XSTATUS_RUNNING}, 
	{XSTATUS_WAIT_SELECT_SKILL,     XEVENT_TIMEOUT,           XSTATUS_RUNNING},
	{XSTATUS_WAIT_SELECT_SKILL,     XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},

	{XSTATUS_WAIT_SELECT_PET,       XEVENT_WAIT_SELECT_PET,   XSTATUS_WAIT_SELECT_PET},
	{XSTATUS_WAIT_SELECT_PET,       XEVENT_RESUME,            XSTATUS_RUNNING}, 
	{XSTATUS_WAIT_SELECT_PET,       XEVENT_TIMEOUT,           XSTATUS_RUNNING},
	{XSTATUS_WAIT_SELECT_PET,       XEVENT_CLOSING,           XSTATUS_CLOSE_WAIT},
};

///
/// 定义战场状态机
///
FSM<Combat> CombatState::fsm_;

void CombatState::Initialize()
{
    //初始化状态机的基础信息
    fsm_.Init(XSTATUS_MAX, XEVENT_MAX);

	//注册状态到状态机
	REGISTER_STATE(new StateClosed(opts_closed,        sizeof(opts_closed)/sizeof(OPT)),     CombatState::fsm_);
	REGISTER_STATE(new StateOpenReady(opts_open_ready, sizeof(opts_open_ready)/sizeof(OPT)), CombatState::fsm_);
	REGISTER_STATE(new StateRunning(opts_running,      sizeof(opts_running)/sizeof(OPT)),    CombatState::fsm_);
	REGISTER_STATE(new StateStoped(opts_stopped,       sizeof(opts_stopped)/sizeof(OPT)),    CombatState::fsm_);
	REGISTER_STATE(new StateSuspend(opts_suspend,      sizeof(opts_suspend)/sizeof(OPT)),    CombatState::fsm_);
	REGISTER_STATE(new StateCloseWait(opts_close_wait, sizeof(opts_close_wait)/sizeof(OPT)), CombatState::fsm_);
	REGISTER_STATE(new StateLastWait(opts_last_wait,   sizeof(opts_last_wait)/sizeof(OPT)),  CombatState::fsm_);
	REGISTER_STATE(new StateWaitSelectSkill(opts_wait_select_skill,      sizeof(opts_wait_select_skill)/sizeof(OPT)),    CombatState::fsm_);
	REGISTER_STATE(new StateWaitSelectPet(opts_wait_select_pet,      sizeof(opts_wait_select_pet)/sizeof(OPT)),    CombatState::fsm_);

	//初始化状态转移表
	fsm_.InitStateShiftTable(combat_state_shift_table, sizeof(combat_state_shift_table)/sizeof(ShiftNode));
}


/*************************CombatState****************************/
/*************************CombatState****************************/
/*************************CombatState****************************/
/*************************CombatState****************************/
void CombatState::Init()
{
    if (!state_)
    {
        XState<Combat>* state = fsm_.QueryState(XSTATUS_CLOSED);
        if (state)
        {
            state_ = state->Clone();
        }
    }

    Update(XEVENT_INIT, COMBAT_OPEN_READY_TIME / MSEC_PER_TICK);
}

void CombatState::Reset()
{
    Update(XEVENT_INIT, COMBAT_WAVE_READY_TIME / MSEC_PER_TICK);
}

void CombatState::Update(XEVENT event, int time/*tick*/)
{
    int tick = -1;
    if (time > 0)
    {
        tick = time;
    }
	fsm_.Shift(state_, event, combat_, tick);
}

bool CombatState::OptionPolicy(XOPT opt) const
{
    return state_->OptionPolicy(opt);
}

void CombatState::HeartBeat()
{
	int rst = state_->HeartBeat(combat_);
	if (!rst)
	{
		//state timeout
		Update(XEVENT_TIMEOUT);
	}
	else if (rst < 0)
	{
		//won't timeout forever
	}
	else if (rst > 0)
	{
		//hasn't timeout until now
	}
}

void CombatState::Clear()
{
}



///
/// StateOpenReady
///
void StateOpenReady::OnEnter(Combat* combat)
{
    combat->OnCombatStart();
}

void StateOpenReady::OnLeave(Combat* combat)
{
}

///
/// StateRunning
///
void StateRunning::OnEnter(Combat* combat)
{
    combat->ActivateATB();
}

///
/// StateStoped
///

///
/// StateSuspend
///
void StateSuspend::OnLeave(Combat* combat)
{
    CombatSceneAI* scene_ai = combat->scene_ai();
    if (scene_ai != NULL && scene_ai->IsYieldStatus())
    {
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_RESUME_SCENE_SCRIPT, combat->GetXID(), combat->GetXID(), 0, NULL, 0);
        combat->SendMSG(msg);
    }
}

///
/// StateCloseWait
///
void StateCloseWait::OnEnter(Combat* combat)
{
    combat->UnRegisterAllATB();
}

void StateCloseWait::OnHeartBeat(Combat* combat)
{
    if (combat->TestCloseCombat())
    {
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_COMBAT_END, combat->GetXID(), combat->GetXID(), 0, NULL, 0);
        combat->SendMSG(msg);
    }
}

///
/// StateLastWait
///
void StateLastWait::OnLeave(Combat* combat)
{
    if (combat->TryCloseCombat())
    {
        //战斗结束
        combat->DoCombatEnd();
        return;
    }

    //多波战斗
    MSG msg;
    BuildMessage(msg, COMBAT_MSG_COMBAT_CONTINUE, combat->GetXID(), combat->GetXID(), 0, NULL, 0);
    combat->SendMSG(msg);
}

///
/// StateStoped
///

///
/// StateWaitSelectSkill
///
void StateWaitSelectSkill::OnLeave(Combat* combat)
{
    CombatSceneAI* scene_ai = combat->scene_ai();
    if (scene_ai != NULL && scene_ai->IsYieldStatus())
    {
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_RESUME_SCENE_SCRIPT, combat->GetXID(), combat->GetXID(), 0, NULL, 0);
        combat->SendMSG(msg);
    }
}

///
/// StateWaitSelectPet
///
void StateWaitSelectPet::OnLeave(Combat* combat)
{
    CombatSceneAI* scene_ai = combat->scene_ai();
    if (scene_ai != NULL && scene_ai->IsYieldStatus())
    {
        MSG msg;
        BuildMessage(msg, COMBAT_MSG_RESUME_SCENE_SCRIPT, combat->GetXID(), combat->GetXID(), 0, NULL, 0);
        combat->SendMSG(msg);
    }
}

};
