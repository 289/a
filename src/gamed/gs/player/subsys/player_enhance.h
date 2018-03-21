#ifndef GAMED_GS_SUBSYS_PLAYER_ENHANCE_H_
#define GAMED_GS_SUBSYS_PLAYER_ENHANCE_H_

#include "gs/player/player_subsys.h"
#include "gamed/client_proto/G2C_proto.h"

namespace gamed
{

enum EnhanceOpenMode
{
    ENHANCE_OPEN_INVALID,
    ENHANCE_OPEN_LEVEL,
    ENHANCE_OPEN_VIP,
    ENHANCE_OPEN_CASH,
    ENHANCE_OPEN_TYPE_MAX,
};

enum EnhanceSlotStatus
{
    ENHANCE_SLOT_NORMAL,
    ENHANCE_SLOT_PROTECTED,
};

/**
 * @brief：player附魔子系统
 */
class PlayerEnhance : public PlayerSubSystem
{
public:
    PlayerEnhance(Player& player);
    virtual ~PlayerEnhance();

	bool SaveToDB(common::PlayerEnhanceData* pData);
	bool LoadFromDB(const common::PlayerEnhanceData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    inline int32_t cash_slot_num() const;

    void PlayerGetEnhanceData() const;
    void OpenEnhanceSlot(int8_t mode);
    void ProtectEnhanceSlot(int8_t slot_index);
    void UnProtectEnhanceSlot(int8_t slot_index);
    void DoEnhance(int32_t enhance_gid, int32_t count);
    void QueryEnhanceSkill(std::set<int32_t>& skills) const;
    int32_t GetUnProtectSlotNum() const;
    int32_t GetProtectSlotNum() const;
    int32_t RandSelectEnhanceId(int32_t enhance_gid) const;
    int32_t GetEnhanceNum(int32_t level, int32_t rank) const;
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_OpenEnhanceSlot(const C2G::OpenEnhanceSlot&);
	void CMDHandler_ProtectEnhanceSlot(const C2G::ProtectEnhanceSlot&);
	void CMDHandler_UnProtectEnhanceSlot(const C2G::UnProtectEnhanceSlot&);
private:
    size_t GetUnProtectSlot(std::vector<int32_t>& slot_vec) const;
    size_t GetSlotNum(int32_t mode) const;
    static size_t GetMaxSlotNum(int32_t mode);
private:
    static const size_t kMaxEnhanceNum = 16;

    G2C::EnhanceList enhance_list_;
};

inline int32_t PlayerEnhance::cash_slot_num() const
{
    int32_t num = 0;
    for (size_t i = 0; i < enhance_list_.size(); ++i)
    {
        if (enhance_list_[i].open_mode == ENHANCE_OPEN_CASH)
        {
            ++num;
        }
    }
    return num;
}

} // namespace gamed 

#endif // GAMED_GS_SUBSYS_PLAYER_ENHANCE_H_
