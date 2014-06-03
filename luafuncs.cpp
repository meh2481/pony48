#include "luafuncs.h"
#include "luainterface.h"
#include "Pony48.h"

extern Pony48Engine* g_pGlobalEngine;

//Class for interfacing between Pony48Engine and Lua
//(defined here instead of inside header because of weird cross-inclusion stuff)
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
	
	static Vec3* getCamera()
	{
		return &g_pGlobalEngine->CameraPos;
	}
	
	static TilePiece* getTile(uint32_t num)
	{
		uint32_t x = num % 4;
		uint32_t y = num / 4;
		if(x < BOARD_WIDTH && y < BOARD_HEIGHT)
			return g_pGlobalEngine->m_Board[x][y];
		return NULL;
	}
};

luaFunc(fireparticles)	//fireparticles(string particleSysName, bool bFire)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->firing = b;
	luaReturnNil();
}

luaFunc(showparticles)	//showparticles(string particleSysName, bool bShow)
{
	string s = getStr(L, 1);
	bool b = getBool(L, 2);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->show = b;
	luaReturnNil();
}

luaFunc(resetparticles)	//resetparticles(string particleSysName)
{
	string s = getStr(L, 1);
	ParticleSystem* sys = PonyLua::getParticleSys(s);
	if(sys)
		sys->killParticles();
	luaReturnNil();
}

luaFunc(pinwheelspeed)	//pinwheelspeed(float speed)
{
	float32 f = lua_tonumber(L, 1);
	pinwheelBg* bg = (pinwheelBg*)PonyLua::getBg();
	if(bg)
		bg->speed = f;
	luaReturnNil();
}

luaFunc(setcameraxy)	//setcameraxy(float x, float y)
{
	float32 x = lua_tonumber(L, 1);
	float32 y = lua_tonumber(L, 2);
	Vec3* cam = PonyLua::getCamera();
	cam->x = x;
	cam->y = y;
	luaReturnNil();
}

luaFunc(settilecol)	//settilecol(int tile, float r, float g, float b, float a)
{
	int num = lua_tointeger(L, 1);
	TilePiece* pc = PonyLua::getTile(num);
	if(pc != NULL)
	{
		Color col;
		col.r = lua_tonumber(L, 2);
		col.g = lua_tonumber(L, 3);
		col.b = lua_tonumber(L, 4);
		col.a = lua_tonumber(L, 5);
		if(col.r == 1 && col.g == 1 && col.b == 1 && col.a == 1)
			pc->bg->col = pc->origCol;
		else
			pc->bg->col = col;
	}
	luaReturnNil();
}

static LuaFunctions s_functab[] =
{
	luaRegister(fireparticles),
	luaRegister(showparticles),
	luaRegister(resetparticles),
	luaRegister(pinwheelspeed),
	luaRegister(setcameraxy),
	luaRegister(settilecol),
	{NULL, NULL}
};

void lua_register_enginefuncs(lua_State *L)
{	
	for(unsigned int i = 0; s_functab[i].name; ++i)
		lua_register(L, s_functab[i].name, s_functab[i].func);
}
