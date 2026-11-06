extern "C" {
#include "../../http_server_header.h"   
#include "../../sqlite3.h"
}

#include <gtest/gtest.h>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>

class HTTPLoginTest : public testing::Test {
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
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Users;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Sessions (id_session_id INTEGER PRIMARY KEY, session_id TEXT NOT NULL, user_id INTEGER NOT NULL, expiry INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Users (user_id INTEGER PRIMARY KEY, username TEXT NOT NULL, password TEXT NOT NULL, email TEXT);", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);        

        sqlite3_finalize(stmt);


        generate_session_id(session_id_length, session_id);

        sqlite3_prepare_v2(db, "INSERT INTO Users (username, password) VALUES(\"admin\", \"passwd\");", -1, &stmt, NULL);
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
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE  IF EXISTS Users;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        free(session_id);
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


TEST_F(HTTPLoginTest, CorrectUserCredentials) {
    char buffer[] = "POST / HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=--dsaeCSawdsasw\r\nform-data; name=\"login\"\r\n\r\nadmin\r\nname=\"password\"\r\n\r\npasswd";
    
    char db_path[] = "test_database.db";

    ASSERT_EQ(http_login_routine(sockets[0], buffer, session_id_length, 10, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Set-Cookie: sessionId="), std::string::npos);
    EXPECT_NE(response.find("; Path=/; HttpOnly; SameSite=Lax; Max-Age=600"), std::string::npos);
}


TEST_F(HTTPLoginTest, IncorrectUserCredentials) {
    char buffer[] = "POST / HTTP/1.1\r\nContent-Type: multipart/form-data; boundary=--dsaeCSawdsasw\r\nform-data; name=\"login\"\r\n\r\nuser\r\nname=\"password\"\r\n\r\npasswd";
    
    char db_path[] = "test_database.db";

    ASSERT_EQ(http_login_routine(sockets[0], buffer, session_id_length, 10, db_path), 1);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 401 Unauthorized\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n\r\n"), std::string::npos);
}


TEST_F(HTTPLoginTest, NoLoginInfo) {
    char buffer[] = "POST / HTTP/1.1\r\nContent-Type: multipart/form-data; ";
    
    char db_path[] = "test_database.db";

    ASSERT_EQ(http_login_routine(sockets[0], buffer, session_id_length, 10, db_path), -2);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 400 Bad Request\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 0\r\n"), std::string::npos);
    EXPECT_NE(response.find("Connection: close\r\n\r\n"), std::string::npos);
}
