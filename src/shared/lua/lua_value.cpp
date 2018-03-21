#include "lua_value.h"
#include <assert.h>

namespace luabind
{

LuaValue::LuaValue() 
	: type_(SVT_INVALID)
{
}

LuaValue::LuaValue(const LuaValue& value)
	: type_(value.type_), id_(value.id_), str_(value.str_)
{
}

LuaValue::LuaValue(bool value) 
	: type_(SVT_BOOL), bool_(value)
{
}

LuaValue::LuaValue(int value) 
	: type_(SVT_NUMBER), number_(value)
{
}

LuaValue::LuaValue(float value) 
	: type_(SVT_NUMBER), number_(value)
{
}

LuaValue::LuaValue(double value) 
	: type_(SVT_NUMBER), number_(value)
{
}

LuaValue::LuaValue(const std::string& value) 
	: type_(SVT_STRING), str_(value)
{
}

LuaValue::LuaValue(const char* value, size_t len)
	: type_(SVT_STRING), str_(value, len)
{
}

LuaValue::LuaValue(int64_t value)
	: type_(SVT_INT64), id_(value)
{
}

bool LuaValue::GetBool() const
{
	assert(type_ == SVT_BOOL);
	return bool_;
}

int LuaValue::GetInt() const
{
	assert(type_ == SVT_NUMBER);
	return (int)number_;
}

float LuaValue::GetFloat() const
{
	assert(type_ == SVT_NUMBER);
	return (float)number_;
}

double LuaValue::GetDouble() const
{
	assert(type_ == SVT_NUMBER);
	return number_;
}

std::string LuaValue::GetString() const
{
	assert(type_ == SVT_STRING);
	return str_;
}

int64_t LuaValue::GetInt64() const
{
	assert(type_ == SVT_INT64);
	return id_;
}

void LuaValue::SetValue(bool value)
{
	type_ = SVT_BOOL;
	bool_ = value;
}

void LuaValue::SetValue(int value)
{
	type_ = SVT_NUMBER;
	number_ = value;
}

void LuaValue::SetValue(float value)
{
	type_ = SVT_NUMBER;
	number_ = value;
}

void LuaValue::SetValue(double value)
{
	type_ = SVT_NUMBER;
	number_ = value;
}

void LuaValue::SetValue(const std::string& value)
{
	type_ = SVT_STRING;
	str_ = value;
}

void LuaValue::SetValue(const char* value, size_t len)
{
	type_ = SVT_STRING;
	str_.assign(value, len);
}

void LuaValue::SetValue(int64_t value)
{
	type_ = SVT_INT64;
	id_ = value;
}

} // namespace luabind
