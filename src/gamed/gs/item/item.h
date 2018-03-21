#ifndef _GAMED_GS_ITEM_ITEM_H_
#define _GAMED_GS_ITEM_ITEM_H_


#include "item_data.h"
#include "item_essence.h"

#include "gs/global/game_types.h"
#include "gs/template/data_templ/base_datatempl.h"
#include "gamed/client_proto/G2C_proto.h"
#include "shared/logsys/logging.h"
#include "common/obj_data/player_attr_declar.h"

namespace gamed
{

namespace playerdef
{
    struct CoolDownInfo;
};

class Unit;
class Item;
class ItemBody;
class ItemEssence;
class ItemManager;


void MakeItem(Item& it, const itemdata& data);
void MakeItemdata(itemdata& data, const G2C::ItemDetail& detail);
void MakeItemDetail(G2C::ItemDetail& detail, const itemdata& data);

/**
 * @class Item
 * @brief 物品类对象
 * @ItemData 定义在common下面，玩家身上的物品数据类型
 * @itemdata 辅助物品数据，主要用在从物品模板中生成一个物品
 */
class Item
{
private:
	int32_t type;
	int32_t count;
	int32_t pile_limit;     //堆叠上限
	int32_t proc_type;      //处理规则
	int32_t expire_date;    //过期时间
	int32_t price;          //卖店价(出售价)
	int32_t equip_mask;     //装备位置
	int32_t item_cls;       //物品分类
    ItemEssence ess;        //动态属性
	ItemBody* pBody;        //物品实体部分(缓存物品的静态数据)
	bool is_active;         //是否激活
	int ref;				//测试使用

public:

	friend class ItemBody;
	friend class ItemList;

	friend void MakeItem(Item& it, const itemdata& data);
	friend void MakeItemdata(itemdata& data, const G2C::ItemDetail& detail);
	friend void MakeItemDetail(G2C::ItemDetail& detail, const itemdata& data);

	enum LOCATION
	{
		INVENTORY,
		EQUIPMENT,
		TASK_INV,
		HIDE_INV,
		PET_INV,
        CARD_INV,
        MOUNT_INV,
		INV_MAX,
	};

	enum ITEM_CLS
	{
		ITEM_CLS_INVALID,
		ITEM_CLS_NORMAL,
		ITEM_CLS_EQUIP,
		ITEM_CLS_CARD,
		ITEM_CLS_TASK,
		ITEM_CLS_HIDE,
		ITEM_CLS_TASK_SCROLL,
		ITEM_CLS_PET,
        ITEM_CLS_SKILL,
        ITEM_CLS_MOUNT,
		ITEM_CLS_MAX,
	};

	enum EQUIP_MASK
	{
		EQUIP_MASK_WEAPON   = 0x0001, //武器
		EQUIP_MASK_HEAD     = 0x0002, //头部
		EQUIP_MASK_NECK     = 0x0004, //颈部
		EQUIP_MASK_CHEST    = 0x0008, //胸部
		EQUIP_MASK_HAND     = 0x0010, //手部
		EQUIP_MASK_LEG      = 0x0020, //腿部
		EQUIP_MASK_SHOE     = 0x0040, //脚部
		EQUIP_MASK_ALL		= 0x007F,
	};

	enum EQUIP_INDEX
	{
		EQUIP_INDEX_WEAPON,
		EQUIP_INDEX_HEAD,
		EQUIP_INDEX_NECK,
		EQUIP_INDEX_CHEST,
		EQUIP_INDEX_HAND,
		EQUIP_INDEX_LEG,
		EQUIP_INDEX_SHOE,
		EQUIP_INDEX_MAX,
	};

	enum ITEM_PROC_TYPE
	{
		ITEM_PROC_TYPE_NODROP           = 0x0001,	//不可丢弃
		ITEM_PROC_TYPE_NOSELL           = 0x0002,	//不可出售
		ITEM_PROC_TYPE_NOTRADE          = 0x0004,	//不可交易
		ITEM_PROC_TYPE_BIND_ON_GAIN     = 0x0008,	//拾取绑定
		ITEM_PROC_TYPE_BIND_ON_EQUIP    = 0x0010,	//装备绑定
		ITEM_PROC_TYPE_BIND             = 0x0020,	//处于绑定状态
        ITEM_PROC_TYPE_DISABLED         = 0x0040,   //装备失效
	};

    enum ITEM_USE
    {
        ITEM_USE_FAILURE = -1, // 使用物品失败
        ITEM_USE_CONSUME = 0,  // 消耗物品，使用成功也返回这个值
        ITEM_USE_RETAIN,       // 物品保留，使用后不消耗物品
    };

    ///
    /// 根据物品的模板类型获得相应的物品分组类型
    ///
    static int LocateItemCls(dataTempl::TemplType type)
    {
        switch(type)
        {
            case dataTempl::TEMPL_TYPE_EQUIP:
                return ITEM_CLS_EQUIP;
            case dataTempl::TEMPL_TYPE_CARD:
                return ITEM_CLS_CARD;
            case dataTempl::TEMPL_TYPE_HIDDEN_ITEM:
                return ITEM_CLS_HIDE;
            case dataTempl::TEMPL_TYPE_NORMAL_ITEM:
                return ITEM_CLS_NORMAL;
            case dataTempl::TEMPL_TYPE_TASK_ITEM:
                return ITEM_CLS_TASK;
            case dataTempl::TEMPL_TYPE_TASK_SCROLL:
                return ITEM_CLS_TASK_SCROLL;
            case dataTempl::TEMPL_TYPE_PET_ITEM:
                return ITEM_CLS_PET;
            case dataTempl::TEMPL_TYPE_SKILL_ITEM:
                return ITEM_CLS_SKILL;
            case dataTempl::TEMPL_TYPE_MOUNT:
                return ITEM_CLS_MOUNT;
            default:
                return ITEM_CLS_INVALID;
        };
    }

    ///
    /// 根据物品的分类类型确定应该往哪个包裹放
    ///
    static int LocateLocation(int item_cls)
    {
        switch (item_cls)
        {
            case ITEM_CLS_NORMAL:
            case ITEM_CLS_EQUIP:
            case ITEM_CLS_TASK_SCROLL:
            case ITEM_CLS_SKILL:
                return INVENTORY;
            case ITEM_CLS_TASK:
                return TASK_INV;
            case ITEM_CLS_HIDE:
                return HIDE_INV;
            case ITEM_CLS_PET:
                return PET_INV;
            case ITEM_CLS_CARD:
                return CARD_INV;
            case ITEM_CLS_MOUNT:
                return MOUNT_INV;
            default:
                LOG_ERROR << "未知物品类型, item_cls = " << item_cls;
                return -1;
        }

        return -1;
    }

public:
	Item() : type(-1),
	         count(0),
			 pile_limit(0),
			 proc_type(0),
			 expire_date(0),
			 price(0),
			 equip_mask(0),
			 item_cls(0),
			 pBody(NULL),
			 is_active(false),
			 ref(0)
			 { }
	~Item()
	{
		Clear();
		ASSERT(NULL == pBody && !is_active);
	}

	Item& operator=(const Item& rhs)
	{
        ASSERT(rhs.count > 0);
		type = rhs.type;
		count = rhs.count;
		pile_limit = rhs.pile_limit;
		proc_type = rhs.proc_type;
		expire_date = rhs.expire_date;
		price = rhs.price;
		equip_mask = rhs.equip_mask;
		item_cls = rhs.item_cls;
		pBody = rhs.pBody;
		is_active = false;
		ref = 0;
        ess = rhs.ess;
		return *this;
	}

	int32_t Type() const				{ return type; }
	int32_t Amount() const				{ return count; }
	int32_t PileLimit()	const			{ return pile_limit; }
	int32_t ProcType() const			{ return proc_type; }
	int32_t ExpireDate() const			{ return expire_date; }
	int32_t BasePrice() const           { return price; } // 取物品的总价格要用TotalPrice
	int32_t ItemCls() const             { return item_cls; }
	void SetType(int32_t t)			    { type = t; }
	void SetAmount(int32_t n)			{ ASSERT(n > 0); count = n; }
	void SetExpireDate(int time)        { expire_date = time; }
	void SetProcType(int proctype)      { proc_type = proctype; }
	void IncAmount(int32_t n)			{ ASSERT(count + n <= pile_limit); count += n; }
	void DecAmount(int32_t n)			{ ASSERT(count - n >= 0); count -= n; }
	bool CanDrop() const				{ return !(proc_type & (ITEM_PROC_TYPE_NODROP | ITEM_PROC_TYPE_BIND)); }
	bool CanTrade() const				{ return !(proc_type & (ITEM_PROC_TYPE_NOTRADE | ITEM_PROC_TYPE_BIND)); }
	bool CanSell() const				{ return !(proc_type & ITEM_PROC_TYPE_NOSELL); }//注意: 绑定不影响卖店
	bool TestBind() const				{ return proc_type & ITEM_PROC_TYPE_BIND; }
	bool TestBindOnGain() const			{ return proc_type & ITEM_PROC_TYPE_BIND_ON_GAIN; }
	bool TestBindOnEquip() const		{ return proc_type & ITEM_PROC_TYPE_BIND_ON_EQUIP; }
    bool TestDisabled() const           { return proc_type & ITEM_PROC_TYPE_DISABLED; }
	void SetActive(bool active)			{ is_active = active; }
	bool IsActive() const				{ return is_active; }
	void Bind()							{ proc_type |= ITEM_PROC_TYPE_BIND; }
    void SetDisabled()                  { proc_type |= ITEM_PROC_TYPE_DISABLED; }
    void ClrDisabled()                  { proc_type &= ~ITEM_PROC_TYPE_DISABLED; }
    int32_t GetTotalPrice() const;

    void  Clear();
    void  Release();
    void  LoadFromDB(const common::PlayerInventoryData::ItemData& data);
    void  SaveForDB(common::PlayerInventoryData::ItemData& data) const;
    void  SaveItem(itemdata& data) const;

    // 转发到body的函数
	inline int   GetRank() const;                                        // 整理包裹时使用,同ID物品存放顺序
	inline int   GetEquipMask() const;                                   // 获取可以装备的部位
	inline int   GetExtraPrice() const;                                  // 属性提升得到的额外卖店价
	inline ClassMask GetClsLimit() const;                                // 获取装备时的职业限制
	inline int   GetRefineLevel() const;                                 // 获取装备的精练等级
	inline bool  CanUse(LOCATION l, size_t index, Unit* obj);		     // 是否能使用
	inline bool  CanRefine(LOCATION l, size_t index, Unit* obj);	     // 是否能使用
	inline bool  CanActivate(LOCATION l, size_t index, Unit* obj);	     // 是否能装备
	inline bool  CanComposite(LOCATION l, size_t index, Unit* obj);      // 是否能合成
	inline void  RefineEquip(LOCATION l, size_t index, Unit* obj);	     // 装备精练
	inline void  OnPutIn(LOCATION l, size_t index, Unit* obj);		     // 物品进包裹
	inline void  OnTakeOut(LOCATION l, size_t index, Unit* obj);	     // 从包裹取出
	inline bool  Update(LOCATION l, size_t index, Unit* obj);            // 更新物品
	inline bool  Composite(LOCATION l, size_t index, Unit* obj);         // 合成卡牌
    inline void  Enable(LOCATION l, size_t index, Unit* obj);            // 使装备生效
    inline void  Disable(LOCATION l, size_t index, Unit* obj);           // 使物品失效
	inline ITEM_USE UseItem(LOCATION l, size_t index, Unit* obj);		 // 使用物品，返回0表示需要使用后扣除物品
    inline ItemEssence& GetItemEssence();
    inline const ItemEssence& GetItemEssence() const;

	void LocateSelf();
};

/**
 * @class ItemBody
 * @brief 物品实体类
 * @brief 静态数据类，禁止修改内部数据
 * @brief 可以通过该类提供的函数来修改物品的动态属性数据
 */
class ItemBody : public shared::noncopyable
{
protected:
	int id_;

public:
	explicit ItemBody(int id) : id_(id) { }
	virtual ~ItemBody() { }

    bool CanUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
    bool CanRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
    bool CanActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
    bool CanComposite(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const;
    Item::ITEM_USE UseItem(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void RefineEquip(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void OnPutIn(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void OnTakeOut(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    bool Update(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    bool Composite(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void Enable(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void Disable(Item::LOCATION l, size_t index, Unit* obj, Item* parent);

	virtual int  GetRank(const Item* parent) const                    { return -1; }
	virtual int  GetEquipMask() const                                 { return 0; }
	virtual int  GetExtraPrice(const Item* parent) const              { return 0; }
	virtual ClassMask GetClsLimit() const                             { return ClassMask(); }
	virtual int  GetRefineLevel(const Item* parent) const             { return 0; }
	virtual void Initialize(const dataTempl::ItemDataTempl*)          { }
	virtual void OnLoadFromDB(Item* parent) const                     { }

private:
    void Activate(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void Deactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void DoEnable(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
    void DoDisable(Item::LOCATION l, size_t index, Unit* obj, Item* parent);
	
	virtual bool  TestUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const        { return false; }
	virtual bool  TestRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const     { return false; }
	virtual bool  TestActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const   { return false; }
	virtual bool  TestComposite(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const  { return false; }
	virtual Item::ITEM_USE OnUse(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const { return Item::ITEM_USE_FAILURE; }
	virtual void  OnActivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const     { }
	virtual void  OnDeactivate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const   { }
	virtual void  OnEnable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const       { }
	virtual void  OnDisable(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const      { }
	virtual void  DoRefine(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const       { }
	virtual bool  DoUpdate(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const       { return false; }
	virtual bool  DoComposite(Item::LOCATION l, size_t index, Unit* obj, Item* parent) const    { return false; }
};

///
/// inline func
///
inline int  Item::GetRank() const                                           { if (!pBody) return -1; return pBody->GetRank(this); }
inline int  Item::GetEquipMask() const                                      { if (!pBody) return 0; return pBody->GetEquipMask(); }
inline int  Item::GetExtraPrice() const                                     { if (!pBody) return 0; return pBody->GetExtraPrice(this); }
inline ClassMask Item::GetClsLimit() const                                  { if (!pBody) return ClassMask(); return pBody->GetClsLimit(); }
inline int  Item::GetRefineLevel() const                                    { if (!pBody) return 0; return pBody->GetRefineLevel(this); }
inline bool Item::CanUse(Item::LOCATION l, size_t index, Unit* obj)         { if (!pBody) return false; return pBody->CanUse(l,index,obj,this); }
inline bool Item::CanRefine(Item::LOCATION l, size_t index, Unit* obj)      { if (!pBody) return false; return pBody->CanRefine(l,index,obj,this); }
inline bool Item::CanActivate(Item::LOCATION l, size_t index, Unit* obj)    { if (!pBody) return false; return pBody->CanActivate(l,index,obj,this); }
inline bool Item::CanComposite(Item::LOCATION l, size_t index, Unit* obj)   { if (!pBody) return false; return pBody->CanComposite(l,index,obj,this); }
inline void Item::RefineEquip(Item::LOCATION l, size_t index, Unit* obj)    { if (!pBody) return; pBody->RefineEquip(l,index,obj,this); }
inline bool Item::Update(Item::LOCATION l, size_t index, Unit* obj)         { if (!pBody) return false; return pBody->Update(l,index,obj,this); }
inline bool Item::Composite(Item::LOCATION l, size_t index, Unit* obj)      { if (!pBody) return false; return pBody->Composite(l,index,obj,this); }
inline void Item::OnPutIn(Item::LOCATION l, size_t index, Unit* obj)        { ASSERT(ref == 0); ++ref; if (pBody) pBody->OnPutIn(l,index,obj,this); }
inline void Item::OnTakeOut(Item::LOCATION l, size_t index, Unit* obj)      { ASSERT(ref == 1); --ref; if (pBody) pBody->OnTakeOut(l,index,obj,this); }
inline void Item::Enable(Item::LOCATION l, size_t index, Unit* obj)         { if (!pBody) return; pBody->Enable(l,index,obj,this); }
inline void Item::Disable(Item::LOCATION l, size_t index, Unit* obj)        { if (!pBody) return; pBody->Disable(l,index,obj,this); }
inline Item::ITEM_USE Item::UseItem(Item::LOCATION l, size_t index, Unit* obj) { if (!pBody) return ITEM_USE_FAILURE; return pBody->UseItem(l,index,obj,this); }
inline ItemEssence& Item::GetItemEssence() { return ess; }
inline const ItemEssence& Item::GetItemEssence() const { return ess; }

}; //namespace gamed

#endif // _GAMED_GS_ITEM_ITEM_H_
