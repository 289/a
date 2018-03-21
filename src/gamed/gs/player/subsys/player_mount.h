#ifndef GAMED_GS_SUBSYS_PLAYER_MOUNT_H_
#define GAMED_GS_SUBSYS_PLAYER_MOUNT_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"

namespace dataTempl
{
class MountTempl;
class MountEquipTempl;
}

namespace gamed
{

class PlayerMount : public PlayerSubSystem
{
public:
    PlayerMount(Player& player);
    virtual ~PlayerMount();

	bool SaveToDB(common::PlayerMountData* pData);
	bool LoadFromDB(const common::PlayerMountData& data);

	virtual void RegisterCmdHandler();

    void PlayerGetMountData() const;
	void RegisterMount(int32_t item_index, int32_t item_id);
	void UnRegisterMount(int32_t item_index);
    void OpenMountEquip(int32_t equip_index);
    int32_t GetMountCategory() const;
    int32_t GetMountEquipLevel(int32_t index) const;
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_MountMount(const C2G::MountMount&);
	void CMDHandler_MountExchange(const C2G::MountExchange&);
	void CMDHandler_MountEquipLevelUp(const C2G::MountEquipLevelUp&);
private:
    void NotifyMountChange(int32_t mount_id);
private:
    typedef std::map<int32_t/*item_index*/, const dataTempl::MountTempl*> MountMap;
    typedef std::map<int32_t/*mount_templ_id*/, int32_t/*num*/> CategoryMap;
    int32_t mount_index_;                       // 当前骑乘的坐骑索引
    MountMap mount_inv_;                        // 坐骑列表
    std::vector<int32_t> mount_equip_level_;    // 骑具等级列表
    CategoryMap mount_category_;                // 坐骑的种类
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_MOUNT_H_
