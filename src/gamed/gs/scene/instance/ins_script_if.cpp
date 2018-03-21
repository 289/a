#include "ins_script_if.h"

#include "shared/lua/lua_value.h"
#include "shared/logsys/logging.h"
#include "gs/global/game_lua_ent.h"
#include "gs/global/game_util.h"
#include "gs/template/data_templ/templ_manager.h"
#include "gs/template/data_templ/instance_templ.h"

#include "ins_func_table.h"


namespace gamed {

using namespace luabind;
using namespace dataTempl;

namespace {

	static const size_t kMaxInsLuaEntCount = 1;//16;
	static const char* kScriptFilePrefix   = "instance_";
	static const char* kScriptFileSuffix   = ".lua";
	static size_t lua_entity_rr_key        = 0;
	static shared::net::FixedArray<GameLuaEntity, kMaxInsLuaEntCount> g_ins_lua_ent;

	GameLuaEntity* GetLuaEntityRR()
	{
		return &g_ins_lua_ent[++lua_entity_rr_key % kMaxInsLuaEntCount];
	}
	
	class InitLuaEntity
	{
	public:
		InitLuaEntity()
		{
			for (size_t i = 0; i < kMaxInsLuaEntCount; ++i)
			{
				g_ins_lua_ent[i].Init(insScript::ins_script_func_table, insScript::func_table_size);
			}
		}
	};

	InitLuaEntity initObj;

    // script function
    struct func_node_t
    {
        int func_type;
        const char* name;
    };

    // 需要和枚举的顺序一致
    static func_node_t func_list[] =
    {
        { InsScriptIf::INS_START,            "instanceStart"     },
        { InsScriptIf::PLAYER_ENTER,         "playerEnter"       },
        { InsScriptIf::MONSTER_DEAD,         "monsterDead"       },
        { InsScriptIf::COUNTER_CHANGE,       "counterChange"     },
        { InsScriptIf::CLOCK_TIMEUP,         "clockTimeUp"       },
        { InsScriptIf::PLAYER_GATHER_MINE,   "playerGatherMine"  },
        { InsScriptIf::REACH_DESTINATION,    "reachDestination"  },
        { InsScriptIf::SIFT_MAX,             NULL                }
    };

} // Anonymous


///
/// static function
///
void InsScriptIf::InitScriptSys(const char* script_dir)
{
    std::vector<const InstanceTempl*> ins_templ_vec;
    s_pDataTempl->QueryDataTemplByType<InstanceTempl>(ins_templ_vec);
    for (size_t i = 0; i < ins_templ_vec.size(); ++i)
    {
        const InstanceTempl* ptpl = ins_templ_vec[i];
        if (ptpl->ins_script_id > 0)
        {
            std::string file = script_dir + std::string(kScriptFilePrefix) 
                + itos(ptpl->ins_script_id) + kScriptFileSuffix;
            for (size_t j = 0; j < kMaxInsLuaEntCount; ++j)
            {
                if (!g_ins_lua_ent[j].LoadFile(file.c_str()))
                    break;
            }
        }
    }
}


///
/// InsScriptIf
///
InsScriptIf::InsScriptIf()
	: world_xid_(),
	  script_id_(0),
      lua_ent_(NULL)
{
}

InsScriptIf::~InsScriptIf()
{
    SAFE_DELETE(lua_ent_);
}

bool InsScriptIf::Init(const XID& world_xid, int32_t script_id, const char* script_dir)
{
	world_xid_  = world_xid;
	script_id_  = script_id;

	if (script_id > 0)
	{
		script_file_ = script_dir + std::string(kScriptFilePrefix) + itos(script_id) + kScriptFileSuffix;
		if (!touchFile(script_file_.c_str()))
		{
			LOG_WARN << "副本:" << MAP_ID(world_xid_) << " script_id:" << script_id
				<< " 脚本文件没有找到！";
			script_file_.clear();
		}
        else // script file found
        {
            for (int i = 0; i < SIFT_MAX; ++i)
            {
                script_func_[i] = func_list[i].name + std::string("_") + itos(script_id);
            }

            if (!InitLuaScript(script_file_))
            {
                LOG_ERROR << "副本:" << MAP_ID(world_xid_) << " script_id:" << script_id
                    << " lua脚本初始化失败！";
                script_file_.clear();
            }
        }
	}
	
	return true;
}

bool InsScriptIf::InitLuaScript(const std::string& script_file)
{
    if (!script_file.empty())
    {
        lua_ent_ = new GameLuaEntity();
        if (!lua_ent_->LoadFile(script_file.c_str()))
            return false;
        if (!lua_ent_->Init(insScript::ins_script_func_table, insScript::func_table_size))
            return false;
    }
    return true;
}

void InsScriptIf::InstanceStart()
{
	if (!has_script())
		return;

	//GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	if (!guard.lua_engine()->Call(script_func_[INS_START].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! InstanceStart()";
		return;
	}
}

void InsScriptIf::PlayerEnter(int64_t role_id)
{
	if (!has_script())
		return;

	//GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	args.push_back(LuaValue(role_id));
	if (!guard.lua_engine()->Call(script_func_[PLAYER_ENTER].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! PlayerEnter()";
		return;
	}
}

void InsScriptIf::MonsterDead(int32_t monster_tid, int32_t count)
{
	if (!has_script())
		return;

	//GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	args.push_back(LuaValue(monster_tid));
    args.push_back(LuaValue(count));
	if (!guard.lua_engine()->Call(script_func_[MONSTER_DEAD].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! MonsterDead()";
		return;
	}
}

void InsScriptIf::CounterChange(int32_t index, int32_t cur_value)
{
	if (!has_script())
		return;

	//GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	args.push_back(LuaValue(index));
	args.push_back(LuaValue(cur_value));
	if (!guard.lua_engine()->Call(script_func_[COUNTER_CHANGE].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! CounterChange()";
		return;
	}
}

void InsScriptIf::ClockTimeUp(int32_t index)
{
	if (!has_script())
		return;

	//GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	args.push_back(LuaValue(index));
	if (!guard.lua_engine()->Call(script_func_[CLOCK_TIMEUP].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! ClockTimeUp()";
		return;
	}
}

void InsScriptIf::PlayerGatherMine(int64_t roleid, int32_t mine_tid)
{
    if (!has_script())
        return;

    //GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
	args.push_back(LuaValue(roleid));
    args.push_back(LuaValue(mine_tid));
	if (!guard.lua_engine()->Call(script_func_[PLAYER_GATHER_MINE].c_str(), &args))
	{
		LOG_ERROR << "LuaEngine::Call Error! PlayerGatherMine()";
		return;
	}
}

void InsScriptIf::ReachDestination(int32_t elem_id, const A2DVECTOR& pos)
{
    if (!has_script())
        return;

    //GameLuaEntity* lua_ent = GetLuaEntityRR();
	// lock
	GameLuaEntityLockGuard guard(*lua_ent_);
	LuaValueArray args;
	args.push_back(LuaValue(world_xid_.id));
    args.push_back(LuaValue(elem_id));
    args.push_back(LuaValue(pos.x));
    args.push_back(LuaValue(pos.y));
    if (!guard.lua_engine()->Call(script_func_[REACH_DESTINATION].c_str(), &args))
    {
        LOG_ERROR << "LuaEngine::Call Error! ReachDestination()";
        return;
    }
}

} // namespace gamed
