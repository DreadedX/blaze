#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <cstring>

#include <iostream>

int main() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == 0) {
		perror("");
		throw std::runtime_error("Failed socket");
	}

	int opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("");
		throw std::runtime_error("Failed setsockopt");
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(50000);

	if (bind(sockfd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("");
		throw std::runtime_error("Failed bind");
	}

	if (listen(sockfd, 3) <  0) {
		throw std::runtime_error("Failed listen");
	}

	int addrlen = sizeof(address);
	int newsock = accept(sockfd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
	if (newsock < 0) {
		throw std::runtime_error("Failed accept");
	}

	std::vector<char> buffer(1024);
	int valread = read(newsock, buffer.data(), buffer.size());
	printf("%s\n", buffer.data());
	const char* hello = "Hello world!";
	send(newsock, hello, strlen(hello), 0);
}
