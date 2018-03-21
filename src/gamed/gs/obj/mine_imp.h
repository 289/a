#ifndef GAMED_GS_OBJ_MINE_IMP_H_
#define GAMED_GS_OBJ_MINE_IMP_H_

#include "matter.h"


namespace mapDataSvr
{
	class SpotMine;
	class AreaMine;
}


namespace gamed {

struct MineElemInfo
{
	A2DVECTOR birth_place;          // 出生点
	uint8_t   birth_dir;            // 出生朝向
	uint32_t  gather_player_num;    // 同时采集人数，0表示不限制采集人数
	bool      is_gather_disappear;  // 采集后是否消失
	bool      is_auto_refresh;      // 消失后是否刷新
	int32_t   refresh_time;         // 刷新时间，单位为妙
};

/**
 * @brief MineImp
 *    （1）不要直接访问Matter的成员变量（虽然是friend class）
 *         可以通过调用Matter的public及protected函数来实现
 */
class MineImp : public MatterImp
{
	static const int kStdGatherTimeoutDelay  = 15; // 单位:s
	static const int kDefaultMaxGatherPlayer = 1000;
public:
	MineImp(Matter& matter);
	virtual ~MineImp();

	virtual bool OnInit();
	virtual void OnHeartbeat();
	virtual int  OnMessageHandler(const MSG& msg);
	virtual void OnPlayerLeaveView(RoleID playerid);
    virtual bool OnMapTeleport();
	
	virtual bool CanAutoReborn() const;
	virtual int  GetRebornInterval();
    virtual void SetBirthPlace(const A2DVECTOR& pos);


private:
	bool InitRuntimeData();
	void FillElemInfo(const mapDataSvr::SpotMine* pmine);
	void FillElemInfo(const mapDataSvr::AreaMine* pmine);
	void HandleGatherRequest(const MSG& msg);
	void HandleGatherComplete(const MSG& msg);
	void RemoveGatheringPlayer(RoleID playerid, int32_t gather_seq_no);
	bool CanGather() const;


private:
	MineElemInfo        elem_info_;
	matterdef::MineType mine_type_;

	struct GatherInfo
	{
		GatherInfo() : gather_seq_no(0), gather_timeout(0) { }
		int32_t gather_seq_no;
		int32_t gather_timeout;
	};
	typedef std::map<RoleID, GatherInfo> GatheringPlayerMap;
	GatheringPlayerMap gathering_player_map_;
};

///
/// inline func
///

} // namespace gamed

#endif // GAMED_GS_OBJ_MINE_IMP_H_
