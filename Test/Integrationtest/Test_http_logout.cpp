extern "C" {
#include "../../http_server_header.h"   
#include "../../sqlite3.h"
}

#include <gtest/gtest.h>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>


class HTTPLogoutTest : public testing::Test {
protected:
    int sockets[2];

    int session_id_length = 24;
    char* session_id = (char*)malloc(session_id_length + 1);


    void SetUp() override {
        sqlite3* db;
        sqlite3_open("test_database.db", &db);

        sqlite3_stmt* stmt;

        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Sessions;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Sessions (id_session_id INTEGER PRIMARY KEY, session_id TEXT NOT NULL, user_id INTEGER NOT NULL, expiry INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);


        generate_session_id(session_id_length, session_id);

        sqlite3_prepare_v2(db, "INSERT INTO Sessions (session_id, user_id, expiry) VALUES(?, 1, 1214312);", -1, &stmt, NULL);
        ASSERT_EQ(sqlite3_bind_text(stmt, 1, session_id, -1, SQLITE_TRANSIENT), SQLITE_OK);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    }


    void TearDown() override {
        if(sockets[0] != -1) {
            close(sockets[0]);
        }
        if(sockets[1] != -1) {
            close(sockets[1]);
        }

        sqlite3* db;
        sqlite3_open("test_database.db", &db);

        sqlite3_stmt* stmt;
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Sessions;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
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


TEST_F(HTTPLogoutTest, CorrectSessionId) {
    char buffer[300];
    snprintf(buffer, 300, "POST /logout HTTP/1.1\r\nCookie: sessionId=%s\r\n", session_id);
    char db_path[] = "test_database.db";

    ASSERT_EQ(http_logout_routine(sockets[0], buffer, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: main_page.html"), std::string::npos);
    EXPECT_NE(response.find("Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n"), std::string::npos);
}


TEST_F(HTTPLogoutTest, IncorrectSessionId) {
    char buffer[300];
    snprintf(buffer, 300, "POST /logout HTTP/1.1\r\nCookie: sessionId=FSSCeSAdw343sSA\r\n");
    char db_path[] = "test_database.db";

    ASSERT_EQ(http_logout_routine(sockets[0], buffer, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: main_page.html"), std::string::npos);
    EXPECT_NE(response.find("Set-Cookie: sessionId=; Max-Age=0; Path=/; HttpOnly; SameSite=Lax\r\n"), std::string::npos);
}

TEST_F(HTTPLogoutTest, IncorrectDBPath) {
    char buffer[300];
    snprintf(buffer, 300, "POST /logout HTTP/1.1\r\nCookie: sessionId=FSSCeSAdw343sSA\r\n");
    char db_path[] = "wrong_path";

    ASSERT_EQ(http_logout_routine(sockets[0], buffer, db_path), -3);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 500 Internal Server Error\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n\r\n"), std::string::npos);
}



