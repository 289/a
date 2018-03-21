#ifndef LUABIND_LUA_VALUE_H
#define LUABIND_LUA_VALUE_H

#include <stdint.h>
#include <vector>
#include <string>

namespace luabind
{

class LuaValue
{
public:
	enum SV_TYPE
	{
		SVT_INVALID,
		SVT_BOOL,
		SVT_NUMBER,
		SVT_STRING,
		SVT_INT64,
	};

	LuaValue();
	LuaValue(const LuaValue& value);
	LuaValue(bool value);
	LuaValue(int value);
	LuaValue(float value);
	LuaValue(double value);
	LuaValue(const std::string& value);
	LuaValue(const char* value, size_t len);
	explicit LuaValue(int64_t value);

	inline SV_TYPE GetType() const;

	bool GetBool() const;
	int GetInt() const;
	float GetFloat() const;
	double GetDouble() const;
	std::string GetString() const;
	int64_t GetInt64() const;

	void SetValue(bool value);
	void SetValue(int value);
	void SetValue(float value);
	void SetValue(double value);
	void SetValue(const std::string& value);
	void SetValue(const char* value, size_t len);
	void SetValue(int64_t value);
private:
	SV_TYPE type_;

	union
	{
		bool bool_;
		double number_;
		int64_t id_;
	};

	std::string str_;
};

inline LuaValue::SV_TYPE LuaValue::GetType() const
{
	return type_;
}

typedef std::vector<LuaValue> LuaValueArray;

} // namespace luabind

#endif // LUABIND_LUA_VALUE_H
