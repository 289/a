#ifndef GAMED_GS_SUBSYS_PLAYER_MALL_H_
#define GAMED_GS_SUBSYS_PLAYER_MALL_H_

#include "gs/player/player_subsys.h"
#include "gs/player/player_def.h"


namespace gamed {

class PlayerMall : public PlayerSubSystem
{
public:
	PlayerMall(Player& player);
	virtual ~PlayerMall();

	bool SaveToDB(common::PlayerMallData* pData);
	bool LoadFromDB(const common::PlayerMallData& data);

	virtual void RegisterCmdHandler();
	virtual void RegisterMsgHandler();

    void    SetCashInfo(int32_t cash, int32_t cash_used);   // 上线时调用
	void    GetCashInfo(int32_t& cash, int32_t& cash_used); // 存盘时调用
	void    SetCashTotal(int32_t cash_total); // 充值即时到账调用
	int32_t GetCash() const;     // 获取当前可用点数
	int32_t GetCashUsed() const; // 获取已经使用点数
	bool    NeedSave() const;
	bool    CheckCash(int32_t cash) const;
	void    UseCash(int32_t offset);
	void    AddCash(int32_t cash_add);
    void    PlayerGetCash() const;


protected:
	void    CMDHandler_OpenMall(const C2G::OpenMall& cmd);
	void    CMDHandler_QueryMallGoodsDetail(const C2G::QueryMallGoodsDetail& cmd);
	void    CMDHandler_MallShopping(const C2G::MallShopping& cmd);


private:
	int32_t GetBuyCount(int32_t goods_id) const;
	void    PlayerGetMallGoodsDetail(int32_t goods_id) const;
	void    DeleteExpiredMallOrder();
	void    NotifyCashChange(int32_t old_cash) const;
	
    struct mall_order
	{
		int32_t order_id;
		int32_t goods_id;
		int32_t goods_count;
		int32_t timestamp;
	};

    mall_order* GetMallOrder(int32_t goods_id);
	const mall_order* GetMallOrder(int32_t goods_id) const;

	typedef std::vector<mall_order> MallOrderVec;
	MallOrderVec mall_orders_;


private:
	/**
	 * 1) 玩家可用点数：    mall_cash_ + mall_cash_offset_;
	 * 2) 玩家充值总点数：  mall_cash_ + mall_cash_used_;
	 * 3) 玩家使用总点数：  mall_cash_used_;  
	 * 4) 玩家购耗点数记在：mall_cash_offset_上，存盘成功后，清空mall_cash_offset_;
	 */
	int32_t mall_cash_;           // 原来可用点数,
	int32_t mall_cash_used_;      // 总共用了多少点数,
	int32_t mall_cash_offset_;    // 消耗了多少点数(上次存盘到现在玩家用的点数,为负数),
	int32_t mall_order_id_;       // 商城购物的流水号,
	int32_t mall_order_id_saved_; // 最近一次存盘的商城购物流水号,
};

};

#endif
