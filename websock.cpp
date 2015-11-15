#include "websock.h"

WebsockFrontend::WebsockFrontend() {
	printf("WebsockFrontend::WebsockFrontend()\n");
}
WebsockFrontend::~WebsockFrontend() {
	printf("WebsockFrontend::~WebsockFrontend()\n");
}


bool WebsockFrontend::InitSocket() {
	printf("WebsockFrontend::InitSocket()\n");

	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->sock < 0) {
		perror("WebsockFrontend::InitSocket: Socket create failed");
		return false;
	}

	sockaddr_in desc;
	desc.sin_family = AF_INET;
	desc.sin_port = htons(SOCK_PORT);
	desc.sin_addr.s_addr = 0; // 0.0.0.0 

	if (bind(this->sock, (sockaddr*)&desc, sizeof(desc)) != 0) {
		perror("WebsockFrontend::InitSocket: Bind failed");
		return false;
	}


	if (listen(this->sock, 16) != 0) {
		perror("WebsockFrontend::InitSocket: Listen failed");
		return false;
	}

	return true;
}

bool WebsockFrontend::InitEpoll() {
	printf("WebsockFronted::InitEpoll()\n");
	this->epfd = epoll_create(2);
	if (this->epfd < 0) {
		perror("WebsockFrontend::InitEpoll: epoll_create failed");
		return false;
	}

	epoll_event evt;
	evt.events = EPOLLIN | EPOLLRDHUP;
	evt.data.u64 = 0; // 0 => main socket (otherwise it will be pointer to struct)
	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->sock, &evt) != 0) {
		perror("WebsockFrontend::InitEpoll: epoll_ctl add main socket failed");
	}

	return true;
}

bool WebsockFrontend::Init() {
	return this->InitSocket() && this->InitEpoll();
}

void WebsockFrontend::Run() {
	this->running = true;
	this->thrd = std::thread(&WebsockFrontend::RunThrd, this);
}
void WebsockFrontend::Stop() {
	this->running = false;
	this->thrd.join();
}

void WebsockFrontend::Accept() {
	printf("WebsockFrontend::Accept()\n");
	sockaddr_in addr;
	
	socklen_t len;
	int fd = accept(this->sock, (sockaddr*)&addr, &len);
	if (fd < 0) {
		perror("WebsockFrontend::Accept: Accept fail");
		return;
	}

	char straddr[INET_ADDRSTRLEN];
	printf("Somebody connected: %s, port %d\n", inet_ntop(AF_INET, &addr.sin_addr, straddr, sizeof(straddr)), ntoh(addr.sin_port));
}

void WebsockFrontend::RunThrd() {
	epoll_event event;
	while(this->running) {
		int wait = epoll_wait(this->epfd, &event, 1, 500);

		if (wait == 0) { printf("."); fflush(stdout); continue; }

		if(event.data.u64 == 0) { this->Accept(); continue; }


	}
}
