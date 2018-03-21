#pragma once

// 这个文件里统一了多平台下STL的名字

#include <vector>
#include <set>
#include <map>

#define stl_vector    std::vector
#define stl_set       std::set
#define stl_map       std::map
#define stl_make_pair std::make_pair
#define stl_pair      std::pair

#ifdef PLATFORM_WINDOWS
# include <hash_map>
# define stl_hash_map stdext::hash_map 
#elif PLATFORM_LINUX
# include <backward/hash_map>
# define stl_hash_map __gnu_cxx::hash_map
# if __WORDSIZE == 64
# else
namespace __gnu_cxx
{
  template<>
  struct hash<int64_t>
  {
    size_t operator()(int64_t __x) const
    { return __x; }
  };
};
# endif
#elif PLATFORM_ANDROID
# include <backward/hash_map>
# define stl_hash_map __gnu_cxx::hash_map
namespace __gnu_cxx
{
  template<>
  struct hash<int64_t>
  {
    size_t operator()(int64_t __x) const
    { return __x; }
  };
};
#else
#endif
