#include "sys_mail_config.h"

#include "common/3rd/pugixml/inc/pugixml.hpp"
#include "shared/logsys/logging.h"

#include "dbgprt.h"


namespace gamed {

using namespace pugi;

SysMailConfig::SysMailConfig()
{
}

SysMailConfig::~SysMailConfig()
{
}

bool SysMailConfig::Init(const char* path)
{
    if (path == NULL)
        return true;

    xml_document doc;
    xml_parse_result result = doc.load_file(path);
    if (!result)
    {
        __PRINTF("SysMail config XML load_file error! %s", path);
        return false;
    }

    // foreach category
    xml_node mailNode = doc.child("SysMail");
    for (; mailNode; mailNode = mailNode.next_sibling("Category"))
    {
        int64_t mailId      = mailNode.attribute("id").as_int();
        xml_node formatNode = mailNode.child("Format");

        SysMailCfgMap::iterator it = sys_mail_map_.find(mailId);
        if (it != sys_mail_map_.end())
        {
            __PRINTF("SysMail config has repeat mail id! %s", path);
            return false;
        }

        sys_mail_map_[mailId].sender  = formatNode.attribute("sender").as_string();
        sys_mail_map_[mailId].title   = formatNode.attribute("title").as_string();
        sys_mail_map_[mailId].content = formatNode.attribute("content").as_string();
    }

    // check default mail format
    if (!CheckDefaultMail())
        return false;

    return true;
}

bool SysMailConfig::GetMailFormat(int64_t id, Entry& ent) const
{
    SysMailCfgMap::const_iterator it = sys_mail_map_.find(id);
    if (it == sys_mail_map_.end())
    {
        // not found
        ent.sender  = "system";
        ent.title   = "Default";
        ent.content = "This is a default system mail, and then some errors occur!";
        LOG_WARN << "SysMail 系统发邮件没找到对应id的邮件格式！mailid：" << id;
        return false;
    }

    ent = it->second;
    return true;
}

bool SysMailConfig::CheckDefaultMail() const
{
    Entry ent;
    if (!GetMailFormat(SMT_WORLD_BOSS, ent))
    {
        __PRINTF("SysMail 世界BOSS的默认邮件格式不存在！mailid:%d", SMT_WORLD_BOSS);
        return false;
    }

    return true;
}

void SysMailConfig::GetWorldBossMail(int64_t id, Entry& ent) const
{
    if (!GetMailFormat(id, ent))
    {
        GetMailFormat(SMT_WORLD_BOSS, ent);
    }
}

} // namespace gamed
