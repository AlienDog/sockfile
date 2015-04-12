#ifdef __WIN32__
#include <winsock2.h>

char* sockGetErrorString() {
    char *s = NULL;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, (DWORD) WSAGetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR) &s, 0, NULL);
    return s;
}
#else
#include <arpa/inet.h>
char* sockGetErrorString() {
    return strerror(errno);
}
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
        printf("Failed to open file: %s\n", sockGetErrorString());
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
        printf("Failed to open socket: %s\n", sockGetErrorString());
#ifdef __WIN32__
        WSACleanup();
#endif
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(5000);
    address.sin_addr.s_addr = inet_addr(argv[1]);
    if(connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
        printf("Failed to connect: %s\n", sockGetErrorString());
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

    send(sock, (char*) &fileSize, sizeof(fileSize), 0);

    printf("Sending file...\n");
    fflush(stdout);

    uint64_t bufSize = 1024 * 16; // 16KB
    void* buf = malloc(bufSize);
    for(uint64_t pos = 0; pos < size; pos += bufSize) {
        size_t read = fread(buf, 1, bufSize, fd);
        send(sock, (char*) buf, read, 0);
    }

    printf("Waiting for server to finish receiving...\n");

    char temp;
    while(recv(sock, &temp, sizeof(temp), 0) != 0) {
        sleep(1);
    }

    printf("Closing...\n");

    close(sock);
    fclose(fd);

    printf("File successfully sent.\n");

#ifdef __WIN32__
    WSACleanup();
#endif

    return 0;
}