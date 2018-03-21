#ifndef GAMED_GS_GLOBAL_MAP_ELEM_CTRL_H_
#define GAMED_GS_GLOBAL_MAP_ELEM_CTRL_H_

#include "shared/base/singleton.h"

#include "time_slot.h"


namespace extraTempl {
	class MapElemCtrl;
} // namespace extraTempl


namespace gamed {

/**
 * @brief MapElemController
 */
class MapElemCtrller : public shared::Singleton<MapElemCtrller>
{
	friend class shared::Singleton<MapElemCtrller>;
	const static int kMaxCheckoutTime = 9*60 + 1; // 9分零1秒
public:
	static inline MapElemCtrller* GetInstance() {
		return &(get_mutable_instance());
	}

	/**
	 * @brief Init 
	 * （1）必须在所有地图初始化完毕后才能调用该Init函数
	 */
	bool Init();

	/**
	 * @brief HeartbeatTick 
	 * （1）Tick心跳，每秒20次
	 */
	void HeartbeatTick();


protected:
	MapElemCtrller();
	~MapElemCtrller();


private:
	enum ListInsertType
	{
		NOTHING_INSERTED = 0,
		INSERT_TO_ACTIVE,
		INSERT_TO_WAITING,
		INSERT_TO_INVALID,
	};

	struct CtrllerInfo
	{
		CtrllerInfo() : ptempl(NULL) { }

		TimeSlot time_slot;
		const extraTempl::MapElemCtrl* ptempl;
	};
	
	typedef std::pair<time_t, const CtrllerInfo*> Entry;
	typedef std::set<Entry> CtrllerList;
	
	int32_t CalcNextCheckoutTime(time_t now);
	bool    BuildCtrllerList(time_t now);
	void    ActivateCtrller(const CtrllerInfo& info);
	void    DeactivateCtrller(const CtrllerInfo& info);
	void    EnableMapElem(int32_t elemid);
	void    DisableMapElem(int32_t elemid);
	std::vector<Entry> GetExpired(time_t now, CtrllerList& ctrller_list);
	ListInsertType InsertToCtrllerList(time_t now, const CtrllerInfo* info);


private:
	int32_t         hb_ticker_;       // 心跳计数
	int32_t         check_countdown_; // 下一次检查时间, 单位：s

	typedef std::vector<CtrllerInfo> CtrllerInfoVec;
	CtrllerInfoVec  ctrller_info_vec_; // 启动后数量就不会变化,因此可以直接使用vector元素的地址

	// count-down-time and CtrllerInfo pointer
	CtrllerList     active_ctrller_;
	CtrllerList     waiting_ctrller_;
	CtrllerList     invalid_ctrller_;

	std::vector<Entry> expired_vec_;
};

#define s_pMapElemCtrller gamed::MapElemCtrller::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_MAP_ELEM_CTRL_H_
