#include "websock.h"

WebsockFrontend::WebsockFrontend() {
	printf("WebsockFrontend::WebsockFrontend()\n");

	this->inputnames = 0;
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

WSClient * WebsockFrontend::NewClient(int fd) {
	WSClient * ret = new WSClient();
	ret->fd = fd;

	return ret;
}

void WebsockFrontend::CloseClient(WSClient * client) {
	printf("WebsockFrontend::CloseClient()\n");

	epoll_event evt;
	if (epoll_ctl(this->epfd, EPOLL_CTL_DEL, client->fd, &evt) != 0) {
		perror("WebsockFrontend::CloseClient: epoll_ctl delete failed");
	}

	if (close(client->fd) != 0) {
		perror("WebsockFrontend::CloseClient: close failed");
	}

	delete client;
	printf("WebsockFrontend::CloseClient: Client removed\n");
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
	printf("WebsockFrontend::Accept: connected: %s, port %d\n", inet_ntop(AF_INET, &addr.sin_addr, straddr, sizeof(straddr)), ntohs(addr.sin_port));

	WSClient * client = this->NewClient(fd);

	epoll_event evt;
	evt.data.ptr = client;
	evt.events = EPOLLIN;// | EPOLLRDHUP;

	if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &evt) != 0) {
		perror("WebsockFrontend::Accept: epoll_ctl add accepted socket failed");

		this->CloseClient(client);

		return;
	}


	this->SendHello(client);
}

void WebsockFrontend::ProcessEvent(WSClient * client) {
	char buffer[1024];
	int rd = read(client->fd, buffer, sizeof(buffer));

	if (rd == 0) { // disconnected? 
		this->CloseClient(client);
		return;
	}


}


void WebsockFrontend::RunThrd() {
	epoll_event event;
	while(this->running) {
		int wait = epoll_wait(this->epfd, &event, 1, 1000);

		if (wait == 0) {  // nothing has arrived
			printf("."); 
			fflush(stdout); 
			continue; 
		}

		if(event.data.u64 == 0) {  // read possible on main socket -> client connected
			this->Accept(); 
			continue; 
		}

		if (event.events == EPOLLIN) {
			this->ProcessEvent((WSClient*)event.data.ptr);
		}
		else {
			printf("eeeeee: %d\n", event.events);
		}
	}
}

void WebsockFrontend::SetInputNames(const char * const * inputnames) {
	this->inputnames = inputnames;
}

void WebsockFrontend::SendHello(WSClient * client) {
	char buf[1024];

	strcpy(buf, "LMCM"); // header
	strcpy(buf + 4, "HELLO"); // packet type
	buf[9] = INPUTS; // Number of INputs
	buf[10] = OUTPUTS; // Number of OUTputs


	char * ptr = &buf[11];
	if (this->inputnames != 0) {
		for (int i = 0; i < INPUTS; i++) {
			*ptr++ = strlen(this->inputnames[i]);
			strcpy(ptr, this->inputnames[i]);
			ptr += strlen(this->inputnames[i]);
		}
	}
	else {
		*ptr++ = 0;
	}


	size_t bufs = ptr - buf;
	int snd = send(client->fd, buf, bufs, 0);

	if (snd != bufs) {
		printf("WebsockFrontend::SendHello: Incomplete write (%d of %d)\n", snd, bufs);
	}
}
