#ifndef SHARED_NET_BUFFER_H_
#define SHARED_NET_BUFFER_H_

#include <string.h>

#include <algorithm>
#include <vector>

#include "shared/base/assertx.h"
#include "shared/base/types.h"
#include "shared/base/stringpiece.h"
#include "shared/base/copyable.h"
#include "shared/net/endian_ex.h"


namespace shared {
namespace net {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer : public shared::copyable
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize  = 1024;

	Buffer()
		: buffer_(kCheapPrepend + kInitialSize),
		  readerIndex_(kCheapPrepend),
		  writerIndex_(kCheapPrepend)
	{
		assert(ReadableBytes() == 0);
		assert(WritableBytes() == kInitialSize);
		assert(PrependableBytes() == kCheapPrepend);
	}

	// implicit copy-ctor, move-ctor, dtor and assignment are fine
	// NOTE: implicit move-ctor is added in g++ 4.6

	void Swap(Buffer& rhs)
	{
		buffer_.swap(rhs.buffer_);
		std::swap(readerIndex_, rhs.readerIndex_);
		std::swap(writerIndex_, rhs.writerIndex_);
	}

	size_t ReadableBytes() const
	{ return writerIndex_ - readerIndex_; }

	size_t WritableBytes() const
	{ return buffer_.size() - writerIndex_; }

	size_t PrependableBytes() const
	{ return readerIndex_; }

	const char* peek() const
	{ return begin() + readerIndex_; }

	const char* FindCRLF() const
	{
		const char* crlf = std::search(peek(), BeginWrite(), kCRLF, kCRLF+2);
		return crlf == BeginWrite() ? NULL : crlf;
	}

	const char* FindCRLF(const char* start) const
	{
		assert(peek() <= start);
		assert(start <= BeginWrite());
		const char* crlf = std::search(start, BeginWrite(), kCRLF, kCRLF+2);
		return crlf == BeginWrite() ? NULL : crlf;
	}

	// retrieve returns void, to prevent
	// string str(retrieve(readableBytes()), readableBytes());
	// the evaluation of two functions are unspecified
	void Retrieve(size_t len)
	{
		assert(len <= ReadableBytes());
		if (len < ReadableBytes())
		{
			readerIndex_ += len;
		}
		else
		{
			RetrieveAll();
		}
	}

	void RetrieveUntil(const char* end)
	{
		assert(peek() <= end);
		assert(end <= BeginWrite());
		Retrieve(end - peek());
	}

	void RetrieveInt32()
	{
		Retrieve(sizeof(int32_t));
	}

	void RetrieveInt16()
	{
		Retrieve(sizeof(int16_t));
	}

	void RetrieveInt8()
	{
		Retrieve(sizeof(int8_t));
	}

	void RetrieveAll()
	{
		readerIndex_ = kCheapPrepend;
		writerIndex_ = kCheapPrepend;
	}

    std::string RetrieveAllAsString()
	{
		return RetrieveAsString(ReadableBytes());;
	}

    std::string RetrieveAsString(size_t len)
	{
		assert(len <= ReadableBytes());
        std::string result(peek(), len);
		Retrieve(len);
		return result;
	}

	StringPiece ToStringPiece() const
	{
		return StringPiece(peek(), static_cast<int>(ReadableBytes()));
	}

	void Append(const StringPiece& str)
	{
		Append(str.data(), str.size());
	}

	void Append(const char* /*restrict*/ data, size_t len)
	{
		EnsureWritableBytes(len);
		std::copy(data, data+len, BeginWrite());
		HasWritten(len);
	}

	void Append(const void* /*restrict*/ data, size_t len)
	{
		Append(static_cast<const char*>(data), len);
	}

	void EnsureWritableBytes(size_t len)
	{
		if (WritableBytes() < len)
		{
			MakeSpace(len);
		}
		assert(WritableBytes() >= len);
	}

	char* BeginWrite()
	{ return begin() + writerIndex_; }

	const char* BeginWrite() const
	{ return begin() + writerIndex_; }

	void HasWritten(size_t len)
	{ writerIndex_ += len; }

	///
	/// Append int32_t using network endian
	///
	void AppendInt32(int32_t x)
	{
		int32_t be32 = sockets::HostToNetwork32(x);
		Append(&be32, sizeof be32);
	}

	void AppendInt16(int16_t x)
	{
		int16_t be16 = sockets::HostToNetwork16(x);
		Append(&be16, sizeof be16);
	}

	void AppendInt8(int8_t x)
	{
		Append(&x, sizeof x);
	}

	///
	/// Read int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	int32_t ReadInt32()
	{
		int32_t result = PeekInt32();
		RetrieveInt32();
		return result;
	}

	int16_t ReadInt16()
	{
		int16_t result = PeekInt16();
		RetrieveInt16();
		return result;
	}

	int8_t ReadInt8()
	{
		int8_t result = PeekInt8();
		RetrieveInt8();
		return result;
	}

	///
	/// Peek int32_t from network endian
	///
	/// Require: buf->readableBytes() >= sizeof(int32_t)
	int32_t PeekInt32() const
	{
		assert(ReadableBytes() >= sizeof(int32_t));
		int32_t be32 = 0;
		::memcpy(&be32, peek(), sizeof be32);
		return sockets::NetworkToHost32(be32);
	}

	int16_t PeekInt16() const
	{
		assert(ReadableBytes() >= sizeof(int16_t));
		int16_t be16 = 0;
		::memcpy(&be16, peek(), sizeof be16);
		return sockets::NetworkToHost16(be16);
	}

	int8_t PeekInt8() const
	{
		assert(ReadableBytes() >= sizeof(int8_t));
		int8_t x = *peek();
		return x;
	}

	///
	/// Prepend int32_t using network endian
	///
	void PrependInt32(int32_t x)
	{
		int32_t be32 = sockets::HostToNetwork32(x);
		Prepend(&be32, sizeof be32);
	}

	void PrependInt16(int16_t x)
	{
		int16_t be16 = sockets::HostToNetwork16(x);
		Prepend(&be16, sizeof be16);
	}

	void PrependInt8(int8_t x)
	{
		Prepend(&x, sizeof x);
	}

	void Prepend(const void* /*restrict*/ data, size_t len)
	{
		assert(len <= PrependableBytes());
		readerIndex_ -= len;
		const char* d = static_cast<const char*>(data);
		std::copy(d, d+len, begin()+readerIndex_);
	}

	void Shrink(size_t reserve)
	{
		// FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
		Buffer other;
		other.EnsureWritableBytes(ReadableBytes()+reserve);
		other.Append(ToStringPiece());
		Swap(other);
	}

	size_t InternalCapacity() const
	{
		return buffer_.capacity();
	}

	/// Read data directly into buffer.
	///
	/// It may implement with readv(2)
	/// @return result of read(2), @c errno is saved
	ssize_t ReadFd(int fd, int* savedErrno);

private:

	char* begin()
	{ return &*buffer_.begin(); }

	const char* begin() const
	{ return &*buffer_.begin(); }

	void MakeSpace(size_t len)
	{
		if (WritableBytes() + PrependableBytes() < len + kCheapPrepend)
		{
			// FIXME: move readable data
			buffer_.resize(writerIndex_+len);
		}
		else
		{
			// move readable data to the front, make space inside buffer
			assert(kCheapPrepend < readerIndex_);
			size_t readable = ReadableBytes();
			std::copy(begin()+readerIndex_,
					begin()+writerIndex_,
					begin()+kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == ReadableBytes());
		}
	}

private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;

	static const char kCRLF[];
};

} // namespace net
} // namespace shared

#endif  // SHARED_NET_BUFFER_H_
