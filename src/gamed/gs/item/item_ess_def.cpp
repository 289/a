#include "item_ess_def.h"
#include "shared/net/buffer.h"
#include "common/obj_data/gen/player/item_essence.pb.h"

namespace gamed
{

#define DECODE(value) \
    if (tmp.has_##value()) \
    { \
        value = tmp.value(); \
    }

#define ENCODE(value) tmp.set_##value(value);

EssBase* CreateEssById(ESSENCE_ID id)
{
    EssBase* ess = NULL;
    switch (id)
    {
    case ESS_ID_REFINE:
        ess = new EssRefine;
        break;
    case ESS_ID_PET_PROP:
        ess = new EssPetProp;
        break;
    case ESS_ID_CARD:
        ess = new EssCard;
        break;
    default:
        break;
    }
    return ess;
}

EssBase* CreateEssByName(const std::string& name)
{
    EssBase* ess = NULL;
    if (name == "common.scalable.EssRefine")
    {
        ess = new EssRefine;
    }
    else if (name == "common.scalable.EssPetProp")
    {
        ess = new EssPetProp;
    }
    else if (name == "common.scalable.EssCard")
    {
        ess = new EssCard;
    }
    return ess;
}

int32_t EssRefine::LoadScalableEss(const char* buf, int32_t len)
{
    common::scalable::EssRefine tmp;
    if (!tmp.ParseFromArray(buf, len))
    {
        return 0;
    }
    DECODE(refine_tid)
    DECODE(refine_lvl)
    return tmp.ByteSize();
}

void EssRefine::SaveScalableEss(std::string& content) const
{
    common::scalable::EssRefine tmp;
    ENCODE(refine_tid)
    ENCODE(refine_lvl)
    std::string data;
    data.resize(tmp.ByteSize());
    if (!tmp.SerializeToArray((void*)data.data(), data.size()))
    {
        return;
    }
	const std::string& typeName = tmp.GetTypeName();
    shared::net::Buffer buf;
    int32_t nameLen = typeName.size() + 1;
    buf.AppendInt32(nameLen);
    buf.Append(typeName.c_str(), nameLen);
    buf.Append(data.c_str(), data.size());
    content = buf.RetrieveAllAsString();
}

void EssRefine::Pack(shared::net::ByteBuffer& buf) const
{
    buf << refine_tid << refine_lvl;
}

void EssRefine::UnPack(shared::net::ByteBuffer& buf)
{
    buf >> refine_tid >> refine_lvl;
}

int32_t EssPetProp::LoadScalableEss(const char* buf, int32_t len)
{
    common::scalable::EssPetProp tmp;
    if (!tmp.ParseFromArray(buf, len))
    {
        return 0;
    }
    DECODE(pet_exp)
    DECODE(pet_level)
    DECODE(pet_blevel)
    return tmp.ByteSize();
}

void EssPetProp::SaveScalableEss(std::string& content) const
{
    common::scalable::EssPetProp tmp;
    ENCODE(pet_exp)
    ENCODE(pet_level)
    ENCODE(pet_blevel)
    std::string data;
    data.resize(tmp.ByteSize());
    if (!tmp.SerializeToArray((void*)data.data(), data.size()))
    {
        return;
    }
	const std::string& typeName = tmp.GetTypeName();
    shared::net::Buffer buf;
    int32_t nameLen = typeName.size() + 1;
    buf.AppendInt32(nameLen);
    buf.Append(typeName.c_str(), nameLen);
    buf.Append(data.c_str(), data.size());
    content = buf.RetrieveAllAsString();
}

void EssPetProp::Pack(shared::net::ByteBuffer& buf) const
{
    buf << pet_exp << pet_level << pet_blevel;
}

void EssPetProp::UnPack(shared::net::ByteBuffer& buf)
{
    buf >> pet_exp >> pet_level >> pet_blevel;
}

int32_t EssCard::LoadScalableEss(const char* buf, int32_t len)
{
    common::scalable::EssCard tmp;
    if (!tmp.ParseFromArray(buf, len))
    {
        return 0;
    }
    DECODE(card_exp)
    DECODE(star_id)
    return tmp.ByteSize();
}

void EssCard::SaveScalableEss(std::string& content) const
{
    common::scalable::EssCard tmp;
    ENCODE(card_exp)
    ENCODE(star_id)
    std::string data;
    data.resize(tmp.ByteSize());
    if (!tmp.SerializeToArray((void*)data.data(), data.size()))
    {
        return;
    }
	const std::string& typeName = tmp.GetTypeName();
    shared::net::Buffer buf;
    int32_t nameLen = typeName.size() + 1;
    buf.AppendInt32(nameLen);
    buf.Append(typeName.c_str(), nameLen);
    buf.Append(data.c_str(), data.size());
    content = buf.RetrieveAllAsString();
}

void EssCard::Pack(shared::net::ByteBuffer& buf) const
{
    buf << card_exp << star_id;
}

void EssCard::UnPack(shared::net::ByteBuffer& buf)
{
    buf >> card_exp >> star_id;
}

} // namespace gamed
