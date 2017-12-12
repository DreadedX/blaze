#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdexcept>

#include <iostream>

int main() {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("");
		throw std::runtime_error("Failed socket");
	}

	struct sockaddr_in address;
	memset(&address, '0', sizeof(address));

	address.sin_family = AF_INET;
	address.sin_port = htons(50000);

	if (inet_pton(AF_INET, "192.168.178.75", &address.sin_addr) <= 0) {
		perror("");
		throw std::runtime_error("Invalid address");
	}

	if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("");
		throw std::runtime_error("Connection failed");
	}

	const char* hello = "Hello world! This is a message";
	send(sock, hello, strlen(hello), 0);

	std::vector<char> buffer(1024);
	int valread = read(sock, buffer.data(), buffer.size());

	printf("%s\n", buffer.data());

}
