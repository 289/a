#ifndef SHARED_NET_PACKET_PLATFORM_DEFINE_H_
#define SHARED_NET_PACKET_PLATFORM_DEFINE_H_


namespace shared {
namespace net {

	const static size_t kMaxBBDynamicContainerSize = 4096;

} // namespace net
} // shared


///
/// endian
///
#define SHARED_LITTLEENDIAN 0
#define SHARED_BIGENDIAN    1

#if !defined(SHARED_ENDIAN)
#  if defined (USE_BIG_ENDIAN)
#    define SHARED_ENDIAN SHARED_BIGENDIAN
#  else 
#    define SHARED_ENDIAN SHARED_LITTLEENDIAN
#  endif 
#endif // SHARED_ENDIAN


///
/// define run in client/server side
///
#define RUN_IN_SERVER_SIDE

#if defined(RUN_IN_SERVER_SIDE)
  #ifndef SERVER_SIDE
    #define SERVER_SIDE 
  #endif // SERVER_SIDE
#else
  #ifndef CLIENT_SIDE
    #define CLIENT_SIDE 
  #endif // CLIENT_SIDE
#endif


#endif // SHARED_NET_PACKET_PLATFORM_DEFINE_H_
