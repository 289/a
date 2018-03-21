#include "item_essence.h"
#include "shared/base/base_define.h"
#include "shared/net/buffer.h"

namespace gamed
{

ItemEssence::ItemEssence()
{
}

ItemEssence::~ItemEssence()
{
    Clear();
}

ItemEssence& ItemEssence::operator=(const ItemEssence& rhs)
{
    std::string content;
    rhs.SaveEss(content);
    LoadEss(content);
    return *this;
}

void ItemEssence::Clear()
{
	for (size_t i = 0; i < ess_vec_.size(); ++ i)
	{
        SAFE_DELETE(ess_vec_[i]);
	}
	ess_vec_.clear();
}

void ItemEssence::Pack(shared::net::ByteBuffer& buf) const
{
    int32_t size = ess_vec_.size();
    buf << size;
    for (int32_t i = 0; i < size; ++i)
    {
        buf << ess_vec_[i]->id;
        ess_vec_[i]->Pack(buf);
    }
}

void ItemEssence::UnPack(shared::net::ByteBuffer& buf)
{
    Clear();
    int32_t size = 0;
    buf >> size;
    ess_vec_.resize(size);
    for (int32_t i = 0; i < size; ++i)
    {
        ESSENCE_ID id = 0;
        buf >> id;
        ess_vec_[i] = CreateEssById(id);
        ess_vec_[i]->UnPack(buf);
    }
}

void ItemEssence::LoadScalableEss(const std::string& content)
{
    if (content.empty())
    {
        return;
    }
    const char* buf = content.c_str();
    int32_t len = content.size();
    // 动态属性的条目
    int32_t size = 0;
    ::memcpy(&size, buf, sizeof(int32_t));
    size = be32toh(size);
    ess_vec_.resize(size);
    buf += sizeof(int32_t);
    len -= sizeof(int32_t);
    const int32_t kNamelenLen = sizeof(int32_t);
    for (int32_t i = 0; i < size; ++i)
    {
        int32_t nameLen = 0;
        ::memcpy(&nameLen, buf, kNamelenLen); 
        nameLen = be32toh(nameLen);
        buf += kNamelenLen;
        len -= kNamelenLen;
        ess_vec_[i] = CreateEssByName(std::string(buf, buf + nameLen - 1));
        buf += nameLen;
        len -= nameLen;
        int32_t offset = ess_vec_[i]->LoadScalableEss(buf, len);
        buf += offset;
    }
}

void ItemEssence::SaveScalableEss(std::string& content) const
{
    if (ess_vec_.empty())
    {
        return;
    }
    int32_t size = ess_vec_.size();
    shared::net::Buffer buf;
    buf.AppendInt32(size);
    for (int32_t i = 0; i < size; ++i)
    {
        std::string tmp_content;
        ess_vec_[i]->SaveScalableEss(tmp_content);
        buf.Append(tmp_content.c_str(), tmp_content.size());
    }
    content = buf.RetrieveAllAsString();
}

void ItemEssence::LoadEss(const std::string& content)
{
    if (content.empty())
    {
        return;
    }
    shared::net::ByteBuffer buf;
    buf.append(content.c_str(), content.size());
    UnPack(buf);
}

void ItemEssence::SaveEss(std::string& content) const
{
    if (ess_vec_.empty())
    {
        return;
    }
    shared::net::ByteBuffer buf;
    Pack(buf);
    content.append((const char*)buf.contents(), buf.size());
}

void ItemEssence::InsertEss(EssBase* pEss)
{
    ASSERT(pEss);
	EssVector::const_iterator it = std::find_if(ess_vec_.begin(), ess_vec_.end(), EssFinder(pEss->id));
	if (it != ess_vec_.end())
	{
		ASSERT(false);
	}
	ess_vec_.push_back(pEss);
}

void ItemEssence::DeleteEss(ESSENCE_ID ess_id)
{
	EssVector::iterator it = std::find_if(ess_vec_.begin(), ess_vec_.end(), EssFinder(ess_id));
	if (it == ess_vec_.end())
	{
		ASSERT(false);
	}

	SAFE_DELETE(*it);
	ess_vec_.erase(it);
}

} // namespace gamed
