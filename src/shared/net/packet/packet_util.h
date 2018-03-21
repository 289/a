#ifndef SHARED_NET_PACKET_PACKET_UTIL_H_
#define SHARED_NET_PACKET_PACKET_UTIL_H_

#include "shared/base/assertx.h"
#include "shared/net/packet/proto_packet.h"

///
/// init a proto_packet: must in cpp file
///
#define INIT_STATIC_PROTOPACKET(class_name, type) \
	const class_name class_name::init_packet_##class_name((uint16_t)type); 


///
/// declare a proto_packet
///
#define DECLARE_PROTOPACKET(class_name, type) \
	DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, shared::net::ProtoPacket, shared::net::PacketManager)

// prototype model needed clone
#define DECLARE_PACKET_CLONE_TEMPLATE(class_name, type, packet_name, manager_name) \
private: \
    static const class_name init_packet_##class_name; \
    class_name(packet_name::Type id) \
	    : packet_name(id) \
	{ \
		assert((uint16_t)id == class_name::TypeNumber()); \
		Release(); \
		manager_name::InsertPacket(id, this);\
	} \
    DECLARE_PACKET_COMMON_TEMPLATE(class_name, type, packet_name)

//
#define DECLARE_PACKET_COMMON_TEMPLATE(class_name, type, packet_name) \
public: \
	class_name(const class_name& rhs) \
	    : packet_name(rhs.typeid_) \
	{ \
		Release(); \
	} \
    class_name() \
        : packet_name(type) \
    { \
		Release(); \
	} \
	virtual packet_name* Clone()\
	{ \
		return static_cast<packet_name*>(new class_name(*this)); \
	} \
    inline static uint16_t TypeNumber() { return (uint16_t)type; } \
    inline static std::string TypeName() { return std::string(#class_name); }



///
/// implement marshal/unmarshal code
/// version 2se
///
#define PACKET_DEFINE(...) \
	virtual void Marshal() \
    { \
		shared::detail::make_define(__VA_ARGS__).marshal(buf_); \
	} \
    virtual void Unmarshal()\
    { \
		shared::detail::make_define(__VA_ARGS__).unmarshal(buf_); \
	}

// nested class of struct
#define NESTED_DEFINE(...) \
	void Pack(shared::net::ByteBuffer& buf) \
    { \
		shared::detail::make_define(__VA_ARGS__).marshal(buf); \
	} \
    void UnPack(shared::net::ByteBuffer& buf) \
    { \
		shared::detail::make_define(__VA_ARGS__).unmarshal(buf); \
	}



namespace shared {
namespace detail {
	

template <typename A0 = void, typename A1 = void, typename A2 = void, typename A3 = void, typename A4 = void, typename A5 = void, typename A6 = void, typename A7 = void, typename A8 = void, typename A9 = void, typename A10 = void, typename A11 = void, typename A12 = void, typename A13 = void, typename A14 = void, typename A15 = void, typename A16 = void, typename A17 = void, typename A18 = void, typename A19 = void, typename A20 = void, typename A21 = void, typename A22 = void, typename A23 = void, typename A24 = void, typename A25 = void, typename A26 = void, typename A27 = void, typename A28 = void, typename A29 = void, typename A30 = void, typename A31 = void, typename A32 = void>
struct define;

template <>
struct define<> {
	void marshal(shared::net::ByteBuffer& buf) {}
	void unmarshal(shared::net::ByteBuffer& buf) {}
};

template <typename A0>
struct define<A0> {
	define(A0& _a0)
		: a0(_a0) {}
	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0;
	}

	A0& a0;
};

template <typename A0, typename A1>
struct define<A0, A1> {
	define(A0& _a0, A1& _a1) :
		a0(_a0), a1(_a1) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1;
	}

	A0& a0;
	A1& a1;
};

template <typename A0, typename A1, typename A2>
struct define<A0, A1, A2> {
	define(A0& _a0, A1& _a1, A2& _a2) :
		a0(_a0), a1(_a1), a2(_a2) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2;
	}

	A0& a0;
	A1& a1;
	A2& a2;
};

template <typename A0, typename A1, typename A2, typename A3>
struct define<A0, A1, A2, A3> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3;
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4>
struct define<A0, A1, A2, A3, A4> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4;
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct define<A0, A1, A2, A3, A4, A5> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5;
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct define<A0, A1, A2, A3, A4, A5, A6> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6;
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct define<A0, A1, A2, A3, A4, A5, A6, A7> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7;
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8;
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10){}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10;  
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10, A11& _a11) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10), a11(_a11) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10 >> a11; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
	A11& a11;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10, A11& _a11, A12& _a12) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10), a11(_a11), a12(_a12) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10 >> a11 >> a12; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
	A11& a11;
	A12& a12;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10, A11& _a11, A12& _a12, A13& _a13) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10), a11(_a11), a12(_a12), a13(_a13) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10 >> a11 >> a12 >> a13; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
	A11& a11;
	A12& a12;
	A13& a13;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10, A11& _a11, A12& _a12, A13& _a13, A14& _a14) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10), a11(_a11), a12(_a12), a13(_a13), a14(_a14) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13 << a14; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10 >> a11 >> a12 >> a13 >> a14; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
	A11& a11;
	A12& a12;
	A13& a13;
	A14& a14;
};

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
struct define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> {
	define(A0& _a0, A1& _a1, A2& _a2, A3& _a3, A4& _a4, A5& _a5, A6& _a6, A7& _a7, A8& _a8, A9& _a9, A10& _a10, A11& _a11, A12& _a12, A13& _a13, A14& _a14, A15& _a15) :
		a0(_a0), a1(_a1), a2(_a2), a3(_a3), a4(_a4), a5(_a5), a6(_a6), a7(_a7), a8(_a8), a9(_a9), a10(_a10), a11(_a11), a12(_a12), a13(_a13), a14(_a14), a15(_a15) {}

	void marshal(shared::net::ByteBuffer& buf) 
	{
		buf << a0 << a1 << a2 << a3 << a4 << a5 << a6 << a7 << a8 << a9 << a10 << a11 << a12 << a13 << a14 << a15; 
	}
	void unmarshal(shared::net::ByteBuffer& buf) 
	{
		buf >> a0 >> a1 >> a2 >> a3 >> a4 >> a5 >> a6 >> a7 >> a8 >> a9 >> a10 >> a11 >> a12 >> a13 >> a14 >> a15; 
	}

	A0& a0;
	A1& a1;
	A2& a2;
	A3& a3;
	A4& a4;
	A5& a5;
	A6& a6;
	A7& a7;
	A8& a8;
	A9& a9;
	A10& a10;
	A11& a11;
	A12& a12;
	A13& a13;
	A14& a14;
	A15& a15;
};



/// 
/// define<> 
///
inline define<> make_define()
{
	return define<>();
}

template <typename A0>
define<A0> make_define(A0& a0)
{
	return define<A0>(a0);
}

template <typename A0, typename A1>
define<A0, A1> make_define(A0& a0, A1& a1)
{
	return define<A0, A1>(a0, a1);
}

template <typename A0, typename A1, typename A2>
define<A0, A1, A2> make_define(A0& a0, A1& a1, A2& a2)
{
	return define<A0, A1, A2>(a0, a1, a2);
}

template <typename A0, typename A1, typename A2, typename A3>
define<A0, A1, A2, A3> make_define(A0& a0, A1& a1, A2& a2, A3& a3)
{
	return define<A0, A1, A2, A3>(a0, a1, a2, a3);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
define<A0, A1, A2, A3, A4> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4)
{
	return define<A0, A1, A2, A3, A4>(a0, a1, a2, a3, a4);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
define<A0, A1, A2, A3, A4, A5> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
{
	return define<A0, A1, A2, A3, A4, A5>(a0, a1, a2, a3, a4, a5);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
define<A0, A1, A2, A3, A4, A5, A6> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6)
{
	return define<A0, A1, A2, A3, A4, A5, A6>(a0, a1, a2, a3, a4, a5, a6);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
define<A0, A1, A2, A3, A4, A5, A6, A7> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7>(a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8>(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10, A11& a11)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10, A11& a11, A12& a12)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10, A11& a11, A12& a12, A13& a13)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10, A11& a11, A12& a12, A13& a13, A14& a14)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15> make_define(A0& a0, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, A6& a6, A7& a7, A8& a8, A9& a9, A10& a10, A11& a11, A12& a12, A13& a13, A14& a14, A15& a15)
{
	return define<A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15>(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

} // namespace detail
} // namespace shared

#endif // SHARED_NET_PACKET_PACKET_UTIL_H_
