#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "check.h"
#include "event_callback.h"
#include "event_manager.h"

class ServerCallback : public EventCallback {
 public:
    ServerCallback() : 
     _r_size(0), _r_pos(0), _w_size(0), _w_pos(0), _w(false) {
        _event_manager = EventManager::Instance();
    }

    virtual void read_cb(int fd) {
        int sz, n;
        if (_r_size == 0) {
            n = read(fd, &sz, sizeof(sz));
            if (n <= 0 && errno == EAGAIN) {
                return;
            }
            if (n != sizeof(sz)) {
                std::cout << "Server read pdu size from fd:" << fd << " failed" << std::endl;
                _event_manager->del_callback(fd, EVENT_RO);
                _r_pos = 0;
                return;
            }
            _r_size = ntohl(sz);
        }

        n = read(fd, _r_buf+_r_pos, _r_size-_r_pos);
        if (n <= 0 && errno == EAGAIN) {
            return;
        }
        if (n <= 0) {
            std::cout << "Server read pdu fd " << fd << " failed" << std::endl;
            _event_manager->del_callback(fd, EVENT_RO);
            _r_pos = 0;
            return;
        }

        _r_pos += n;
        if (_r_pos == _r_size) {
            std::cout << "Server read data: "
                      << std::string(_r_buf, _r_size) << std::endl;
            _event_manager->del_callback(fd, EVENT_RO);
            bcopy(_r_buf, _w_buf, _r_size);
            _w_size = _r_size;
            CHECK_EQ(_event_manager->add_callback(fd, EVENT_WO, this) == true);
            _r_pos = 0;
            _r_size = 0;
        }
    }

    virtual void write_cb(int fd) {
        if (!_w) {
            _w = true;
            int sz = htonl(_w_size);
            int n = write(fd, (char *)(&sz), sizeof(sz));
            if (n <= 0 && errno == EAGAIN) {
                return;
            }
            if (n != sizeof(sz)) {
                std::cout << "Server write pdu size to fd " << fd << " failed" << std::endl;
                _event_manager->del_callback(fd, EVENT_WO);
            }

            _w_pos = 0;
        }

        int n = write(fd, _w_buf+_w_pos, _w_size-_w_pos);
        if (n <= 0 && errno == EAGAIN) {
            return;
        }
        if (n <= 0) {
            std::cout << "Server write pdu to fd " << fd << " failed" << std::endl;
            _event_manager->del_callback(fd, EVENT_WO);
            _w_pos = 0;
            _w_size = 0;
            _w = false;
        }
        _w_pos += n;

        if (_w_pos == _w_size) {
            std::cout << "Server write data: "
                      << std::string(_w_buf, _w_size) << std::endl;
            _event_manager->del_callback(fd, EVENT_WO);
            CHECK_EQ(_event_manager->add_callback(fd, EVENT_RO, this) == true);
            _w_pos = 0;
            _w_size = 0;
            _w = false;
        }
    }

    virtual void error_cb(int fd, int events) {
        std::cout << "Server got error on fd:" << fd << ", close it." << std::endl;
        _event_manager->del_callback(fd, EVENT_RW);
        close(fd);
    }

 private:
    char _r_buf[1024];
    int _r_size;
    int _r_pos;

    bool _w;
    char _w_buf[1024];
    int _w_size;
    int _w_pos;

    EventManager* _event_manager;
};

int main(int argc, char *argv[]) {
    auto event_manager = EventManager::Instance();

    if (argc != 2) {
        std::cout << "You should input port as param." << std::endl;
        return -1;
    }

    int port = atoi(argv[1]);
    std::cout << "port:" <<  port << std::endl;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0 ) {
        std::cout << "Create sock fd failed." << std::endl;
        return -1;
    }
    assert(sock >= 0);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_port = htons(port);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定ip和端口
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cout << "Bind sock fd failed." << std::endl;
        return -1;
    }

    if (listen(sock, 5) < 0) {
        std::cout << "Listen sock fd failed." << std::endl;
        return -1;
    }
    
    while (true) {
        struct sockaddr_in c_addr;
        socklen_t len = sizeof(c_addr);
        // sock is in block mode.
        int fd = accept(sock, (struct sockaddr*)(&c_addr), &len);
        if (fd < 0 ) {
            std::cout << "Accept sock fd failed." << std::endl;
            break;
        }
        // Set fd non-block.
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0) {
            std::cout << "Get fd:" << fd << " flags failed.";
            close(fd);
            continue;
        }
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) < 0) {
            std::cout << "Set fd:" << fd << " flags failed.";
            close(fd);
            continue;
        }

        auto callback = new ServerCallback();
        if (!event_manager->add_callback(fd, EVENT_RO, callback)) {
            std::cout << "Add fd:" << fd << " callback failed";
            close(fd);
            continue;
        }
    }

    close(sock);

    return 0;
}
