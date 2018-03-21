#ifndef GAMED_GS_SUBSYS_PLAYER_STAR_H_
#define GAMED_GS_SUBSYS_PLAYER_STAR_H_

#include "gs/player/player_subsys.h"
#include "gamed/client_proto/G2C_proto.h"

namespace dataTempl
{
class StarTempl;
} // namespace dataTempl

namespace gamed
{

typedef std::map<int8_t, int8_t> SparkMap;
typedef std::map<int32_t, SparkMap> StarMap;

/**
 * @brief：player星盘子系统
 */
class PlayerStar : public PlayerSubSystem
{
public:
    PlayerStar(Player& player);
    virtual ~PlayerStar();

	bool SaveToDB(common::PlayerStarData* pData);
	bool LoadFromDB(const common::PlayerStarData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    void PlayerGetStarData() const;
    void OpenStar(int32_t star_id);
    void OpenStar(const std::vector<int32_t>& star_vec);
    void CloseStar(int32_t star_id);
    void ActivateSpark(int32_t star_id);
    bool AllSparkActivate(int32_t star_id) const;
    bool IsStarOpen(int32_t star_id) const;
    int32_t GetFullActivateStarNum() const;
    int32_t GetSparkNum() const;
protected:
	//
	// CMD处理函数
	//
	void CMDHandler_ActivateSpark(const C2G::ActivateSpark&);
private:
    bool CanOpenStar(int32_t star_id) const;
    bool CanActivateSpark(const dataTempl::StarTempl* templ) const;
    void AddSparkProp(int32_t prop, int32_t value);
private:
    StarMap star_map;
};

} // namespace gamed

#endif // GAMED_GS_SUBSYS_PLAYER_STAR_H_
