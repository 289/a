#include "mysqldbm.h"
#include <vector>
#include "shared/base/conf.h"
#include "shared/base/strtoken.h"

namespace shared
{
namespace database
{

using namespace std;

MysqlDBM::DBMap MysqlDBM::s_databases_;
MysqlDBM::Table2DB MysqlDBM::s_table2db_;

Transaction::Transaction() 
	: database_(MysqlDBM::GetDataBase("0")), trans_(database_->begin())
{
}

bool MysqlDBM::Init(Conf* conf)
{
	DBInfo info;
	int32_t db_num = atoi(conf->find("DataBase", "dbnum").c_str());
	for (int32_t i = 0; i < db_num; ++i)
	{
		char buf[100];
		snprintf(buf, sizeof(buf), "DB%d", i);
		info.user_name = conf->find(buf, "username");
		info.password = conf->find(buf, "password");
		info.db_name = conf->find(buf, "dbname");
		info.host = conf->find(buf, "host");
		info.port = conf->find(buf, "port");
		string tables = conf->find(buf, "tables");

		int32_t db_id = AddDataBase(info);
		if (!AddTables(tables, db_id))
		{
			return false;
		}		
	}
	return true;
}

int32_t MysqlDBM::AddDataBase(const DBInfo& info)
{
	int32_t db_id = s_databases_.size();
	s_databases_[db_id] = new odb::mysql::database(info.user_name, info.password, info.db_name, info.host, atoi(info.port.c_str()));
	return db_id;
}

bool MysqlDBM::AddTables(string tables, int32_t db_id)
{
	const char *delim = ";,\r\n";
	StrToken token;
	vector<string> vec_tables;

	if (token.GetTokenData(tables.c_str(), delim, vec_tables))
	{		
		return false;
	}

	char buf[100] = {0};
	snprintf(buf, sizeof(buf), "%d", db_id);
	vec_tables.push_back(buf);
	for (size_t i = 0; i < vec_tables.size(); i++)
	{
		string table_name = vec_tables[i];
		s_table2db_[table_name] = db_id;
	}
	return true;
}

void MysqlDBM::Destroy()
{
	for (DBMap::iterator it = s_databases_.begin(); it != s_databases_.end(); ++it)
	{
		odb::core::database* db = it->second;
		if (db != NULL)
		{
			delete db;
			db = NULL;
		}
	}
	s_databases_.clear();
	s_table2db_.clear();
}

odb::core::database* MysqlDBM::GetDataBase(const std::string& table_name)
{
	Table2DB::iterator tit = s_table2db_.find(table_name);
	if (tit == s_table2db_.end())
	{
		return NULL;
	}
	int32_t db_id = tit->second;
	DBMap::iterator dit = s_databases_.find(db_id);
	return dit == s_databases_.end() ? NULL : dit->second;
}

} // namespace database
} // namespace shared
