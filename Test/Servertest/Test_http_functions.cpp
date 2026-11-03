extern "C" {
#include "../../http_server_header.h"   
}

#include <gtest/gtest.h>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>


class HTTPFunctionsTest : public testing::Test {
protected:
    int sockets[2];

    void SetUp() override {
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    }

    void TearDown() override {
        close(sockets[0]);
        close(sockets[1]);
    }

    std::string recieve_response() {
        shutdown(sockets[0], SHUT_WR);

        char buffer[2048];
        std::string response;


        size_t bytes_recieved = 0;

        while((bytes_recieved = recv(sockets[1], buffer, sizeof(buffer), 0) > 0)) {
            response.append(buffer, bytes_recieved);
        }

        return response;
    }
};
