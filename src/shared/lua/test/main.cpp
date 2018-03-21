#include <stdio.h>
#include <lua.hpp>
#include "shared/lua/lua_engine.h"

#include <list>
#include <stdlib.h>


using namespace luabind;


#define RETURN_1_VALUE(engine, value) \
	engine.ClearStack(); \
	engine.PushValue(luabind::LuaValue(value)); \
	return 1;

int PlayerMoveTask(lua_State* state)
{
	luabind::LuaEngine engine;
	engine.Init(state);

	
	RETURN_1_VALUE(engine, 1);
}



struct TimerHandler
{
	int _start;
	int _delay;
	typedef void (*handler_fn)( void *arg );
	handler_fn _fn;
};

typedef std::list<TimerHandler*> TimerHandlerList;
TimerHandlerList gTimerHandlers;

void onTimerOut( void *arg )
{
	lua_State *L = (lua_State*) arg;
	lua_resume( L, NULL, 0 );
}

extern "C"
{
	int Sleep( lua_State *L )
	{
		int sec = (int)lua_tonumber( L, -1 );

		TimerHandler *h = (TimerHandler*) malloc( sizeof( TimerHandler ) );
		h->_start = time(NULL);
		h->_delay = sec;
		h->_fn = onTimerOut;

		gTimerHandlers.push_back( h );

		return lua_yield( L, 0 );
	}

}

luabind::CFunction func_table[] =
{

	/// Condition Function
	///
	{"PlayerMoveTask", PlayerMoveTask},
	{"sleep", Sleep},
};

int func_table_size = sizeof(func_table)/sizeof(luabind::CFunction);

int do_script( lua_State *L, const char *script )
{
	lua_pushcfunction( L, Sleep );
	lua_setglobal( L, "sleep" );

	int error = luaL_loadfile( L, script );
	if( error != 0 )
	{
		fprintf( stderr, "load script [%s] failed\n", script );
		return -1;
	}

	lua_resume( L, NULL, 0 );
	return 0;
}


int main(int argc, char** argv)
{
	lua_State* L = luaL_newstate();
	luaopen_base(L);
	do_script(L, "test_coro.lua");

	/*
	LuaEngine lua_engine;
	lua_engine.Init(L);
	lua_engine.RegisterCFunc(func_table, func_table_size);
	//lua_engine.Call("1011.lua", "fooXX");
	lua_engine.Call("test_coro.lua", "fooXX");
	lua_resume(L, NULL, 0);
	//
	*/

	/* for testing purpose only */
	while( gTimerHandlers.size() != 0 )
	{
		time_t now = time(NULL);
		for( TimerHandlerList::iterator it = gTimerHandlers.begin(); it != gTimerHandlers.end(); )
		{
			TimerHandler *h = *it;
			if( h->_start + h->_delay <= now )
			{
				h->_fn( L );
				free( h );
				it = gTimerHandlers.erase( it );
				continue;
			}

			++ it;
		}

		sleep( 1 );
	}


	/*
	   int index = 0;
	   while(true)
	   {
	   if (index++ < 10)
	   {
			printf("C++ PlayerMoveTask %d \n", index);
			sleep(1);
		}
		else
		{
			break;
		}
	}

	lua_engine.Call("1011.lua", "resumeCurCoroutine");
	*/

	//lua_engine.Call("102.lua", "fooXX");
	//lua_engine.Call("101.lua", "fooXX");

	//int points[6] = {11, 12, 21, 22, 31, 32};
	//lua_engine.Call("test.lua", "TableTest1", points, sizeof(points)/ sizeof(int));
	//lua_engine.Destroy();

	return 0;
}
