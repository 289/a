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
 * @brief ������filter��Ҫ��filter.cpp����REGISTER_FILTER
 *  ��1��typeһ��ֻ������������ɾ�����޸ģ���Ϊ�����漰���̵�filter
 */
// ע������˳���ܱ䣡��������
enum FILTER_TYPE
{
	FILTER_TYPE_INVALID,
	FILTER_TYPE_INVINCIBLE,      // �޵�
	FILTER_TYPE_IMMUNE_LANDMINE, // ���߰���
    FILTER_TYPE_TRANSFORM,       // ����
    FILTER_TYPE_PROPERTY,        // �޸�����
	FILTER_TYPE_MAX,
};


/**
 * @brief filter�Ĺ���mask
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
    FILTER_MASK_PRIORITY_UNIQUE = 0x00000020, // �������ȼ�Ψһ����ͬ������������ȼ���������ȼ�����ͬ���ȼ��ĺ���������ǰ
};


class Player;

/**
 * @class Filter
 * @brief ������ϳ���һ��ʱ���Ч��
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

	virtual void    Heartbeat(); // ����̳�ʱ��������ø����Heartbeat��������ʱfilter������Ч
	virtual bool    Load(const common::FilterInfo& info);
	virtual bool    Save(common::FilterInfo* pinfo);
    virtual int32_t GetEffectID() const   { return effect_id_; } // world buff ����effect-id
    virtual int32_t GetPriority() const   { return 0; }
    virtual int32_t GetMutexGroup() const { return 0; } // ��ȡ�����飬��ͬ��������Ƚ����ȼ���������

    
protected:
	Filter(int32_t type);
    void    SetFilterInit(bool inited)  { is_inited_ = inited; }
    bool    CheckEffectID(int32_t effectid) const { return (effectid > 0); } 
	virtual Filter* Clone() { ASSERT(false); return NULL; }
    // subclass interface
	virtual void OnAttach() = 0;
	virtual void OnDetach() = 0;
    virtual bool OnLoad(const std::string& detail) { return false; } // ��������ע��ְ汾����Ҫ�����޸�
	virtual bool OnSave(std::string* pdetail)      { return false; } // ��������ע��ְ汾����Ҫ�����޸�
    virtual void OnHeartbeat()                     { }


protected:
    bool    is_inited_;

    ///
    /// filter �������ݵ�������
    ///
	int32_t filter_type_; // filterType
    int32_t timeout_;     // ÿ��buff���г�ʱʱ�䣬û�������Ǹ�ֵ
    int32_t effect_id_;   // Ч��id��������buff����ֵ

    // ����ʱ����
	int32_t filter_mask_; // ������������
	bool    is_actived_;  // �Ƿ��Ѿ�����
    bool    is_reject_save_; // �ܾ����̣�һ�����񷢵�buff�����������������

    ///
    /// ���ָ��
    ///
	Player* pplayer_;
};


/**
 * @class FilterMan
 * @brief Buff��������������߼��ش���Buffʱʹ��
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
