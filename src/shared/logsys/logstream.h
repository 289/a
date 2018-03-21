#ifndef SHARED_LOGSYS_LOGSTREAM_H_
#define SHARED_LOGSYS_LOGSTREAM_H_

#include <assert.h>
#include <string.h> // memcpy
#include <string>

#include "shared/base/types.h"
#include "shared/base/noncopyable.h"
#include "shared/base/stringpiece.h"

namespace shared {

namespace detail 
{

	const int kSmallBuffer = 4000;
	const int kLargeBuffer = 4000*1000;

	template<int SIZE>
	class FixedBuffer : noncopyable
	{
		public:
			FixedBuffer()
				: cur_(data_)
			{
				SetCookie(CookieStart);
			}

			~FixedBuffer()
			{
				SetCookie(CookieEnd);
			}

			void append(const char* /*restrict*/ buf, size_t len)
			{
				// FIXME: append partially
				if (implicit_cast<size_t>(avail()) > len)
				{
					memcpy(cur_, buf, len);
					cur_ += len;
				}
			}

			const char* data() const { return data_; }
			int		length() const { return static_cast<int>(cur_ - data_); }

			// write to data_ directly
			char*	current() { return cur_; }
			int		avail() const { return static_cast<int>(end() - cur_); }
			void	add(size_t len) { cur_ += len; }

			void	reset() { cur_ = data_; }
			void	bzero() { ::bzero(data_, sizeof data_); }

			// for used by GDB
			const char* DebugString();
			void	SetCookie(void (*cookie)()) { cookie_ = cookie; }
			// for used by unit test
            std::string	AsString() const { return std::string(data_, length()); }

		private:
			const char* end() const { return data_ + sizeof data_; }
			// Must be outline function for cookies.
			static void CookieStart();
			static void CookieEnd();

			void (*cookie_)();
			char	data_[SIZE];
			char*	cur_;
	};

}

class LogStream : noncopyable
{
	typedef LogStream self;
public:
	typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

	self& operator<<(bool v)
	{
		buffer_.append(v ? "1" : "0", 1);
		return *this;
	}

	self& operator<<(short);
	self& operator<<(unsigned short);
	self& operator<<(int);
	self& operator<<(unsigned int);
	self& operator<<(long);
	self& operator<<(unsigned long);
	self& operator<<(long long);
	self& operator<<(unsigned long long);

	self& operator<<(const void*);

	self& operator<<(float v)
	{
		*this << static_cast<double>(v);
		return *this;
	}
	self& operator<<(double);
	// self& operator<<(long double);

	self& operator<<(char v)
	{
		buffer_.append(&v, 1);
		return *this;
	}

	// self& operator<<(signed char);
	// self& operator<<(unsigned char);

	self& operator<<(const char* v)
	{
		buffer_.append(v, strlen(v));
		return *this;
	}

	/*self& operator<<(const string& v)
	{
		buffer_.append(v.c_str(), v.size());
		return *this;
	}*/

	self& operator<<(const std::string& v)
	{
		buffer_.append(v.c_str(), v.size());
		return *this;
	}

	self& operator<<(const StringPiece& v)
	{
		buffer_.append(v.data(), v.size());
		return *this;
	}

	void Append(const char* data, int len) { buffer_.append(data, len); }
	const Buffer& buffer() const { return buffer_; }
	void ResetBuffer() { buffer_.reset(); }

private:
	void StaticCheck();

	template<typename T>
	void FormatInteger(T);

	Buffer buffer_;

	static const int kMaxNumericSize = 32;
};

class Fmt
{
public:
	template<typename T>
	Fmt(const char* fmt, T val);

	const char* data() const { return buf_; }
	int			length() const { return length_; }

private:
	char	buf_[32];
	int		length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
	s.Append(fmt.data(), fmt.length());
	return s;
}

} // namespace shared

#endif  // SHARED_LOGSYS_LOGSTREAM_H_

