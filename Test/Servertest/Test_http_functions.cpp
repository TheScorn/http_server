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
        if(sockets[0] != -1) {
            close(sockets[0]);
        }
        if(sockets[1] != -1) {
            close(sockets[1]);
        }
        
    }

    std::string recieve_response() {
        shutdown(sockets[0], SHUT_WR);

        char buffer[2048];
        std::string response;


        while(true) {
            ssize_t n = recv(sockets[1], buffer, sizeof(buffer), 0);

            if(n > 0) {
                response.append(buffer, n);
            }
            else {
                break;
            }

        }

        

        return response;
    }
};


TEST_F(HTTPFunctionsTest, send200OK) {

    const char* message = "{\"success\":true}";

    ASSERT_EQ(send_http_200_OK(sockets[0], (char*)message), 0);

    std::string response = recieve_response();


    EXPECT_NE(response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 16\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\n{\"success\":true}"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send200OKsetCookie) {

    const int session_id_len = 24;
    const int session_id_lifespan = 10;
    char session_id[session_id_len + 1];
    generate_session_id(session_id_len, session_id);



    ASSERT_EQ(send_http_200_OK_set_cookie(sockets[0], session_id, session_id_lifespan), 0);
    std::string response = recieve_response();
    std::string setCookie = "Set-Cookie: sessionId=";
    setCookie.append(session_id);


    EXPECT_NE(response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(response.find(setCookie), std::string::npos);
    EXPECT_NE(response.find("Path=/; HttpOnly; SameSite=Lax; Max-Age="), std::string::npos);

}

TEST_F(HTTPFunctionsTest, send201Created) {

    const char* message = "{\"success\":true}";

    ASSERT_EQ(send_http_201_Created(sockets[0], (char*)message), 0);

    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 201 Created\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: application/json\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 16\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\n{\"success\":true}"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send302FoundMain) {
    ASSERT_EQ(send_http_302_Found_main(sockets[0], false), 0);

    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: main_page.html"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send302FoundMainCookie) {
    ASSERT_EQ(send_http_302_Found_main(sockets[0], true), 0);

    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: main_page.html"), std::string::npos);
    EXPECT_NE(response.find("Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send302FoundLogin) {
    ASSERT_EQ(send_http_302_Found_login(sockets[0], false), 0);

    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: /login_page.html"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send302FoundLoginCookie) {
    ASSERT_EQ(send_http_302_Found_login(sockets[0], true), 0);

    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: /login_page.html"), std::string::npos);
    EXPECT_NE(response.find("Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send400BadRequest) {
    const char* error = "Request Malformed!";

    ASSERT_EQ(send_http_400_Bad_Request(sockets[0], (char*)error), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 400 Bad Request\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 18\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\nRequest Malformed!"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send401Unauthorized) {
    ASSERT_EQ(send_http_401_Unauthorized(sockets[0]), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 401 Unauthorized\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send403Forbidden) {
    const char* error = "Insufficient access rights!";

    ASSERT_EQ(send_http_403_Forbidden(sockets[0], (char*)error), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 403 Forbidden\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 27\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\nInsufficient access rights!"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send404NotFound) {
    ASSERT_EQ(send_http_404_Not_Found(sockets[0]), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 404 Not Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 13\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\n404 Not Found"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send414URITooLong) {
    const int uri_max = 4096;
    


    ASSERT_EQ(send_http_414_Request_URI_Too_Long(sockets[0], uri_max), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 414 URI Too Long\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 52\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\nURI exeeds the maximum allowed length of 4096 bytes."), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send500InternalServerError) {
    ASSERT_EQ(send_http_500_Internal_Server_Error(sockets[0], NULL), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 500 Internal Server Error\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n\r\n"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send502BadGateway) {
    ASSERT_EQ(send_http_502_Bad_Gateway(sockets[0]), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 502 Bad Gateway\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
}

TEST_F(HTTPFunctionsTest, send503ServiceUnavailable) {
    const char* error = "Service currently unavailable.";

    ASSERT_EQ(send_http_503_Service_Unavailable(sockets[0], (char*)error), 0);
    std::string response = recieve_response();

    EXPECT_NE(response.find("HTTP/1.1 503 Service Unavailable\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: text/plain\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 30\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n"), std::string::npos);
    EXPECT_NE(response.find("\r\n\r\nService currently unavailable."), std::string::npos);
}



