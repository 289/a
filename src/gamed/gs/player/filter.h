#ifndef GAMED_GS_PLAYER_FILTER_H_
#define GAMED_GS_PLAYER_FILTER_H_

#include <stdint.h>
#include <map>

#include "shared/base/assertx.h"
#include "shared/base/singleton.h"
#include "shared/net/packet/packet_util.h"


namespace common {
    class FilterInfo;
} // namespace common


namespace gamed {

#define DECLARE_COMMON_FILTER(class_name, type, mask, base_class) \
public: \
    static int32_t TypeNumber() { return (int32_t)type; } \
    class_name(int32_t t) \
      : base_class(t) \
    { \
        filter_mask_ = mask; \
    } \
protected: \
	virtual base_class* Clone() \
    { \
        return static_cast<base_class*>(new class_name(type)); \
    }

#define DECLARE_FILTER(class_name, type, mask) \
    DECLARE_COMMON_FILTER(class_name, type, mask, Filter)


typedef uint64_t FilterTypeMask;

/**
 * @brief 新增的filter需要在filter.cpp里做REGISTER_FILTER
 *  （1）type一般只做新增，不做删除和修改，因为可能涉及存盘的filter
 */
// 注意以下顺序不能变！！！！！
enum FILTER_TYPE
{
	FILTER_TYPE_INVALID,
	FILTER_TYPE_INVINCIBLE,      // 无敌
	FILTER_TYPE_IMMUNE_LANDMINE, // 免疫暗雷
    FILTER_TYPE_TRANSFORM,       // 变身
    FILTER_TYPE_PROPERTY,        // 修改属性
	FILTER_TYPE_MAX,
};


/**
 * @brief filter的功能mask
 */
enum FILTER_MASK
{
    FILTER_MASK_INVALID         = 0x00000000,
    // 0
	FILTER_MASK_BUFF            = 0x00000001,
	FILTER_MASK_DEBUFF          = 0x00000002,
	FILTER_MASK_UNIQUE          = 0x00000004,
	FILTER_MASK_WEAK            = 0x00000008,
    // 4
	FILTER_MASK_SAVE_DB_DATA    = 0x00000010,
    FILTER_MASK_PRIORITY_UNIQUE = 0x00000020, // 根据优先级唯一：相同互斥组里高优先级顶替低优先级，相同优先级的后来顶替先前
};


class Player;

/**
 * @class Filter
 * @brief 玩家身上持续一段时间的效果
 */
class Filter
{
    friend class FilterMan;
public:
	Filter(Player* player, int32_t type, int32_t mask, int32_t effectid);
	virtual ~Filter();

    void    Attach();
	void    Detach();

    bool    IsInited() const         { return is_inited_;   }
	int32_t GetMask() const          { return filter_mask_; }
	int32_t GetType() const          { return filter_type_; }
	bool    IsActived() const        { return is_actived_;  }
	void    SetActived(bool flag)    { is_actived_ = flag;  }
	void    SetPlayer(Player* p)     { pplayer_ = p;        }
    void    SetTimeout(int32_t time) { timeout_ = time;     }
    bool    HasTimeout() const       { return timeout_ > 0; }
	int32_t GetTimeout() const       { return HasTimeout() ? timeout_ : 0; }
    void    SetRejectSave()          { is_reject_save_ = true; }
    bool    IsNeedSave() const;

	virtual void    Heartbeat(); // 子类继承时，必须调用父类的Heartbeat函数，定时filter才能生效
	virtual bool    Load(const common::FilterInfo& info);
	virtual bool    Save(common::FilterInfo* pinfo);
    virtual int32_t GetEffectID() const   { return effect_id_; } // world buff 才有effect-id
    virtual int32_t GetPriority() const   { return 0; }
    virtual int32_t GetMutexGroup() const { return 0; } // 获取互斥组，相同互斥组里比较优先级才有意义

    
protected:
	Filter(int32_t type);
    void    SetFilterInit(bool inited)  { is_inited_ = inited; }
    bool    CheckEffectID(int32_t effectid) const { return (effectid > 0); } 
	virtual Filter* Clone() { ASSERT(false); return NULL; }
    // subclass interface
	virtual void OnAttach() = 0;
	virtual void OnDetach() = 0;
    virtual bool OnLoad(const std::string& detail) { return false; } // 存盘数据注意分版本，不要轻易修改
	virtual bool OnSave(std::string* pdetail)      { return false; } // 存盘数据注意分版本，不要轻易修改
    virtual void OnHeartbeat()                     { }


protected:
    bool    is_inited_;

    ///
    /// filter 存盘数据的数据项
    ///
	int32_t filter_type_; // filterType
    int32_t timeout_;     // 每个buff都有超时时间，没设置则是负值
    int32_t effect_id_;   // 效果id，大世界buff才有值

    // 运行时数据
	int32_t filter_mask_; // 处理哪种内容
	bool    is_actived_;  // 是否已经激活
    bool    is_reject_save_; // 拒绝存盘，一下任务发的buff生命周期由任务管理

    ///
    /// 玩家指针
    ///
	Player* pplayer_;
};


/**
 * @class FilterMan
 * @brief Buff管理器，玩家上线加载存盘Buff时使用
 */
class FilterMan : public shared::Singleton<FilterMan>
{
	friend class shared::Singleton<FilterMan>;
public:
    static inline FilterMan* GetInstance() {
		return &(get_mutable_instance());
	}

	Filter* CreateFilter(int32_t filter_type);

protected:
    FilterMan();
	~FilterMan();

private:
	typedef std::map<int32_t, Filter*> FilterMap;
	FilterMap    prototype_map_;
};

#define s_pfilterMan gamed::FilterMan::GetInstance()

}; // namespace gamed

#endif // GAMED_GS_PLAYER_FILTER_H_
