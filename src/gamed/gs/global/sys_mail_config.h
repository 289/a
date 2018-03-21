#ifndef GAMED_GS_GLOBAL_SYS_MAIL_CONFIG_H_
#define GAMED_GS_GLOBAL_SYS_MAIL_CONFIG_H_

#include <stdint.h>
#include <string>
#include <map>

#include "shared/base/singleton.h"


namespace gamed {

class SysMailConfig : public shared::Singleton<SysMailConfig>
{
    friend class shared::Singleton<SysMailConfig>;
public:
    struct Entry
    {
        std::string sender;
        std::string title;
        std::string content;
    };

    // 定义的类型需要在CheckDefaultMail()里做检查
    enum SysMailType
    {
        SMT_WORLD_BOSS = 1, // 世界BOSS的默认系统邮件格式
    };

public:
    static inline SysMailConfig* GetInstance() {
		return &(get_mutable_instance());
	}

    bool Init(const char* path);
    void GetWorldBossMail(int64_t id, Entry& ent) const;


protected:
    SysMailConfig();
    ~SysMailConfig();
    
    bool GetMailFormat(int64_t id, Entry& ent) const;
    bool CheckDefaultMail() const;


private:
    // id对应SysMailType枚举，或者直接填模板id
    typedef std::map<int64_t/*id*/, Entry> SysMailCfgMap;
    SysMailCfgMap sys_mail_map_;
};


#define s_pSysMailCfg gamed::SysMailConfig::GetInstance()

} // namespace gamed

#endif // GAMED_GS_GLOBAL_SYS_MAIL_CONFIG_H_
