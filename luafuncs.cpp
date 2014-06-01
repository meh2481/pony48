#include "luafuncs.h"
#include "luainterface.h"
#include "Pony48.h"

luaFunc(sleep)	//seconds
{
	//sleep(lua_tointeger(L, 1));
	luaReturnNil();
}

static LuaFunctions s_functab[] =
{
	luaRegister(sleep),
	{NULL, NULL}
};

void lua_register_enginefuncs(lua_State *L)
{	
	for(unsigned int i = 0; s_functab[i].name; ++i)
		lua_register(L, s_functab[i].name, s_functab[i].func);
}
