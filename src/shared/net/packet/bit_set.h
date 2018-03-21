#ifndef SHARED_NET_PACKET_LONG_MASK_H_
#define SHARED_NET_PACKET_LONG_MASK_H_

#include <bitset>

#include "shared/base/copyable.h"
#include "bytebuffer.h"


struct BSBoolValue
{
	BSBoolValue(bool v) : val(v) { }
	operator bool() const { return val; }
	bool val;
};
typedef BSBoolValue BSBOOL;


namespace shared {
namespace net {

/**
 * @brief 目前BitSet作为std::bitset的一个wrapper，使其能pack、unpack
 *    （1）可以使用if(!bs)来判断BitSet，但无法用if(bs)做判断，因为不能重载到bool型的
 *         类型转换。可以使用if(bs.any())，或者if(static_cast<BSBOOL>(bs))。
 *         以此类推if(bs1 & bs2)是无法使用的，只能写成if (static_cast<BSBOOL>(bs1 & bs2))
 *    （2）shared/net/tests/packet_test.cpp里有使用范例
 */
template <size_t M>
class BitSet : public copyable
{
	template <size_t K>
	friend BitSet<K> operator&(const BitSet<K>& lhs, const BitSet<K>& rhs);
	template <size_t K>
	friend BitSet<K> operator|(const BitSet<K>& lhs, const BitSet<K>& rhs);
	
public:
	BitSet()  { }
	~BitSet() { }
	BitSet(const std::bitset<M>& rhsbits);
	BitSet(const std::string& str);

	operator BSBOOL() const;
	BitSet& operator= (const std::bitset<M>& rhsbits);
	bool    operator!() const;
	
	// contrast with std::bitset
	BitSet& operator&= (const BitSet<M>& rhs);
	BitSet& operator|= (const BitSet<M>& rhs);
	BitSet& operator<<= (size_t pos);
	BitSet& operator>>= (size_t pos);
	BitSet  operator~ () const;
	BitSet  operator<< (size_t pos) const;
	BitSet  operator>> (size_t pos) const;
	bool    operator== (const BitSet<M>& rhs) const;
	bool    operator!= (const BitSet<M>& rhs) const;

	std::string to_string() const;
	bool    test(size_t pos) const;
	size_t  size() const;
	bool    any() const;  // Test if any bit is set 
	bool    none() const; // Test if no bit is set 
	bool    all() const;  // Returns whether all of the bits in the bitset are set (to one).

	BitSet& set();
	BitSet& set(size_t pos, bool val = true);
	BitSet& reset(); // Resets bits to zero: all bits
	BitSet& reset(size_t pos); // Resets bits to zero: single bit
	BitSet& flip();
	BitSet& flip(size_t pos);

	void Pack(shared::net::ByteBuffer& buf);
	void UnPack(shared::net::ByteBuffer& buf);


protected:
	inline void    SetBit(uint8_t* a, uint8_t b);
	inline void    ResetBit(uint8_t* a, uint8_t b);
	inline void    XOrBit(uint8_t* a, uint8_t b);
	inline uint8_t GetBit(uint8_t* a, uint8_t b);


private:
	std::bitset<M> bit_set_;
};

///
/// member functions
///
template <size_t M>
BitSet<M>::BitSet(const std::bitset<M>& rhsbits)
	: bit_set_(rhsbits)
{
}

template <size_t M>
BitSet<M>::BitSet(const std::string& str)
	: bit_set_(str)
{
}

template <size_t M>
BitSet<M>::operator BSBOOL() const
{
	return BSBOOL(bit_set_.any());
}

template <size_t M>
BitSet<M>& BitSet<M>::operator=(const std::bitset<M>& rhsbits)
{
	bit_set_ = rhsbits;
	return *this;
}

template <size_t M>
bool BitSet<M>::operator!() const
{
	return bit_set_.none();
}

template <size_t M>
BitSet<M>& BitSet<M>::operator&=(const BitSet<M>& rhs)
{
	bit_set_ &= rhs.bit_set_;
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::operator|=(const BitSet<M>& rhs)
{
	bit_set_ |= rhs.bit_set_;
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::operator<<=(size_t pos)
{
	bit_set_ <<= pos;
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::operator>>=(size_t pos)
{
	bit_set_ >>= pos;
	return *this;
}

template <size_t M>
BitSet<M> BitSet<M>::operator~() const
{
	BitSet<M> tmp;
	tmp = ~bit_set_;
	return tmp;
}

template <size_t M>
BitSet<M> BitSet<M>::operator<<(size_t pos) const
{
	BitSet<M> tmp;
	tmp = bit_set_ << pos;
	return tmp;
}

template <size_t M>
BitSet<M> BitSet<M>::operator>>(size_t pos) const
{
	BitSet<M> tmp;
	tmp = bit_set_ >> pos;
	return tmp;
}

template <size_t M>
bool BitSet<M>::operator==(const BitSet<M>& rhs) const
{
	return bit_set_ == rhs.bit_set_;
}

template <size_t M>
bool BitSet<M>::operator!=(const BitSet<M>& rhs) const
{
	return bit_set_ != rhs.bit_set_;
}

template <size_t M>
std::string BitSet<M>::to_string() const
{
	return bit_set_.to_string();
}

template <size_t M>
bool BitSet<M>::test(size_t pos) const
{
	return bit_set_.test(pos);
}

template <size_t M>
bool BitSet<M>::any() const
{
	return bit_set_.any();
}
	
template <size_t M>
bool BitSet<M>::none() const
{
	return bit_set_.none();
}

template <size_t M>
size_t BitSet<M>::size() const
{
	return bit_set_.size();
}

template <size_t M>
bool BitSet<M>::all() const
{
	return bit_set_.all();
}

template <size_t M>
BitSet<M>& BitSet<M>::set()
{
	bit_set_.set();
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::set(size_t pos, bool val)
{
	bit_set_.set(pos, val);
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::reset()
{
	bit_set_.reset();
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::reset(size_t pos)
{
	bit_set_.reset(pos);
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::flip()
{
	bit_set_.flip();
	return *this;
}

template <size_t M>
BitSet<M>& BitSet<M>::flip(size_t pos)
{
	bit_set_.flip(pos);
	return *this;
}

///
/// pack and unpack
///
#define BITS_PER_BYTE 8

template <size_t M>
void BitSet<M>::Pack(shared::net::ByteBuffer& buf)
{
	if (bit_set_.size() > ByteBuffer::kMaxBitsetSize || bit_set_.size() != M)
	{
		throw Exception(std::string("void BitSet<M>::Pack(shared::net::ByteBuffer& buf)"));
	}

	// align to 8-bit
	uint32_t count = ((bit_set_.size() + 7) & (~7)) >> 3;
	buf << count;

	int tail_alignment = ((M % BITS_PER_BYTE) == 0) ? BITS_PER_BYTE : (M % BITS_PER_BYTE);
	for (size_t i = 0; i < M;)
	{
		uint8_t tmpbyte = 0;
		size_t index_count = (1 == count) ? tail_alignment : BITS_PER_BYTE;
		for (size_t j = 0; j < index_count; ++j)
		{
			if (bit_set_[i++])
			{
				SetBit(&tmpbyte, j);
			}
		}
		buf << tmpbyte;
		--count;
	}
	assert(count == 0);
}
    
template <size_t M>
void BitSet<M>::UnPack(shared::net::ByteBuffer& buf)
{
	uint32_t count = 0;
	buf >> count;
	if (count > ByteBuffer::kMaxBitsetSize)
	{
		throw Exception(std::string("void BitSet<M>::UnPack(shared::net::ByteBuffer& buf) count > Max"));
	}

	assert(bit_set_.none());

	int bit_index = 0;
	int tail_alignment = ((M % BITS_PER_BYTE) == 0) ? BITS_PER_BYTE : (M % BITS_PER_BYTE);
	if (M != ((count-1)*8 + tail_alignment))
	{
		throw Exception(std::string("void BitSet<M>::UnPack(shared::net::ByteBuffer& buf) bits != M"));
	}

	while (count)
	{
		uint8_t tmpbyte = 0;
		buf >> tmpbyte;
		size_t index_count = (1 == count) ? tail_alignment : BITS_PER_BYTE;
		for (size_t i = 0; i < index_count; ++i)
		{
			if (GetBit(&tmpbyte, i))
			{
				bit_set_.set(bit_index, 1);
			}
			++bit_index;
		}
		--count;
	}

	assert(bit_index == M && bit_set_.size() == M);
}

#undef BITS_PER_BYTE

///
/// inline function
///
template <size_t M>
inline void BitSet<M>::SetBit(uint8_t* a, uint8_t b)
{
	(((uint8_t*)a)[(b)>>3] |= (1 << ((b)&7)));
}

template <size_t M>
inline uint8_t BitSet<M>::GetBit(uint8_t* a, uint8_t b)
{
	return ((((uint8_t*)a)[(b)>>3] >> ((b)&7)) & 1);
}

template <size_t M>
inline void BitSet<M>::ResetBit(uint8_t* a, uint8_t b)
{
	(((uint8_t*)a)[(b)>>3] &= ~(1 << ((b)&7)));
}

template <size_t M>
inline void BitSet<M>::XOrBit(uint8_t* a, uint8_t b)
{
	(((uint8_t*)a)[(b)>>3] ^= (1 << ((b)&7)));
}

///
/// non-member functions
///
template <size_t M>
BitSet<M> operator&(const BitSet<M>& lhs, const BitSet<M>& rhs)
{
	BitSet<M> tmp;
	tmp = lhs.bit_set_ & rhs.bit_set_;
	return tmp;
}

template <size_t M>
BitSet<M> operator|(const BitSet<M>& lhs, const BitSet<M>& rhs)
{
	BitSet<M> tmp;
	tmp = lhs.bit_set_ | rhs.bit_set_;
	return tmp;
}

} // namespace net
} // namespace shared

#endif // SHARED_NET_PACKET_LONG_MASK_H_
