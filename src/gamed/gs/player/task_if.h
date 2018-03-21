#ifndef GAMED_GS_PLAYER_TASK_IF_H_
#define GAMED_GS_PLAYER_TASK_IF_H_

#include "game_module/task/include/task_interface.h"


namespace gamed {

class Player;

/**
 * @brief PlayerTaskIf
 */
class PlayerTaskIf : public TaskInterface
{
public:
	PlayerTaskIf(Player* player);
	virtual ~PlayerTaskIf();

	// 基本信息相关接口
	virtual int64_t GetId() const;
	virtual int8_t  GetLevel() const;
	virtual int8_t  GetGender() const;
	virtual int32_t GetRoleClass() const; // 返回职业模板ID
	virtual int32_t GetItemCount(int32_t itemid) const;
	virtual int64_t GetGoldNum() const;
	virtual int32_t GetCash() const;
	virtual int32_t GetScore() const;
	virtual int32_t GetPos(double& x, double& y) const;
	virtual int32_t GetCurTime() const;
	virtual int32_t GetInsFinishCount(int32_t ins_id) const;
	virtual bool    GetGlobalCounter(int32_t counter_id, int32_t& value) const;
	virtual bool    GetMapCounter(int32_t id, int32_t world_id, int32_t& value) const;
	virtual bool    GetPlayerCounter(int32_t counter_id, int32_t& value) const;
    virtual int32_t GetReputation(int32_t reputation_id) const;
    virtual bool    IsWorldBuffExist(int32_t buff_id) const;
	virtual bool    CanDeliverItem(int8_t where, int32_t empty_slot_count) const;
    virtual bool    IsInMapArea(int32_t areaid);
	virtual void	ExeScript(int32_t task_id);
	virtual void    ScriptEnd(int32_t task_id, bool skip);
	virtual void	ExeGuide(int32_t task_id);
	virtual void    GuideEnd(int32_t task_id);
    virtual void    ModifyUI(int32_t ui_id, bool show);
    virtual int32_t GetCombatValue() const;
    virtual int32_t GetPetPower() const;
    virtual int32_t GetPetNum(int32_t level, int32_t blevel, int32_t rank) const;
    virtual int32_t GetCardNum(int32_t level, int32_t rank) const;
    virtual int32_t GetEnhanceNum(int32_t level, int32_t rank) const;
    virtual int32_t GetEquipNum(int32_t refine_level, int32_t rank) const;
    virtual int32_t GetFullActivateStarNum() const;
	virtual int32_t GetCatVisionLevel() const;
	virtual int32_t GetTalentLevel(int32_t talent_gid, int32_t talent_id) const;

	// 返回存储的任务数据
	virtual task::ActiveTask*     GetActiveTask();
	virtual task::FinishTask*     GetFinishTask();
	virtual task::FinishTimeTask* GetFinishTimeTask();
	virtual task::TaskDiary*	  GetTaskDiary();
	virtual task::TaskStorage*	  GetTaskStorage();

	virtual void	SendTaskNotify(uint16_t type, const void* buf, size_t size);

	// 只有服务器需要实现的接口
	virtual void    DeliverGold(int32_t num);
	virtual void    DeliverExp(int32_t exp);
	virtual void	DeliverScore(int32_t score);
	virtual void	DeliverSkillPoint(int32_t point);
	virtual void	DeliverTask(int32_t task_id);
	virtual void	FinishPlayerTask(int32_t task_id, bool succ);
	virtual void    DeliverItem(int32_t itemid, int32_t num, int32_t valid_time);
	virtual void    TakeAwayGold(int32_t num);
	virtual void    TakeAwayCash(int32_t num);
	virtual void	TakeAwayScore(int32_t num);
	virtual void    TakeAwayItem(int32_t itemid, int32_t num);
	virtual void    TakeAwayAllItem(int32_t itemid, int32_t num);
	virtual void	TransferClass(int8_t role_class);
	virtual void    ModifyNPCBWList(int32_t templ_id, bool black, bool add);
	virtual void	CtrlMapElement(int32_t element_id, bool open);
	virtual void    ModifyGlobalCounter(int32_t counter_id, int8_t op, int32_t delta);
	virtual void    ModifyPlayerCounter(int32_t id, int8_t op, int32_t value);
	virtual void    ModifyMapCounter(int32_t world_id, int32_t id, int8_t op, int32_t value);
	virtual void	SendSysMail(shared::net::ByteBuffer& data);
	virtual void    EnterInstance(int32_t ins_id);
	virtual void	FinishPlayerIns(int32_t ins_id, bool succ);
	virtual bool	MiniGameEnd(int32_t game_type);
	virtual void	SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content);
	virtual void	ServerTransport(int32_t world_id, double x, double y);
    virtual void    OpenReputation(int32_t reputation_id);
    virtual void    ModifyReputation(int32_t reputation_id, int32_t delta);
    virtual void    ModifyWorldBuff(int8_t op, int32_t buff_id, bool save);
    virtual void    StorageTaskComplete(int32_t taskid, int8_t quality); 
    virtual void    OpenTalent(int32_t talent_gid);
    virtual void    OpenTitle(int32_t title_id);
    virtual void    SubscribeGlobalCounter(int32_t counter_id, bool is_subscribe);
    virtual void    SubscribeMapCounter(int32_t world_id, int32_t id, bool is_subscribe); // 副本或战场里的地图计数器
    virtual void    AddNPCFriend(int32_t id);
    virtual void    DelNPCFriend(int32_t id);
    virtual void    OnlineNPCFriend(int32_t id);
    virtual void    OfflineNPCFriend(int32_t id);
    virtual void    OpenStar(int32_t id);
    virtual void    CompleteGevent(int32_t id);
    virtual void    OpenMountEquip(int32_t index);
    virtual void    ExitBattleGround(int32_t bg_tid);

	virtual void    ShowActiveTask();
	virtual void    ShowFinishTask();
	virtual void    ShowFinishTimeTask();

	// for client
	virtual void	TransportTo(int32_t taskid, int32_t world_id, double x, double y) { }
	virtual void	TransferIns(int32_t taskid, int32_t ins_id) { }
	virtual void    TransferBG(int32_t taskid, int32_t bg_id) { }
	virtual void	SummonNPC(int32_t task_id, const NPCInfo& npc) { }
	virtual void	SummonMonster(int32_t task_id, const MonsterInfo& monster, bool revive) { }
	virtual void	SummonMine(int32_t task_id, const MineInfo& mine) { }
	virtual void	RecycleSummonObj(int32_t task_id, int32_t templ_id) { }
	virtual void	PopupDlg(int32_t taskid, int8_t dlg_type) { }
	virtual void	MiniGameStart(int32_t taskid, int32_t game_type) { }
	virtual void	ChangeScale(int8_t scale) { }
	virtual void	ShowTalk(int32_t task_id, int8_t talk_type) { }
	virtual void	ShowErrMsg(int32_t err) { };
	virtual void	StatusChanged(int32_t taskid, int8_t status, bool succ = false, bool hide = false) { }
	virtual void	CatVisionAnimation(bool open) { }
	virtual void	BlockPlayerMove() { }
    virtual void    CloseNpcArea() { }
    virtual void    ShowItemUseHint(int32_t taskid, int32_t itemid, const std::string& tip) { }
    virtual void    HideItemUseHint(int32_t taskid, int32_t itemid) { }
    virtual void    ClearTaskTracking() { }
    virtual void    DoCameraMaskGfx(const std::string& path, bool open) { }

private:
	Player* pplayer_;
};

} // namespace gamed

#endif // GAMED_GS_PLAYER_TASK_IF_H_
