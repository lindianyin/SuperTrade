#include <cstdio>
#include <iostream>
#include "net.h"
using namespace std;
void print(const boost::system::error_code& e){
	std::cout<<"hello asio"<<std::endl;
}

int main(int argc,char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: server <port>\n";
	 		return 1;
	}
	luaL_openlibs(L);
	lua_register(L,"send",sendFunc);
	int ret = luaL_dofile(L,"script/main.lua");
	if(ret){
		printf(lua_tostring(L,-1));
	}

	boost::asio::io_service io_service;
	server server(io_service, atoi(argv[1]));
	boost::thread logicThread(boost::bind(logicFunc));
	logicThread.detach();
	io_service.run();
	return 0;
}