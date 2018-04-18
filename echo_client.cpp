#include <arpa/inet.h>
#include <assert.h>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void client(int argc, const char *argv[]);

int main(int argc, const char *argv[]) {
    client(argc, argv);

    return 0;
}

void client(int argc, const char * argv[]){
    if (argc < 2) {
        exit(-1);
    }

    // Get ip and port.
    int port = atoi(argv[1]);

    // Create socket.
    int fd = socket(PF_INET,SOCK_STREAM, 0);
    assert(fd >= 0);
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "localhost", &address.sin_addr);
    int ret = connect(fd, (struct sockaddr*)&address, sizeof(address));
    assert(ret >= 0);

    // Start communicate with server.
    std::string t;
    char r_buf[1024];
    int r_size;
    int n;
    while (true) {
        // Read data from stdin.
        std::cin >> t;
        if (t == "q" || t == "") {
            break;
        }

        // Write data to fd.
        int w_size = htonl(t.size());
        n = write(fd, (char *)(&w_size), sizeof(w_size));
        if (n != sizeof(r_size)) {
            std::cout << "Client write pdu size failed" << std::endl;
            break;
        }
        n = write(fd, t.c_str(), t.size());
        if (n != t.size()) {
            std::cout << "Client write pdu failed" << std::endl;
            break;
        }
        std::cout << "Client write data: " << t << std::endl;

        // Read data from fd.
        n = read(fd, (char *)(&r_size), sizeof(r_size));
        if (n != sizeof(r_size)) {
            std::cout << "Client read pdu size failed" << std::endl;
            break;
        }
        r_size = ntohl(r_size);
        n = read(fd, r_buf, r_size);
        if (n != r_size) {
            std::cout << "Client read pdu failed" << std::endl;
            break;
        }
        std::cout << "Client read data: " << std::string(r_buf, r_size) << std::endl;
    }

    close(fd);
}
