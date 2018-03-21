#ifndef SHARED_DATABASE_MYSQLDBM_H
#define SHARED_DATABASE_MYSQLDBM_H

#include <string>
#include <map>
#include <vector>
#include <odb/database.hxx>
#include <odb/mysql/database.hxx>

namespace shared
{

class Conf;

namespace database
{

enum DB_ERROR
{
	DB_NOERROR = 0,
	DB_OBJECT_EXIST,
	DB_OBJECT_NOTEXIST,
	DB_BUSY,
	DB_TABLEUNKNOW,
	DB_UNKNOW,
};

inline DB_ERROR DBError(const odb::exception& e)
{
	std::string msg = e.what();
	if (msg == "object already persistent")
	{
		return DB_OBJECT_EXIST;
	}
	else if (msg == "object not persistent")
	{
		return DB_OBJECT_NOTEXIST;
	}
	else if (msg.find("Duplicate") != std::string::npos)
	{
		return DB_OBJECT_EXIST;
	}
	else
	{
		return DB_UNKNOW;
	}
}

class Transaction
{
public:
	Transaction();

	inline odb::core::database* database() const;
	inline void Commit();
	inline void RollBack();
private:
	odb::core::database* database_;
	odb::core::transaction trans_;
};

class MysqlDBM
{
	enum DB_OP
	{
		DB_INSERT = 0,
		DB_UPDATE,
		DB_DELETE_OBJECT,
		DB_DELETE_ID,
		DB_REPLACE,
	};

	struct DBInfo
	{
		std::string user_name;
		std::string password;
		std::string db_name;
		std::string host;
		std::string port;
	};
public:
	static bool Init(Conf* conf);
	static void Destroy();
	static odb::core::database* GetDataBase(const std::string& table_name);

	// 数据库操作函数
	template <typename T>
	static void Insert(T& object, const Transaction& trans);
	template <typename T>
	static void Insert(std::vector<T>& objects, const Transaction& trans);

	template <typename T>
	static void Update(T& object, const Transaction& trans);

	template <typename T>
	static void Delete(T& object, const Transaction& trans);
	template <typename T>
	static void DeleteAll(int64_t id, const Transaction& trans);
	template <typename T>
	static void Delete(const odb::query<T>& q, const Transaction& trans);

	template <typename T>
	static void Replace(int64_t id, std::vector<T>& object, const Transaction& trans);

	template <typename T>
	static void Query(odb::query<T>& q, std::vector<T>& vec, const Transaction& trans);
private:
	static int32_t AddDataBase(const DBInfo& info);
	static bool AddTables(std::string tables, int32_t db_id);

	template <typename T>
	static void Operate(DB_OP op, T& object, const Transaction& trans);
private:
	typedef std::map<int32_t/*db_id*/, odb::core::database*> DBMap;
	typedef std::map<std::string/*table_name*/, int32_t/*db_id*/> Table2DB;

	static DBMap s_databases_;
	static Table2DB s_table2db_;
};

inline odb::core::database* Transaction::database() const
{
	return database_;
}

inline void Transaction::Commit()
{
	trans_.commit();
}

inline void Transaction::RollBack()
{
	trans_.rollback();
}

template <typename T>
void MysqlDBM::Insert(T& object, const Transaction& trans)
{
	return Operate(DB_INSERT, object, trans);
}

template <typename T>
void MysqlDBM::Insert(std::vector<T>& objects, const Transaction& trans)
{
	typename std::vector<T>::iterator it = objects.begin();
	for (; it != objects.end(); ++it)
	{
		Insert(*it, trans);
	}
}

template <typename T>
void MysqlDBM::Update(T& object, const Transaction& trans)
{
	return Operate(DB_UPDATE, object, trans);
}

template <typename T>
void MysqlDBM::Delete(T& object, const Transaction& trans)
{
	return Operate(DB_DELETE_OBJECT, object, trans);
}

template <typename T>
void MysqlDBM::DeleteAll(int64_t id, const Transaction& trans)
{
	odb::core::database* db = trans.database();
	db->erase<T>(id);
}

template <typename T>
void MysqlDBM::Delete(const odb::query<T>& q, const Transaction& trans)
{
	odb::core::database* db = trans.database();
	db->erase_query<T>(q);
}

template <typename T>
void MysqlDBM::Replace(int64_t id, std::vector<T>& objects, const Transaction& trans)
{
	try
	{
		DeleteAll<T>(id, trans);
		Insert(objects, trans);
	}
	catch(const odb::object_not_persistent& e)
	{
		Insert(objects, trans);
	}
}

template <typename T>
void MysqlDBM::Operate(DB_OP op, T& object, const Transaction& trans)
{
	odb::core::database* db = trans.database();
	switch (op)
	{
	case DB_INSERT:
		db->persist(object);
		break;
	case DB_UPDATE:
		db->update(object);
		break;
	case DB_DELETE_OBJECT:
		db->erase(object);
		break;
	default:
		break;
	}
}

template <typename T>
void MysqlDBM::Query(odb::query<T>& q, std::vector<T>& vec, const Transaction& trans)
{
	odb::core::database* db = trans.database();
	odb::result<T> r(db->query<T>(q));
	for (typename odb::result<T>::iterator it(r.begin()); it != r.end(); ++it)
	{
		vec.push_back(*it);
	}
}

} // namespace database

} // namespace shared

#endif // SHARED_DATABASE_MYSQLMANAGER_H
