#ifndef net_h__
#define net_h__

#include <cstdlib>
#include <iostream>
#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/thread.hpp>
#include <boost/container/map.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/shared_array.hpp> 
#include <boost/container/deque.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/container/map.hpp>
#include "lua.hpp"


class session;
typedef boost::shared_ptr<session> session_ptr;
using boost::asio::ip::tcp;

boost::atomic<long long> g_session_id(0);

boost::container::deque<boost::tuple<long long,boost::shared_array<char>,unsigned short>> g_msgQueue;

lua_State *L = luaL_newstate();

boost::mutex g_mutex;

boost::container::map<long long ,session_ptr> g_sessionMap;



class session
	: public boost::enable_shared_from_this<session>
{
public:
	session(boost::asio::io_service& io_service)
		: socket_(io_service),m_sessionId(++g_session_id)
	{

	}

	tcp::socket& socket()
	{
		return socket_;
	}

	void start()
	{
		socket_.async_read_some(boost::asio::buffer(head_),
			boost::bind(&session::handle_read_header,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
		g_sessionMap[m_sessionId] = shared_from_this();
	}

	void handle_read_header(const boost::system::error_code& error,size_t bytes_transferred)
	{
		if (!error)
		{
			unsigned short len = *(unsigned short*)head_.data();
			boost::shared_array<char> buff(new char[len+1]);
			memset(buff.get(),0,len+1);
			//char * buff = new char[len];
			socket_.async_read_some(boost::asio::buffer(buff.get(),len),
				boost::bind(&session::handle_read_body,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,buff,len));
		}
		else
		{
			boost::mutex::scoped_lock(g_mutex);
			g_sessionMap.erase(m_sessionId);
			handle_close();
			//delete this;
		}
	}

	

	void handle_read_body(const boost::system::error_code& error,
		size_t bytes_transferred, boost::shared_array<char> data, unsigned short len)
	{
		if (!error)
		{
			boost::mutex::scoped_lock lock(g_mutex);
			g_msgQueue.push_back(boost::make_tuple(m_sessionId,data,len));
			socket_.async_read_some(boost::asio::buffer(head_),
				boost::bind(&session::handle_read_header,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			boost::mutex::scoped_lock(g_mutex);
			g_sessionMap.erase(m_sessionId);
			handle_close();
			//delete this;
		}
	}

	void do_write(const char * buff,size_t len)
	{
		boost::shared_array<char> buff_(new char[len+2+24]);
		memset(buff_.get(),0,len+2+24);
		*(unsigned short *)buff_.get() =  (unsigned short)len+24;
		memcpy(buff_.get()+2+24,buff,len);
		
		//assert(len+2+24 <= 65535);
		if(len+2+24 > 65535)
		{
			printf("msg too long");
			//return;
		}

// 		int length = len + 2 + 24;
// 		char * sendBuff = new char[length * 2];
// 		memcpy(sendBuff,buff_.get(),length);
// 		memcpy(sendBuff+length,buff_.get(),length);
// 		boost::shared_array<char> buff1(sendBuff);
// 
// 
// 		socket_.async_write_some(
// 			boost::asio::buffer(buff1.get(), length + 30),
// 			boost::bind(&session::handle_write, shared_from_this(),boost::asio::placeholders::error, buff_));
// 
// 		boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
// 
// 		socket_.async_write_some(
// 			boost::asio::buffer(buff1.get()+length+30, length - 30),
// 			boost::bind(&session::handle_write, shared_from_this(),boost::asio::placeholders::error, buff_));

		
		
		socket_.async_write_some(
			boost::asio::buffer(buff_.get(), len+2+24),
			boost::bind(&session::handle_write, shared_from_this(),boost::asio::placeholders::error, buff_));




		//boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));

	}


	void handle_write(const boost::system::error_code& error, boost::shared_array<char> buff){
		if (!error) {

		}else{
			boost::mutex::scoped_lock(g_mutex);
			handle_close();
			//delete this;
		}
	}

	void handle_close()
	{
		boost::shared_array<char> msg(new char[1024]);
		memset(msg.get(),0,sizeof(msg));
		char *msgBody = "{\"cmd\":11,\"param\":[]}";
		int msgLen = strlen(msgBody) + 1;
		memcpy(msg.get()+24,msgBody,msgLen);
		g_msgQueue.push_back(boost::make_tuple(m_sessionId,msg,msgLen+24));
	}


	~session()
	{

	}

	void close_socket()
	{
		this->socket_.shutdown(boost::asio::socket_base::shutdown_both);
		this->socket_.close();
	}

public:
	long long  m_sessionId;
private:
	tcp::socket socket_;
	boost::array<char,2> head_;
};


class server
{
public:
	server(boost::asio::io_service& io_service, short port)
		: io_service_(io_service),
		acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
	{
		session_ptr new_session(new session(io_service_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}

	void handle_accept(session_ptr new_session,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			new_session->start();
		}

		new_session.reset(new session(io_service_));
		acceptor_.async_accept(new_session->socket(),
			boost::bind(&server::handle_accept, this, new_session,
			boost::asio::placeholders::error));
	}
private:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};


//active script
void ScriptActive(){
	boost::shared_array<char> msg(new char[1024]);
	memset(msg.get(),0,sizeof(msg));
	char *msgBody = "{\"cmd\":12,\"param\":[]}";
	int msgLen = strlen(msgBody) + 1;
	memcpy(msg.get()+24,msgBody,msgLen);
	g_msgQueue.push_back(boost::make_tuple(0,msg,msgLen+24));
}


void logicFunc()
{
	long long llBegin = time(NULL);
	while (true)
	{
		long long llNow = time(NULL);
		boost::mutex::scoped_lock(g_mutex);
		if(g_msgQueue.size() > 0){
			boost::tuple<long long,boost::shared_array<char>,unsigned short> msg = g_msgQueue.front();
			g_msgQueue.pop_front();
			lua_getglobal(L,"main");
			lua_pushinteger(L,msg.get<0>());
			const char* body = (const char *)msg.get<1>().get() + 24;
			unsigned short len = msg.get<2>() - 24;
			lua_pushstring(L,body);
			lua_pcall(L,2,0,0);		
			lua_settop(L,0);
			//printf("g_sessionMap.size()=%d\n",g_sessionMap.size());
		}
		else{
			boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
		}
		if(llNow - llBegin >= 10)
		{
			llBegin = llNow;
			ScriptActive();
		}

	}
}




//send data to client 
int sendFunc(lua_State *L)
{
	boost::mutex::scoped_lock lock(g_mutex);
	long long  sessionId = lua_tointeger(L,-2);
	size_t len = 0;
	const char * msg = luaL_tolstring(L,-1,&len);
	
	boost::container::map<long long ,session_ptr>::iterator iter = g_sessionMap.find(sessionId);
	if(len < 200)
	{
		printf("#do send:%s\n",msg);
	}
	if(iter != g_sessionMap.end())
	{
		iter->second.get()->do_write(msg,len+1);
	}
	return 0;
}

//close session
int closeSession(lua_State *L)
{
	boost::mutex::scoped_lock lock(g_mutex);
	long long  sessionId = lua_tointeger(L,-1);
	boost::container::map<long long ,session_ptr>::iterator iter = g_sessionMap.find(sessionId);
	if(iter != g_sessionMap.end()){
		iter->second.get()->close_socket();
	}
	return 0;
}




// int main(int argc, char* argv[])
// {
// 	luaL_openlibs(L);
// 	luaL_dofile(L,"script/main.lua");
// 	lua_register(L,"send",sendFunc);
// 	try
// 	{
// 		if (argc != 2)
// 		{
// 			std::cerr << "Usage: server <port>\n";
// 			return 1;
// 		}
// 
// 		boost::asio::io_service io_service;
// 
// 		using namespace std; // For atoi.
// 		server s(io_service, atoi(argv[1]));
// 		boost::thread logicThread(boost::bind(logicFunc));
// 		logicThread.detach();
// 		io_service.run();
// 	}
// 	catch (std::exception& e)
// 	{
// 		std::cerr << "Exception: " << e.what() << "\n";
// 	}
// 
// 	return 0;
// }

#endif // net_h__
