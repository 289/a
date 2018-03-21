#ifndef GAMED_TASK_INTERFACE_H_
#define GAMED_TASK_INTERFACE_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace shared
{
namespace net
{
class ByteBuffer;
} // namespace net
} // namespace shared

namespace task
{
	class ActiveTask;
	class FinishTask;
	class FinishTimeTask;
	class TaskDiary;
	class TaskStorage;
} // namespace task

namespace gamed
{

struct Position
{
	int32_t world_id;
	double x;
	double y;
};

struct GameObjInfo
{
	int32_t id;
	int32_t dir;
	Position pos;
	int32_t layout;
	int8_t appear_type;
    std::string name;
};

struct NPCInfo : public GameObjInfo
{
    std::string action;
};

struct MonsterInfo : public GameObjInfo
{
	int32_t monster_group;
	int32_t scene_id;
	bool active;
    std::string action;
};

struct MineInfo : public GameObjInfo
{
};

///
/// 对象封装类,任务系统使用
///
class TaskInterface
{
public:
	TaskInterface()
	{
	}
	virtual ~TaskInterface()
	{
	}

	// 服务器，客户端都需实现的接口
	virtual int64_t GetId() const = 0;
	virtual int8_t  GetLevel() const = 0;
	virtual int8_t GetGender() const = 0;
	virtual int32_t GetRoleClass() const = 0;
	virtual int32_t GetItemCount(int32_t item_id) const = 0;
	virtual int64_t GetGoldNum() const = 0;
	virtual int32_t GetCash() const = 0;
	virtual int32_t GetScore() const = 0;
	virtual int32_t GetPos(double& x, double& y) const = 0; // 返回地图ID
	virtual int32_t GetCurTime() const = 0;	// 获取当前时间	
	virtual int32_t GetInsFinishCount(int32_t ins_id) const = 0;
	virtual bool GetGlobalCounter(int32_t counter_id, int32_t& value) const = 0;
	virtual bool GetMapCounter(int32_t id, int32_t world_id, int32_t& value) const = 0;
	virtual bool GetPlayerCounter(int32_t counter_id, int32_t& value) const = 0;
    virtual int32_t GetReputation(int32_t reputation_id) const = 0;
    virtual bool IsWorldBuffExist(int32_t buff_id) const = 0;
	virtual bool CanDeliverItem(int8_t packet_type, int32_t item_types) const = 0;
    virtual bool IsInMapArea(int32_t areaid) = 0;
	virtual void ExeScript(int32_t taskid) = 0;
	virtual void ScriptEnd(int32_t taskid, bool skip) = 0;
	virtual void ExeGuide(int32_t taskid) = 0;
	virtual void GuideEnd(int32_t taskid) = 0;
    virtual void ModifyUI(int32_t ui_id, bool show) = 0;
	virtual int32_t GetCombatValue() const = 0;
    virtual int32_t GetPetPower() const = 0;
    virtual int32_t GetPetNum(int32_t level, int32_t blevel, int32_t rank) const = 0;
    virtual int32_t GetCardNum(int32_t level, int32_t rank) const = 0;
    virtual int32_t GetEnhanceNum(int32_t level, int32_t rank) const = 0;
    virtual int32_t GetEquipNum(int32_t refine_level, int32_t rank) const = 0;
    virtual int32_t GetFullActivateStarNum() const = 0;
	virtual int32_t GetCatVisionLevel() const = 0;
	virtual int32_t GetTalentLevel(int32_t talent_gid, int32_t talent_id) const = 0;

	virtual task::ActiveTask* GetActiveTask() = 0;
	virtual task::FinishTask* GetFinishTask() = 0;
	virtual task::FinishTimeTask* GetFinishTimeTask() = 0;
	virtual task::TaskDiary* GetTaskDiary() = 0;
	virtual task::TaskStorage* GetTaskStorage() = 0;

	virtual void SendTaskNotify(uint16_t type, const void* buf, size_t size) = 0;

	// 只有客户端需要实现的接口
	virtual void TransportTo(int32_t taskid, int32_t world_id, double x, double y) = 0;
	virtual void TransferIns(int32_t taskid, int32_t ins_id) = 0;
	virtual void TransferBG(int32_t taskid, int32_t bg_id) = 0;
	virtual void SummonNPC(int32_t taskid, const NPCInfo& npc) = 0;
	virtual void SummonMonster(int32_t taskid, const MonsterInfo& monster, bool revive) = 0;
	virtual void SummonMine(int32_t taskid, const MineInfo& mine) = 0;
	virtual void RecycleSummonObj(int32_t taskid, int32_t templ_id) = 0;
	virtual void PopupDlg(int32_t taskid, int8_t dlg_type) = 0;
	virtual void MiniGameStart(int32_t taskid, int32_t game_type) = 0;
	virtual void ChangeScale(int8_t scale) = 0;
	virtual void ShowTalk(int32_t taskid, int8_t talk_type) = 0;
	virtual void ShowErrMsg(int32_t err) = 0;
	virtual void StatusChanged(int32_t taskid, int8_t status, bool succ = false, bool hide = false) = 0;
	virtual void CatVisionAnimation(bool open) = 0;
	virtual void BlockPlayerMove() = 0;
    virtual void CloseNpcArea() = 0;
    virtual void ShowItemUseHint(int32_t taskid, int32_t itemid, const std::string& tip) = 0;
    virtual void HideItemUseHint(int32_t taskid, int32_t itemid) = 0;
    virtual void ClearTaskTracking() = 0;
    virtual void DoCameraMaskGfx(const std::string& path, bool open) = 0;

	// 只有服务器需要实现的接口
	virtual void DeliverGold(int32_t num) = 0;
	virtual void DeliverExp(int32_t exp) = 0;
	virtual void DeliverScore(int32_t score) = 0;
	virtual void DeliverSkillPoint(int32_t point) = 0;
	virtual void DeliverTask(int32_t taskid) = 0;
	virtual void FinishPlayerTask(int32_t taskid, bool succ) = 0;
	virtual void DeliverItem(int32_t item_id, int32_t num, int32_t valid_time) = 0;
	virtual void TakeAwayGold(int32_t num) = 0;
	virtual void TakeAwayCash(int32_t num) = 0;
	virtual void TakeAwayScore(int32_t num) = 0;
	virtual void TakeAwayItem(int32_t item_id, int32_t num) = 0;
	virtual void TakeAwayAllItem(int32_t item_id, int32_t num) = 0;
	virtual void TransferClass(int8_t role_class) = 0;
	virtual void ModifyNPCBWList(int32_t templ_id, bool black, bool add) = 0;
	virtual void CtrlMapElement(int32_t element_id, bool open) = 0;
	virtual void ModifyGlobalCounter(int32_t id, int8_t op, int32_t value) = 0;
	virtual void ModifyPlayerCounter(int32_t id, int8_t op, int32_t value) = 0;
	virtual void ModifyMapCounter(int32_t world_id, int32_t id, int8_t op, int32_t value) = 0;
	virtual void SendSysMail(shared::net::ByteBuffer& data) = 0;
	virtual void EnterInstance(int32_t ins_id) = 0;
	virtual void FinishPlayerIns(int32_t ins_id, bool succ) = 0;
	virtual bool MiniGameEnd(int32_t game_type) = 0;
	virtual void SendSysChat(int8_t channel, const std::string& sender_name, const std::string& content) = 0;
	virtual void ServerTransport(int32_t world_id, double x, double y) = 0;
    virtual void OpenReputation(int32_t reputation_id) = 0;
    virtual void ModifyReputation(int32_t reputation_id, int32_t delta) = 0;
    virtual void ModifyWorldBuff(int8_t op, int32_t buff_id, bool save) = 0;
    virtual void StorageTaskComplete(int32_t taskid, int8_t quality) = 0; 
    virtual void OpenTalent(int32_t talent_gid) = 0;
    virtual void OpenTitle(int32_t title_id) = 0;
    virtual void SubscribeGlobalCounter(int32_t counter_id, bool is_subscribe) = 0;
    virtual void SubscribeMapCounter(int32_t world_id, int32_t id, bool is_subscribe) = 0;
    virtual void AddNPCFriend(int32_t id) = 0;
    virtual void DelNPCFriend(int32_t id) = 0;
    virtual void OnlineNPCFriend(int32_t id) = 0;
    virtual void OfflineNPCFriend(int32_t id) = 0;
    virtual void OpenStar(int32_t star_id) = 0;
    virtual void CompleteGevent(int32_t gevent_gid) = 0;
    virtual void OpenMountEquip(int32_t equip_index) = 0;
    virtual void ExitBattleGround(int32_t battle_ground_id) = 0;
};

} // namespace gamed

#endif // GAMED_TASK_INTERFACE_H_
