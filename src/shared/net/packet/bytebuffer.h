#ifndef SHARED_NET_PACKET_BYTEBUFFER_H_
#define SHARED_NET_PACKET_BYTEBUFFER_H_

#include <assert.h>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <bitset>
#include <string>
#include <string.h> //for strlen, memcpy

#include "shared/base/exception.h"
#include "shared/base/copyable.h"

#include "shared/net/packet/platform_define.h"
#include "shared/net/packet/platform_types.h"
#include "shared/net/packet/byte_converter.h"


namespace shared {
namespace net {

template <class T>
struct Unused
{
	Unused() { }
};

///
/// ByteBuffer : base class
///
class ByteBuffer
{
public:
	const static size_t kDefaultSize   = 64;
	const static size_t kMaxBitsetSize = 1024*1024;

	ByteBuffer()
		: rpos_(0),
		  wpos_(0),
		  bitpos_(8),
		  curbitval_(0)
	{
		storage_.reserve(kDefaultSize);
	}

	ByteBuffer(size_t res)
		: rpos_(0),
		  wpos_(0),
		  bitpos_(8),
		  curbitval_(0)
	{
		storage_.reserve(res);
	}

	ByteBuffer(const ByteBuffer& buf)
		: rpos_(buf.rpos_),
		  wpos_(buf.wpos_),
		  bitpos_(buf.bitpos_),
		  curbitval_(buf.curbitval_),
		  storage_(buf.storage_)
	{ }

	void clear()
	{
		storage_.clear();
		rpos_		= 0;
		wpos_		= 0;
		curbitval_	= 0;
		bitpos_		= 8;
	}

	void free()
	{
		clear();
		std::vector<uint8_t> empty_vec;
		storage_.swap(empty_vec);
	}

	///
	/// unsigned integer as in 1st complement
	///
	ByteBuffer& operator<<(bool value)
	{
		append<uint8_t>(static_cast<uint8_t>(value));
		return *this;
	}

	ByteBuffer& operator<<(uint8_t value)
	{
		append<uint8_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(uint16_t value)
	{
		append<uint16_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(uint32_t value)
	{
		append<uint32_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(uint64_t value)
	{
		append<uint64_t>(value);
		return *this;
	}

	///
	/// signed integer as in 2e complement
	///
	ByteBuffer& operator<<(int8_t value)
	{
		append<int8_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(int16_t value)
	{
		append<int16_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(int32_t value)
	{
		append<int32_t>(value);
		return *this;
	}

	ByteBuffer& operator<<(int64_t value)
	{
		append<int64_t>(value);
		return *this;
	}

	/// 
	/// floating points
	///
	ByteBuffer& operator<<(float value)
	{
		append<float>(value);
		return *this;
	}

	ByteBuffer& operator<<(double value)
	{
		append<double>(value);
		return *this;
	}

	///
	/// string types
	///
	ByteBuffer& operator<<(const std::string& value)
	{
		append(static_cast<uint32_t>(value.size()));
		append(static_cast<const char*>(value.c_str()), value.size());
		return *this;
	}

	ByteBuffer& operator<<(std::string& value)
	{
		append(static_cast<uint32_t>(value.size()));
		append(static_cast<const char*>(value.c_str()), value.size());
		return *this;
	}

	/*
	ByteBuffer& operator<<(const char* str)
	{
		append(static_cast<const char*>(str), str ? strlen(str):0);
		append(static_cast<uint8_t>(0));
		return *this;
	}
	*/

	///
	/// operator >>
	///
	/// integer
	ByteBuffer& operator>>(bool& value)
	{
		value = (read<uint8_t>() != 0) ? true : false;
		return *this;
	}

	ByteBuffer& operator>>(uint8_t& value)
	{
		value = read<uint8_t>();
		return *this;
	}

	ByteBuffer& operator>>(uint16_t& value)
	{
		value = read<uint16_t>();
		return *this;
	}

	ByteBuffer& operator>>(uint32_t& value)
	{
		value = read<uint32_t>();
		return *this;
	}

	ByteBuffer& operator>>(uint64_t& value)
	{
		value = read<uint64_t>();
		return *this;
	}

	/// signed as in 2e complement
	ByteBuffer& operator>>(int8_t& value)
	{
		value = read<int8_t>();
		return *this;
	}

	ByteBuffer& operator>>(int16_t& value)
	{
		value = read<int16_t>();
		return *this;
	}

	ByteBuffer& operator>>(int32_t& value)
	{
		value = read<int32_t>();
		return *this;
	}

	ByteBuffer& operator>>(int64_t& value)
	{
		value = read<int64_t>();
		return *this;
	}

	///
	/// operator >>
	/// 
	/// floating points
	ByteBuffer& operator>>(float& value)
	{
		value = read<float>();
		return *this;
	}

	ByteBuffer& operator>>(double& value)
	{
		value = read<double>();
		return *this;
	}

	/// operator >>
	///
	/// string types
	ByteBuffer& operator>>(std::string& value)
	{
		value.clear();
		uint32_t str_size = read<uint32_t>();
		uint32_t start    = rpos();
		// TODO: merge with readString()
		while (rpos() < size() && rpos() < start + str_size) // prevent crash at wrong string format in packet
		{
			value += read<char>();
		}
		return *this;
	}

	template <class T>
	ByteBuffer& operator>>(const Unused<T>&)
	{
		return readSkip<T>();
	}

	///
	/// operator []
	///
	uint8_t& operator[](const size_t pos)
	{
		if (pos >= size())
		{
			throw Exception(std::string("ByteBuffer - uint8_t& operator[](const size_t pos)"));
		}

		return storage_[pos];
	}

	const uint8_t& operator[](const size_t pos) const
	{
		if (pos >= size())
		{
			throw Exception(std::string("ByteBuffer - const uint8_t& operator[](const size_t pos) const"));
		}

		return storage_[pos];
	}

	///
	/// rpos_, wpos_  opfunc
	///
	size_t rpos() const 
	{ 
		return rpos_; 
	}

	size_t set_rpos(size_t rpos)
	{
		rpos_ = rpos;
		return rpos_;
	}

	void rfinish()
	{
		rpos_ = wpos();
	}

	size_t wpos() const 
	{ 
		return wpos_; 
	}

	size_t set_wpos(size_t wpos)
	{
		wpos_ = wpos;
		return wpos_;
	}
		
	///
	/// read integer
	///
	uint8_t readUInt8()
	{
		uint8_t u = 0;
		(*this) >> u;
		return u;
	}

	uint16_t readUInt16()
	{
		uint16_t u = 0;
		(*this) >> u;
		return u;
	}

	uint32_t readUInt32()
	{
		uint32_t u = 0;
		(*this) >> u;
		return u;
	}

	uint64_t readUInt64()
	{
		uint64_t u = 0;
		(*this) >> u;
		return u;
	}

	int8_t readInt8()
	{
		int8_t u = 0;
		(*this) >> u;
		return u;
	}

	int16_t readInt16()
	{
		int16_t u = 0;
		(*this) >> u;
		return u;
	}

	int32_t readInt32()
	{
		uint32_t u = 0;
		(*this) >> u;
		return u;
	}

	int64_t readInt64()
	{
		int64_t u = 0;
		(*this) >> u;
		return u;
	}

	bool readBoolean()
	{
		uint8_t b = 0;
		(*this) >> b;
		return b > 0 ? true:false;
	}

	float readFloat()
	{
		float f = 0;
		(*this) >> f;
		return f;
	}
	
	///
	/// peek
	///
	int32_t PeekInt32() const
	{
		if (size() - rpos() < sizeof(int32_t))
		{
			throw Exception(std::string("ByteBuffer - int32_t PeekInt32() const"));
		}
		int32_t be32 = 0;
		::memcpy(&be32, contents() + rpos(), sizeof be32);
		return be32;
	}

	const uint8_t* contents() const 
	{ 
		if (empty())
		{
			return NULL;
		}

		return &storage_[0]; 
	}

	size_t size() const 
	{ 
		return storage_.size(); 
	}

	bool empty() const 
	{ 
		return storage_.empty(); 
	}

	void resize(size_t newsize)
	{
		storage_.resize(newsize);
		rpos_ = 0;
		wpos_ = size();
	}

	void reserve(size_t ressize)
	{
		if (ressize > size())
		{
			storage_.reserve(ressize);
		}
	}

	///
	/// append func
	///
	ByteBuffer& append(const std::string& str)
	{
		return append(static_cast<const char*>(str.c_str()), str.size());
	}

	ByteBuffer& append(const char* src, size_t cnt)
	{
		return append(reinterpret_cast<const uint8_t*>(src), cnt);
	}

	template <typename T>
	ByteBuffer& append(const T* src, size_t cnt)
	{
		return append(static_cast<const uint8_t*>(src), cnt * sizeof(T));
	}

	ByteBuffer& append(const uint8_t* src, size_t cnt)
	{
		if (!cnt || src == NULL) 
			return *this;
		assert(size() < 10000000);

		if (storage_.size() < wpos_ + cnt)
		{
			storage_.resize(wpos_ + cnt);
		}

		memcpy(&storage_[wpos_], src, cnt);
		wpos_ += cnt;

		return *this;
	}

	ByteBuffer& append(const ByteBuffer& buffer)
	{
		if (buffer.wpos())
		{
			return append(buffer.contents(), buffer.wpos());
		}

		return *this;
	}

	void put(size_t pos, const uint8_t* src, size_t cnt)
	{
		if (pos + cnt > size())
		{
			throw Exception(std::string("ByteBuffer - void put(size_t pos, const uint8_t* src, size_t cnt)"));
		}

		memcpy(&storage_[pos], src, cnt);
	}

	// TODO: 
	void printStorage() const 
	{ }

	void textlike() const
	{ }

	void hexlike() const
	{ }


private:
	///
	/// read func
	///
	/// don't use pointer type in this func
	template <typename T>
	T read()
	{
		resetBitReader();
		T r = read<T>(rpos_);
		rpos_ += sizeof(T);
		return r;
	}

	template <typename T>
	T read(size_t pos) const
	{
		if (pos + sizeof(T) > size())
		{
			throw Exception(std::string("ByteBuffer - T read(size_t pos) const"));
		}

#ifdef NONE_UNALIGNED_MEMORY_ACCESS
		T val;
		memcpy((void*)&val, (const void*)&storage_[pos], sizeof(T));
#else // !NONE_UNALIGNED_MEMORY_ACCESS
		T val = *(reinterpret_cast<const T*>(&storage_[pos])); // FIXME: const T* --> implicit_cast<const T*>
#endif // NONE_UNALIGNED_MEMORY_ACCESS

		EndianConvert(val);
		return val;
	}

	ByteBuffer& read(uint8_t* dest, size_t len)
	{
		resetBitReader();
		if (rpos_ + len > size())
		{
			throw Exception(std::string("ByteBuffer - ByteBuffer& read(uint8_t* dest, size_t len)"));
		}

		memcpy(dest, &storage_[rpos_], len);
		rpos_ += len;

		return *this;
	}

	///
	/// ---
	///
	template <typename T>
	ByteBuffer& readSkip()
	{
		readSkip(sizeof(T));
		return *this;
	}

	ByteBuffer& readSkip(size_t skip)
	{
		resetBitReader();
		if (rpos_ + skip > size())
		{
			throw Exception(std::string("ByteBuffer - ByteBuffer& readSkip(size_t skip)"));
		}

		rpos_ += skip;
		return *this;
	}

	/// 
	/// read string
	///
	std::string readString()
	{
		std::string s = 0;
		(*this) >> s;
		return s;
	}

	std::string readString(uint32_t count)
	{
		std::string out;
		uint32_t start = rpos();
		while (rpos() < size() && rpos() < start + count) // prevent crash at wrong string format in packet
		{
			out += read<char>();
		}

		return out;
	}

	///
	/// write string
	///
	ByteBuffer& writeStringData(const std::string& str)
	{
		flushBits();
		return append(static_cast<const char*>(str.c_str()), str.size());
	}

	template <typename T> ByteBuffer& append(T value)
	{
		flushBits();
		EndianConvert(value);
		return append(reinterpret_cast<uint8_t*>(&value), sizeof(value)); //FIXME: uint8_t* --> implicit_cast<uint8_t*>
	}

	void flushBits()
	{
		if (8 == bitpos_) return ;

		append(static_cast<uint8_t*>(&curbitval_), sizeof(curbitval_));
		curbitval_	= 0;
		bitpos_		= 8;
	}

	void resetBitReader()
	{
		bitpos_ = 8;
	}

	template <typename T> bool writeBit(T bit)
	{
		--bitpos_;
		if (bit)
		{
			curbitval_ |= (1 << (bitpos_));
		}

		if (bitpos_ == 0)
		{
			bitpos_ = 8;
			append(static_cast<uint8_t*>(&curbitval_), sizeof(curbitval_));
			curbitval_ = 0;
		}

		return bit != 0;
	}

	bool readBit()
	{
		++bitpos_;
		if (bitpos_ > 7)
		{
			curbitval_	= read<uint8_t>();
			bitpos_		= 0;
		}

		return ((curbitval_ >> (7-bitpos_)) & 1) != 0;
	}

	template <typename T> void writeBits(T value, size_t bits)
	{
		for (int32_t i = bits-1; i >= 0; --i)
		{
			writeBit((value >> i) & 1);
		}
	}

	uint32_t readBits(size_t bits)
	{
		uint32_t value = 0;
		for (int32_t i = bits-1; i >= 0; --i)
		{
			if (readBit())
				value |= (i << i);
		}

		return value;
	}
	
	template <typename T> void put(size_t pos, T value)
	{
		EndianConvert(value);
		put(pos, static_cast<uint8_t*>(&value), sizeof(value));
	}


private:
	size_t rpos_;
	size_t wpos_;
	size_t bitpos_;
	uint8_t curbitval_;
	std::vector<uint8_t> storage_;

}; // ByteBuffer


///
/// BoundArray : like std::vector, but with boundary
///
template <typename T, size_t M>
class BoundArray : public shared::copyable
{
	template <typename I, size_t J>
	friend inline ByteBuffer& operator<<(ByteBuffer& b, BoundArray<I, J>& a);
	template <typename K, size_t Y>
    friend inline ByteBuffer& operator>>(ByteBuffer& b, BoundArray<K, Y>& a);
public:
	BoundArray()
		: max_size_(M)
	{ }

	const T* begin() const
	{
		return vec_.size() == 0 ? NULL : &vec_.at(0);
	}

	void push_back(const T& elem)
	{
		if (vec_.size() >= max_size_) {
			assert(false);
			throw Exception(std::string("BoundArray -  void push_back(const T& elem)"));
		}
		vec_.push_back(elem);
	}
	
	T& operator[](size_t n)
	{
		if (n >= vec_.size() || n >= max_size_) {
			assert(false);
			throw Exception(std::string("BoundArray -  T& operator[](size_t n)"));
		}
		return vec_[n];
	}

	const T& operator[](size_t n) const
	{
		if (n >= vec_.size() || n >= max_size_) {
			assert(false);
			throw Exception(std::string("BoundArray - const T& operator[](size_t n)"));
		}
		return vec_[n];
	}

	T& at(size_t n)
	{
		if (n >= vec_.size() || n >= max_size_) {
			assert(false);
			throw Exception(std::string("BoundArray - T& at(size_t n)"));
		}
		return vec_[n];
	}

	const T& at(size_t n) const
	{
		if (n >= vec_.size() || n >= max_size_) {
			assert(false);
			throw Exception(std::string("BoundArray - const T& at(size_t n) const"));
		}
		return vec_[n];
	}

	bool copy_from_array(const T* begin, size_t size)
	{
		if ((size + vec_.size()) > max_size_) {
			//throw Exception(std::string("BoundArray - const T& operator[](size_t n)"));
			assert(false);
			return false;
		}

		for (size_t i = 0; i < size; ++i) {
			vec_.push_back(begin[i]);
		}
		return true;
	}

	bool write_to_array(T* begin, size_t array_size)
	{
		if (array_size < vec_.size()) {
			//throw Exception(std::string("BoundArray - void write_to_array(T* begin, size_t array_size)"));
			assert(false);
			return false;
		}

		for (size_t i = 0; i < vec_.size(); ++i) {
			begin[i] = vec_[i];
		}
		return true;
	}

	std::string to_str() const
	{
		std::string tempstr;
        if (vec_.size() > 0)
        {
            size_t len = vec_.size() * sizeof(T);
            tempstr.assign(reinterpret_cast<const char*>(begin()), len);
        }
		return tempstr;
	}

	inline size_t size() const { return vec_.size(); }
	inline size_t max_size() const { return max_size_; }
	inline bool empty() const { return vec_.empty(); }

	void clear() { vec_.clear(); }

protected:
	BoundArray(size_t maxsize)
		: max_size_(maxsize)
	{ }

private:
	std::vector<T> vec_;
	size_t   max_size_;
};


///
/// BoundString : like std::string, but with boundary
///
template <size_t M>
class BoundString : public shared::copyable
{
    template <size_t J>
	friend inline ByteBuffer& operator<<(ByteBuffer& b, BoundString<J>& a);
	template <size_t K>
    friend inline ByteBuffer& operator>>(ByteBuffer& b, BoundString<K>& a);
public:
    BoundString()
		: max_size_(M)
	{ }

    const std::string& to_str() const
    {
        return str_;
    }

    const char* c_str() const
    {
        return str_.c_str();
    }

    void assign(const std::string& str)
    {
        if (str.size() > max_size_) {
            assert(false);
        }

        str_.assign(str);
    }

    void assign(const char* s, size_t n)
    {
        if (n > max_size_) {
            assert(false);
        }

        str_.assign(s, n);
    }
    
    inline size_t size() const     { return str_.size(); }
	inline size_t max_size() const { return max_size_; }
	inline bool   empty() const    { return str_.empty(); }
	inline void   clear()          { str_.clear(); }


protected:
	BoundString(size_t maxsize)
		: max_size_(maxsize)
	{ }

    
private:
    std::string str_;
    size_t max_size_;
};


///
/// FixedArray : fixed array with boundary
///
template <typename T, size_t M>
class FixedArray : public shared::copyable
{
public:
	FixedArray() { }
	~FixedArray() { }

	T& operator[](size_t n)
	{
		assert(n >= 0 && n < M);
		return array_[n];
	}

	const T& operator[](size_t n) const
	{
		assert(n >= 0 && n < M);
		return array_[n];
	}

	inline size_t size() const { return M; }

private:
	T array_[M];
};


///
/// template Func
/// 
template <typename T>
inline ByteBuffer& operator<<(ByteBuffer& buf, T& t)
{
	t.Pack(buf);
	return buf;
}

template <typename T>
inline ByteBuffer& operator>>(ByteBuffer& buf, T& t)
{
	t.UnPack(buf);
	return buf;
}

template <typename T, size_t M>
inline ByteBuffer& operator<<(ByteBuffer&b, BoundArray<T, M>& a)
{
	if (a.vec_.size() > a.max_size())
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer&b, BoundArray& a)"));
	}

	b << static_cast<uint32_t>(a.max_size());
	return b << a.vec_;
}

template <typename T, size_t M>
inline ByteBuffer& operator>>(ByteBuffer& b, BoundArray<T, M>& a)
{
	uint32_t maxsize;
	b >> maxsize;
	if (a.max_size() != maxsize || static_cast<uint32_t>(b.PeekInt32()) > maxsize)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, BoundArray& a)"));
	}
	return b >> a.vec_;
}

template <size_t M>
inline ByteBuffer& operator<<(ByteBuffer& b, BoundString<M>& a)
{
    if (a.str_.size() > a.max_size())
    {
        throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, BoundString<M>& a)"));
    }

    b << static_cast<uint32_t>(a.max_size());
    return b << a.str_;
}

template <size_t M>
inline ByteBuffer& operator>>(ByteBuffer& b, BoundString<M>& a)
{
    uint32_t maxsize;
    b >> maxsize;
    if (a.max_size() != maxsize || static_cast<uint32_t>(b.PeekInt32()) > maxsize)
    {
        throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, BoundString<M>& a) "));
    }
    return b >> a.str_;
}

template <typename T, size_t M>
inline ByteBuffer& operator<<(ByteBuffer& b, FixedArray<T, M>& a)
{
	if (a.size() != M)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, FixedArray<T, M>& a)"));
	}

	b << static_cast<uint32_t>(a.size());
	for (size_t i = 0; i < a.size(); ++i)
	{
		b << a[i];
	}
	return b;
}

template <typename T, size_t M>
inline ByteBuffer& operator>>(ByteBuffer& b, FixedArray<T, M>& a)
{
	uint32_t maxsize;
	b >> maxsize;
	if (a.size() != maxsize || maxsize != M)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, FixedArray<T, M>& a)"));
	}

	for (size_t i = 0; i < a.size(); ++i)
	{
		b >> a[i];
	}
	return b;
}

template <typename T>
inline ByteBuffer& operator<<(ByteBuffer& b, std::vector<T>& v)
{
	if (v.size() > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, std::vector<T>& v)"));
	}

	b << static_cast<uint32_t>(v.size());
	for (typename std::vector<T>::iterator it = v.begin(); it != v.end(); ++it)
	{
		b << *it;
	}

	return b;
}

template <typename T>
inline ByteBuffer& operator<<(ByteBuffer& b, const std::vector<T>& v)
{
	if (v.size() > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, const std::vector<T>& v)"));
	}

	b << static_cast<uint32_t>(v.size());
	for (typename std::vector<T>::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		b << *it;
	}

	return b;
}

template <typename T>
inline ByteBuffer& operator>>(ByteBuffer& b, std::vector<T>& v)
{
	uint32_t vsize;
	b >> vsize;
	if (vsize > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, std::vector<T>& v)"));
	}

	v.clear();
	while (vsize)
	{
		T t;
		b >> t;
		v.push_back(t);
		--vsize;
	}

	return b;
}

template <typename T>
inline ByteBuffer& operator<<(ByteBuffer& b, std::list<T>& v)
{
	if (v.size() > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, std::list<T>& v)"));
	}

	b << static_cast<uint32_t>(v.size());
	for (typename std::list<T>::iterator it = v.begin(); it != v.end(); ++it)
	{
		b << *it;
	}

	return b;
}

template <typename T>
inline ByteBuffer& operator>>(ByteBuffer& b, std::list<T> &v)
{
	uint32_t vsize;
	b >> vsize;
	if (vsize > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, std::list<T> &v)"));
	}

	v.clear();
	while (vsize)
	{
		T t;
		b >> t;
		v.push_back(t);
		--vsize;
	}

	return b;
}

template <typename K, typename V>
inline ByteBuffer& operator<<(ByteBuffer& b, std::map<K, V> &m)
{
	if (m.size() > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, std::map<K, V> &m)"));
	}

	b << static_cast<uint32_t>(m.size());
	for (typename std::map<K, V>::iterator it = m.begin(); it != m.end(); ++it)
	{
		b << it->first << it->second;
	}

	return b;
}

template <typename K, typename V>
inline ByteBuffer& operator>>(ByteBuffer& b, std::map<K, V>& m)
{
	uint32_t msize;
	b >> msize;
	if (msize > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, std::map<K, V>& m)"));
	}

	m.clear();
	while (msize)
	{
		K k;
		V v;
		b >> k >> v;
		m.insert(std::make_pair(k, v));
		--msize;
	}

	return b;
}

template <typename V>
inline ByteBuffer& operator<<(ByteBuffer& b, std::set<V>& s)
{
	if (s.size() > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator<<(ByteBuffer& b, std::set<V> &s)"));
	}

	b << static_cast<uint32_t>(s.size());
	for (typename std::set<V>::iterator it = s.begin(); it != s.end(); ++it)
	{
		b << (*it);
	}

	return b;
}

template <typename V>
inline ByteBuffer& operator>>(ByteBuffer& b, std::set<V>& s)
{
	uint32_t ssize;
	b >> ssize;
	if (ssize > kMaxBBDynamicContainerSize)
	{
		throw Exception(std::string("inline ByteBuffer& operator>>(ByteBuffer& b, std::set<V>& s)"));
	}

	s.clear();
	while (ssize)
	{
		V v;
		b >> v;
		s.insert(v);
		--ssize;
	}

	return b;
}

template<> inline std::string ByteBuffer::read<std::string>()
{
	std::string tmp;
	*this >> tmp;
	return tmp;
}

template<> inline ByteBuffer& ByteBuffer::readSkip<char *>()
{
	std::string tmp;
	*this >> tmp;

	return *this;
}

template<> inline ByteBuffer& ByteBuffer::readSkip<const char*>()
{
	return readSkip<char*>();
}

template<> inline ByteBuffer& ByteBuffer::readSkip<std::string>()
{
	return readSkip<char*>();
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_BYTEBUFFER_H_

