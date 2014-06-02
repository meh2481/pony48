#include "luafuncs.h"
#include "luainterface.h"
#include "Pony48.h"

extern Pony48Engine* g_pGlobalEngine;
static PonyLua pL;

void PonyLua::fireParticles(string sSys, bool bActive)
{
	if(g_pGlobalEngine->songParticles.count(sSys))
		g_pGlobalEngine->songParticles[sSys]->firing = bActive;
}

void PonyLua::showParticles(string sSys, bool bShow)
{
	if(g_pGlobalEngine->songParticles.count(sSys))
		g_pGlobalEngine->songParticles[sSys]->show = bShow;
}

void PonyLua::pinwheelbg_rotspeed(float speed)
{
	if(g_pGlobalEngine->m_bg != NULL && g_pGlobalEngine->m_bg->type == PINWHEEL)
	{
		pinwheelBg* pBg = (pinwheelBg*) g_pGlobalEngine->m_bg;
		pBg->speed = speed;
	}
}

luaFunc(fireparticles)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	pL.fireParticles(s,b);
	luaReturnNil();
}

luaFunc(showparticles)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	pL.showParticles(s,b);
	luaReturnNil();
}

luaFunc(pinwheelspeed)
{
	float32 f = lua_tonumber(L, 1);
	pL.pinwheelbg_rotspeed(f);
	luaReturnNil();
}

static LuaFunctions s_functab[] =
{
	luaRegister(fireparticles),
	luaRegister(showparticles),
	luaRegister(pinwheelspeed),
	{NULL, NULL}
};

void lua_register_enginefuncs(lua_State *L)
{	
	for(unsigned int i = 0; s_functab[i].name; ++i)
		lua_register(L, s_functab[i].name, s_functab[i].func);
}
