#ifndef GAMED_GS_SUBSYS_PLAYER_BUFF_H_
#define GAMED_GS_SUBSYS_PLAYER_BUFF_H_

#include <map>
#include <vector>

#include "gs/player/filter.h"
#include "gs/player/player_subsys.h"


namespace gamed {

/**
 * @brief 玩家Buff子系统
 */
class PlayerBuff : public PlayerSubSystem
{
public:
	PlayerBuff(Player& player);
	~PlayerBuff();

	virtual void RegisterMsgHandler();
	virtual void OnHeartbeat(time_t cur_time);

	bool    LoadFromDB(const common::PlayerBuffData& data);
	bool    SaveToDB(common::PlayerBuffData* pData);

    void    AddFilter(Filter* obj);
    void    DelFilter(int32_t type);
    void    DelFilterByID(int32_t effectid);
    void    DelSpecFilter(int32_t filter_mask);
    bool    IsFilterExistByID(int32_t effectid);

    
protected:
	int     MSGHandler_AddFilter(const MSG& msg);
	int     MSGHandler_DelFilter(const MSG& msg);
	void    DelFilter(Filter* obj);
    void    AlterVisibleState();
	void    CollectBuffData(std::vector<PlayerVisibleState::BuffInfo>& buff_vec) const;
    bool    GetSysBuffID(int32_t buff_type, int32_t& id) const;


private:
    void    RawAddFilter(Filter* obj);
    void    RawDelFilter(Filter* obj);
	void    DoChange();
    void    DoInsert(Filter* obj);
    void    DoDelete(Filter* obj);
    void    HandleInactiveFilter();
    void    RetainStart();
    void    RetainEnd();

    enum FilterOpType 
    {
        FOT_ADD = 0,
        FOT_DEL,
        FOT_DEL_BY_TYPE,
        FOT_DEL_BY_MASK,
        FOT_DEL_BY_EID,
    };

    struct OpParam
    {
        OpParam(Filter* p, int32_t t)
            : pfilter(p),
              param(t)
        { }
        Filter* pfilter;
        int32_t param;
    };

    
private:
	typedef std::multimap<int32_t/*FILTER_INDEX*/, Filter*> FilterMultimap;
    typedef std::pair<FilterMultimap::iterator, FilterMultimap::iterator> FilterMultimapPair;
    FilterMultimap   filter_map_;

    typedef std::pair<int32_t/*FilterOpType*/, OpParam> WaitingPair;
    typedef std::vector<WaitingPair> FilterWaitingVec;
    FilterWaitingVec list_wait_;

    // retain filter pointer a while
    std::vector<Filter*> retain_filter_;

    // update or not
    bool    update_flag_;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_BUFF_H_
