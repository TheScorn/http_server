extern "C" {
#include "../../http_server_header.h"   
#include "../../sqlite3.h"
}

#include <gtest/gtest.h>
#include <cstring>
#include <regex.h>
#include <unistd.h>
#include <sys/socket.h>

class HTTPGetTest : public testing::Test {
protected:
    int sockets[2];

    int session_id_length = 24;
    char* session_id_weak = (char*)malloc(session_id_length + 1);
    char* session_id_strong = (char*)malloc(session_id_length + 1);

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
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Files;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Zones;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);


        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Sessions (id_session_id INTEGER PRIMARY KEY, session_id TEXT NOT NULL, user_id INTEGER NOT NULL, expiry INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Users (user_id INTEGER PRIMARY KEY, username TEXT NOT NULL, password TEXT NOT NULL, email TEXT, elevated INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt);       
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Files (file_id INTEGER PRIMARY KEY, file_name TEXT NOT NULL, zone_id TEXT NOT NULL, file_path TEXT NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Zones (zone_id INTEGER PRIMARY KEY, zone_name TEXT NOT NULL, zone_type INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt);




        generate_session_id(session_id_length, session_id_weak);
        
        generate_session_id(session_id_length, session_id_strong);
        //session_id_strong[2] = 'F';

        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Users (username, password, elevated) VALUES(\'admin\', \'passwd\', 1), (\'thescorn\', \'3084\', 0);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Zones (zone_name, zone_type) VALUES(\'Main\', 0), (\'Status\', 1), (\'NAS\', 2);", -1, &stmt, NULL),0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Files (file_name, zone_id, file_path) VALUES(\'test_main.txt\', 1, \'test_main.txt\'), (\'test_status.txt\', 2, \'test_status.txt\'), (\'test_nas.txt\', 3, \'test_nas.txt\');", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt); 
        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Sessions (session_id, user_id, expiry) VALUES(?, 2, ?), (?, 1, ?);", -1, &stmt, NULL),0);
        sqlite3_bind_text(stmt, 1, session_id_weak, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, time(NULL) + 4000);
        sqlite3_bind_text(stmt, 3, session_id_strong, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, time(NULL) + 4000);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 

        sqlite3_finalize(stmt);

        

        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);

        ASSERT_EQ(sqlite3_prepare_v2(db, "SELECT Users.username, Sessions.expiry, Users.elevated FROM Sessions INNER JOIN Users ON Sessions.user_id = Users.user_id WHERE session_id=?;", -1, &stmt, NULL), 0);
        sqlite3_bind_text(stmt, 1, session_id_weak, -1, SQLITE_TRANSIENT);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
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
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Users;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Files;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Zones;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        free(session_id_weak);
        free(session_id_strong);
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


TEST_F(HTTPGetTest, NoCredentialsMain) {
    char buffer[] = "GET /test_main.txt HTTP/1.1\r\n";
    
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    regexec(&regex, buffer, 2, matches, 0);


    char db_path[] = "test_database.db";

    ASSERT_EQ(http_get_routine(sockets[0], buffer, matches, 4096, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 18\r\n"), std::string::npos);
    EXPECT_NE(response.find("Main file contents"), std::string::npos);
}

TEST_F(HTTPGetTest, NoCredentialsNAS) {
    char buffer[] = "GET /test_nas.txt HTTP/1.1\r\n";
    
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    regexec(&regex, buffer, 2, matches, 0);

    char db_path[] = "test_database.db";

    ASSERT_EQ(http_get_routine(sockets[0], buffer, matches, 4096, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 302 Found\r\n"), std::string::npos);
    EXPECT_NE(response.find("Location: /login_page.html"), std::string::npos);
}

TEST_F(HTTPGetTest, WeakCredentialsStatus) {

    char buffer[300];
    char buffer1[] = "GET /test_status.txt HTTP/1.1\r\nCookie: sessionId=";
    
    snprintf(buffer, 300, "%s%s\r\n", buffer1, session_id_weak);


    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    regexec(&regex, buffer, 2, matches, 0);

    char db_path[] = "test_database.db";

    ASSERT_EQ(http_get_routine(sockets[0], buffer, matches, 4096, db_path), 4);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 403 Forbidden\r\n"), std::string::npos);
    
}

TEST_F(HTTPGetTest, StrongCredentialsStatus) {


    char buffer[300];
    char buffer1[] = "GET /test_status.txt HTTP/1.1\r\nCookie: sessionId=";
    
    snprintf(buffer, 300, "%s%s\r\n", buffer1, session_id_strong);


    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];
    regexec(&regex, buffer, 2, matches, 0);

    char db_path[] = "test_database.db";

    ASSERT_EQ(http_get_routine(sockets[0], buffer, matches, 4096, db_path), 0);

    std::string response = recieve_response();
    EXPECT_NE(response.find("HTTP/1.1 200 OK\r\n"), std::string::npos);
    EXPECT_NE(response.find("Content-Length: 21\r\n"), std::string::npos);
    EXPECT_NE(response.find("Status file contents."), std::string::npos);
    
}