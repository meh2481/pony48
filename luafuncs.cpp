#include "luafuncs.h"
#include "luainterface.h"
#include "Pony48.h"

extern Pony48Engine* g_pGlobalEngine;

//Class for interfacing between Pony48Engine and Lua
//(defined here because of weird cross-inclusion stuff)
class PonyLua
{
public:
	static ParticleSystem* getParticleSys(string sName)
	{
		if(g_pGlobalEngine->songParticles.count(sName))
			return g_pGlobalEngine->songParticles[sName];
		return NULL;
	}
	
	static Background* getBg()
	{
		return g_pGlobalEngine->m_bg;
	}
};

luaFunc(fireparticles)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->firing = b;
	luaReturnNil();
}

luaFunc(showparticles)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->show = b;
	luaReturnNil();
}

luaFunc(resetparticles)
{
	string s = getStr(L, 1);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->killParticles();
	luaReturnNil();
}

luaFunc(pinwheelspeed)
{
	float32 f = lua_tonumber(L, 1);
	pinwheelBg* bg = (pinwheelBg*)PonyLua::getBg();
	if(bg)
		bg->speed = f;
	luaReturnNil();
}

static LuaFunctions s_functab[] =
{
	luaRegister(fireparticles),
	luaRegister(showparticles),
	luaRegister(resetparticles),
	luaRegister(pinwheelspeed),
	{NULL, NULL}
};

void lua_register_enginefuncs(lua_State *L)
{	
	for(unsigned int i = 0; s_functab[i].name; ++i)
		lua_register(L, s_functab[i].name, s_functab[i].func);
}
