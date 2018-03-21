#ifndef __GAMED_GS_ITEM_ITEM_ESS_DEF_H__
#define __GAMED_GS_ITEM_ITEM_ESS_DEF_H__

#include <stdint.h>
#include <string>
#include "shared/net/packet/bytebuffer.h"

namespace gamed
{

//动态属性类型
enum ESS_ID
{
	ESS_ID_INVALID,
	ESS_ID_REFINE,                      // 装备精练属性
	ESS_ID_PET_PROP,                    // 宠物属性
    ESS_ID_CARD,                        // 卡牌属性
	ESS_ID_MAX,
};

typedef uint32_t ESSENCE_ID;

/**
 * @brief 从这里开始为物品动态属性的结构体定义
 * @brief 动态属性结构一旦定义，禁止修改，否则会造成致命错误！！！
 * @brief 此文件为服务器和客户度共享，修改后，需要同步给客户度。
 */

//动态属性基类
class EssBase
{
public:
    EssBase(ESSENCE_ID ess_id)
        : id(ess_id)
    {
    }

    virtual int32_t LoadScalableEss(const char* buf, int32_t len) = 0;
    virtual void SaveScalableEss(std::string& content) const = 0;
    virtual void Pack(shared::net::ByteBuffer& buf) const = 0;
    virtual void UnPack(shared::net::ByteBuffer& buf) = 0;

	ESSENCE_ID id;
};

//装备精练属性
class EssRefine : public EssBase
{
public:
    EssRefine()
        : EssBase(ESS_ID_REFINE), refine_tid(0), refine_lvl(0)
    {
    }

    virtual int32_t LoadScalableEss(const char* buf, int32_t len);
    virtual void SaveScalableEss(std::string& content) const;
    virtual void Pack(shared::net::ByteBuffer& buf) const;
    virtual void UnPack(shared::net::ByteBuffer& buf);

	int32_t refine_tid;	                // 精练模板ID
	int8_t  refine_lvl;	                // 精练等级(从1开始)
};

//宠物属性
class EssPetProp : public EssBase
{
public:
    EssPetProp()
        : EssBase(ESS_ID_PET_PROP), pet_exp(0), pet_level(0), pet_blevel(0)
    {
    }

    virtual int32_t LoadScalableEss(const char* buf, int32_t len);
    virtual void SaveScalableEss(std::string& content) const;
    virtual void Pack(shared::net::ByteBuffer& buf) const;
    virtual void UnPack(shared::net::ByteBuffer& buf);

	int32_t pet_exp;                    // 宠物经验值
	int16_t pet_level;                  // 宠物普通等级
	int16_t pet_blevel;                 // 宠物血脉等级
};

// 卡牌属性
class EssCard : public EssBase
{
public:
    EssCard()
        : EssBase(ESS_ID_CARD), card_exp(0), star_id(0)
    {
    }

    virtual int32_t LoadScalableEss(const char* buf, int32_t len);
    virtual void SaveScalableEss(std::string& content) const;
    virtual void Pack(shared::net::ByteBuffer& buf) const;
    virtual void UnPack(shared::net::ByteBuffer& buf);

    int32_t card_exp;                   // 卡牌当前的经验值
    int32_t star_id;                    // 卡牌当前镶嵌的星盘ID
};

EssBase* CreateEssById(ESSENCE_ID id);
EssBase* CreateEssByName(const std::string& name);

} // namespace gamed

#endif
