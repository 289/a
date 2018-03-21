#ifndef SHARED_BASE_SINGLETON_H_
#define SHARED_BASE_SINGLETON_H_

#include "shared/base/noncopyable.h"
#include "shared/base/assertx.h"

namespace shared 
{

class SingletonModule : public noncopyable
{
private:
	static bool & get_lock(){
		static bool lock = false;
		return lock;
	}
public:
	//    static const void * get_module_handle(){
	//        return static_cast<const void *>(get_module_handle);
	//    }
	static void lock(){
		get_lock() = true;
	}
	static void unlock(){
		get_lock() = false;
	}
	static bool is_locked() {
		return get_lock();
	}
};

namespace detail {

	template<class T>
	class SingletonWrapper : public T
	{
		public:
			static bool m_is_destroyed;
			~SingletonWrapper(){
				m_is_destroyed = true;
			}
	};

	template<class T>
		bool detail::SingletonWrapper< T >::m_is_destroyed = false;

} // detail

template <class T>
class Singleton : public SingletonModule
{
private:
	static T & instance;
	// include this to provoke instantiation at pre-execution time
	static void use(T const &) {}
	static T & get_instance() {
		static detail::SingletonWrapper< T > t;
		// refer to instance, causing it to be instantiated (and
		// initialized at startup on working compilers)
		assert(! detail::SingletonWrapper< T >::m_is_destroyed);
		use(instance);
		return static_cast<T &>(t);
	}
public:
	static T & get_mutable_instance(){
		assert(! is_locked());
		return get_instance();
	}
	static const T & get_const_instance(){
		return get_instance();
	}
	static bool is_destroyed(){
		return detail::SingletonWrapper< T >::m_is_destroyed;
	}
};

template<class T>
	T & Singleton< T >::instance = Singleton< T >::get_instance();

} // namespace shared

#endif //SHARED_BASE_SINGLETON_H_
