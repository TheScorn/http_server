extern "C" {
#include "../../http_server_header.h"   
}

#include <gtest/gtest.h>
#include <cstring>

#include <unistd.h>
#include <sys/socket.h>
#include <sqlite3.h>

class HTTPLogoutTest : public testing::Test {
protected:
    int sockets[2];

    int session_id_length = 24;

    void SetUp() override {
        //tworzymy tabele Sessions
        sqlite3* db;
        sqlite3_open("test_database.db", &db);
        sqlite3_stmt* stmt;

        sqlite3_prepare_v2(db, "CREATE TABLE Sessions (id_session_id INTEGER PRIMARY KEY, session_id TEXT NOT NULL, user_id INTEGER NOT NULL, expiry INTEGER NOT NULL)", -1, &stmt, NULL);

        

        //trzeba stworzyć session_id
        char session_id[session_id_length + 1];
        generate_session_id(session_id_length, session_id);





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






};


