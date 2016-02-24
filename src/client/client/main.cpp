// asio_client.cpp : Defines the entry point for the console application.
//



//int _tmain(int argc, _TCHAR* argv[])
//{
//	return 0;
//}

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/progress.hpp>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(tcp::v4(), argv[1], argv[2]);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		tcp::socket s(io_service);
		boost::asio::connect(s, iterator);
REDO:
		using namespace std; // For strlen.
		std::cout << "Enter message: ";
		char request[max_length];
		std::cin.getline(request, max_length);
		boost::progress_timer timer;

		size_t request_length = strlen(request) + 1 + 2 + 24;
		char request_[max_length+2+24];
		memset(request_,0,sizeof(request_));
		*(unsigned short *)request_ = (unsigned short)request_length;
		memcpy(request_+2+24,request,request_length);
		boost::asio::write(s, boost::asio::buffer(request_, request_length));
		/**/
		char reply[max_length];
		size_t reply_length = boost::asio::read(s,
			boost::asio::buffer(reply, 2));
		reply_length = boost::asio::read(s,
			boost::asio::buffer(reply+2, *(unsigned short*)reply-2));

		std::cout << "Reply is: ";
		std::cout.write(reply+2+24, reply_length-2-24 + 1);
		std::cout << "\n";
		std::cout << timer.elapsed() << endl;
		//


		system("pause");
		goto REDO;
		s.close();
		//io_service.stop();

	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
