#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <sys/epoll.h>

#include "config.h"


class WebsockFrontend {
	public:
		WebsockFrontend();
		~WebsockFrontend();
		bool Init();
		void Run();
		void Stop();
	private:
		bool InitSocket();
		bool InitEpoll();
		void RunThrd();

		void Accept();

		std::thread thrd;
		int sock;
		bool running;

		int epfd; // epoll fd

};
