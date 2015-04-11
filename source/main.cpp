#ifdef __WIN32__
#include <winsock2.h>
#include <ws2tcpip.h>

int inet_pton(int af, const char* src, void* dst) {
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN + 1];

	ZeroMemory(&ss, sizeof(ss));
	strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
	src_copy[INET6_ADDRSTRLEN] = 0;

	if(WSAStringToAddress(src_copy, af, NULL, (struct sockaddr*) &ss, &size) == 0) {
		switch(af) {
			case AF_INET:
				*(struct in_addr*) dst = ((struct sockaddr_in*) &ss)->sin_addr;
				return 1;
			case AF_INET6:
				*(struct in6_addr*) dst = ((struct sockaddr_in6*) &ss)->sin6_addr;
				return 1;
		}
	}

	return 0;
}
#else
#include <arpa/inet.h>
#endif

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <string>

int main(int argc, const char* argv[]) {
	if(argc != 3) {
		printf("Usage: %s ip file\n", argv[0]);
		return -1;
	}

#ifdef __WIN32__
	WORD versionWanted = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(versionWanted, &wsaData);
#endif

	FILE* fd = fopen(argv[2], "r");
	if(!fd) {
		printf("Failed to open file: %s\n", strerror(errno));
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

	fseek(fd, 0, SEEK_END);
	uint64_t size = (uint64_t) ftell(fd);
	fseek(fd, 0, SEEK_SET);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0) {
		printf("Failed to open socket: %s\n", strerror(errno));
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(5000);
	inet_pton(AF_INET, argv[1], &address.sin_addr);

	if(connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
		printf("Failed to connect: %s\n", strerror(errno));
#ifdef __WIN32__
		WSACleanup();
#endif
		return -1;
	}

	printf("Sending info...\n");
	fflush(stdout);

	uint64_t fileSize = size;
	static const int num = 42;
	if(*((char*) &num) == num) {
		fileSize = (((uint64_t) htonl((uint32_t) fileSize)) << 32) + htonl((uint32_t) (fileSize >> 32));
	}

	write(sock, &fileSize, sizeof(fileSize));

	printf("Sending file...\n");
	fflush(stdout);

	uint64_t bufSize = 1024 * 16; // 16KB
	void* buf = malloc(bufSize);
	for(uint64_t pos = 0; pos < size; pos += bufSize) {
		size_t read = fread(buf, 1, bufSize, fd);
		write(sock, buf, read);
	}

	close(sock);
	fclose(fd);

	printf("File successfully sent.\n");

#ifdef __WIN32__
	WSACleanup();
#endif

	return 0;
}