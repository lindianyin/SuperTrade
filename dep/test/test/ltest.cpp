#include <winsock.h>
#include "lua.hpp"
#include "mysql.h"


static int Ladd(lua_State *L)
{
	lua_Integer a = lua_tointeger(L,1);
	lua_Integer b  = lua_tointeger(L,2);
	lua_pushinteger(L,a+b);
	return 1;
}


static int Linit(lua_State *L)
{
	MYSQL *pMysql = new MYSQL();
	mysql_init(pMysql);
	mysql_real_connect(pMysql,"183.136.221.50","root","gywl2013","booktest",3306,NULL,0);
	printf(mysql_error(pMysql));







   delete pMysql;
}






static const luaL_Reg R[] =
{
	{ "myadd",	Ladd},
	{ NULL,		NULL	}
};

extern "C" __declspec(dllexport) int luaopen_test(lua_State *L)
{
	luaL_newlib(L,R);
	return 1;
}
