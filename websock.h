#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <sys/epoll.h>
#include <string.h>

#include "config.h"


struct WSClient {
	int fd;
};
class WebsockFrontend {
	public:
		WebsockFrontend();
		~WebsockFrontend();
		bool Init();
		void Run();
		void Stop();


		void SetInputNames(const char * const * names);
	private:
		bool InitSocket();
		bool InitEpoll();
		void RunThrd();

		void Accept();
		WSClient * NewClient(int fd);
		void CloseClient(WSClient * client);
		void ProcessEvent(WSClient * client);

		void SendHello(WSClient * client);

		std::thread thrd;
		int sock;
		bool running;

		int epfd; // epoll fd
		const char * const * inputnames;

};


/* WEBSOCK PROTOCOL:
 * Server -> Client after connect: HELLO ("LMCM", "HELLO", [1B] INs, [1B] stereo OUTs)
 *
 * GETIO: Get current levels
 * Client -> Server get cur. values ("LMCM", "GETIO", [1B] select out)
 * Server -> Client reply ("LMCM", "GETIO", [1B] select out, [2B] IN level1, pan1, [2B] IN level2, pan2 ....)
 *
 * SETIO: Set new levels
 * Client -> Server ("LMCM", "SETIO", [1B] select out, [2B] IN level1, pan1, [2B] IN level2, pan2 ....)
 * */
