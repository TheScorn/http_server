extern "C" {
#include "../../http_server_header.h"   
#include "../../sqlite3.h"
#include "../../NAS_API.h"
}

#include <gtest/gtest.h>
#include <cstring>
#include <regex.h>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <thread>


struct step_struct {
    enum {
        EXPECT, SEND
    } kind;
    std::string data;
};

class FakePeer {
    int fd;
    std::vector<step_struct> script;
    std::thread th;
    std::string error;

public:

    FakePeer(int fd, std::vector<step_struct> s) : fd(fd), script(std::move(s)) {}
    
    void start() {
        th = std::thread([this]{ run(); });
    }

    void join() {
        if(th.joinable()) {
            th.join();
        }
    }

    const std::string& err() const { return error; }

private:
    void run() {
        timeval tv{2, 0};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
        for(auto& st : script) {
            if(st.kind == step_struct::SEND) {
                if(send(fd, st.data.data(), st.data.size(), MSG_NOSIGNAL) < 0) {
                    error = "send failed";
                    return;
                }
            }
            else {
                std::string got(st.data.size(), '\0');
                size_t off = 0;
                while(off < got.size()) {
                    ssize_t r = recv(fd, &got[off], got.size() - off, 0);
                    if(r <= 0) {
                        error = "recv failed";
                        return;
                    }
                    off += r;
                }
                if(got != st.data) {
                    error = "expected [" + st.data + "] got [" + got + "]";
                    return;
                }
            }
        }
    }



};

class NASHelperTest : public testing::Test {
protected:
};

class ListToJsonTest : public testing::Test {
protected:
};

class HTTPNASTest : public testing::Test {
protected:
    int sockets[2];

    int NAS_sockets[2];

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
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS NAS_Users;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        

        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Sessions (id_session_id INTEGER PRIMARY KEY, session_id TEXT NOT NULL, user_id INTEGER NOT NULL, expiry INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE Users (user_id INTEGER PRIMARY KEY, username TEXT NOT NULL, password TEXT NOT NULL, email TEXT, elevated INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt); 
        ASSERT_EQ(sqlite3_prepare_v2(db, "CREATE TABLE NAS_Users (NAS_user_id INTEGER PRIMARY KEY, NAS_username TEXT NOT NULL, NAS_password TEXT NOT NULL, user_id INTEGER NOT NULL);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 
        sqlite3_finalize(stmt); 

        generate_session_id(session_id_length, session_id_weak);
        
        generate_session_id(session_id_length, session_id_strong);

        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Users (username, password, elevated) VALUES(\'admin\', \'passwd\', 1), (\'thescorn\', \'3084\', 0);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO NAS_Users (NAS_username, NAS_password, user_id) VALUES(\'admin\', \'passwd\', 1), (\'thescorn\', \'3084\', 2);", -1, &stmt, NULL), 0);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE);
        sqlite3_finalize(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "INSERT INTO Sessions (session_id, user_id, expiry) VALUES(?, 2, ?), (?, 1, ?);", -1, &stmt, NULL),0);
        sqlite3_bind_text(stmt, 1, session_id_weak, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, time(NULL) + 4000);
        sqlite3_bind_text(stmt, 3, session_id_strong, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 4, time(NULL) + 4000);
        ASSERT_EQ(sqlite3_step(stmt), SQLITE_DONE); 


        sqlite3_finalize(stmt);
        sqlite3_close(db);

        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, NAS_sockets), 0);
    }

    void TearDown() override {
        if(sockets[0] != -1) {
            close(sockets[0]);
        }
        if(sockets[1] != -1) {
            close(sockets[1]);
        }
        if(NAS_sockets[0] != -1) {
            close(sockets[0]);
        }
        if(NAS_sockets[1] != -1) {
            close(sockets[1]);
        }

        sqlite3* db;
        sqlite3_open("test_database.db", &db);

        sqlite3_stmt* stmt;
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Sessions;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS Users;", -1, &stmt, NULL), 0);
        sqlite3_step(stmt);
        ASSERT_EQ(sqlite3_prepare_v2(db, "DROP TABLE IF EXISTS NAS_Users;", -1, &stmt, NULL), 0);
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


TEST(NASHelper, ConvertTest) {
    char prefix[] = "000000000000000F";
    unsigned long long prefix_num;
    convert(&prefix_num, prefix);
    ASSERT_EQ(prefix_num, 15);
}

TEST(ListToJsonTest, TestSuccess) {
    char* list_json;
    char list[] = "0000000000000013F000000000000003D0008plik.txt000000000000000F3000000000000003A0009plik2.txt";

    char expected[] = "[{\"name\":\"plik.txt\",\"type\":0,\"size\":319,\"mtime\":61},{\"name\":\"plik2.txt\",\"type\":0,\"size\":243,\"mtime\":58}]";
    LIST_to_json(list, &list_json);
    ASSERT_STREQ(list_json, expected);
}




TEST_F(HTTPNASTest, TestRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000004TEST"},
        {step_struct:: SEND, "0000000000000003ACK"}
    });

    NAS.start();

    char status = TEST_routine(NAS_sockets[0]);

    NAS.join();
    ASSERT_EQ(status, 0);
}

TEST_F(HTTPNASTest, TestRoutineFail) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000004TEST"},
        {step_struct:: SEND, "0000000000000003FCK"}
    });

    NAS.start();

    char status = TEST_routine(NAS_sockets[0]);

    NAS.join();
    ASSERT_EQ(status, 1);
}

TEST_F(HTTPNASTest, LogintestSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000026LOGINTEST login:thescorn password:3084"}, //38
        {step_struct:: SEND, "0000000000000008LOGINACK"}
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "3084";
    char status = LOGINTEST_routine(NAS_sockets[0], login, password);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 0);
}

TEST_F(HTTPNASTest, LogintestFalse) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000026LOGINTEST login:thescorn password:1111"},
        {step_struct:: SEND, "000000000000000ALOGINFALSE"}
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    int status = LOGINTEST_routine(NAS_sockets[0], login, password);

    NAS.join();
    ASSERT_EQ(status, 1);
}

TEST_F(HTTPNASTest, LISTRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FLIST /path/to/dir/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "000000000000002D00000000000000009000000000000000F0008plik.txt"} //45
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/dir/";
    char* list_buffer;
    int status = LIST_routine(NAS_sockets[0], path, login, password, &list_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 0);
    EXPECT_STREQ(list_buffer, "00000000000000009000000000000000F0008plik.txt"); //45 = 32 + 13 = 2*16 + D
    free(list_buffer);

}

TEST_F(HTTPNASTest, LISTRoutineErrorFromNAS) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FLIST /path/to/dir/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000020ERROR Some Error from NAS server"} //32
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/dir/";
    char* list_buffer;
    int status = LIST_routine(NAS_sockets[0], path, login, password, &list_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 1);
    EXPECT_STREQ(list_buffer, "ERROR Some Error from NAS server"); //45 = 32 + 13 = 2*16 + D
    free(list_buffer);

}

TEST_F(HTTPNASTest, LISTRoutineFatalError) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FLIST /path/to/dir/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000J00020ERROR Some Error from NAS server"} //32
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/dir/";
    char* list_buffer;
    int status = LIST_routine(NAS_sockets[0], path, login, password, &list_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, -4);
}

TEST_F(HTTPNASTest, GETRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002EGET /path/to/file login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000021000000000000000FF0000000000000009"}, //33
        {step_struct:: EXPECT, "0000000000000006ACCEPT"},
        {step_struct:: SEND, "0000000000000009"}
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file";
    char* response;
    unsigned long long filesize;
    unsigned long long mtime;

    int status = pre_GET_routine_acc(NAS_sockets[0], path, login, password, &response, &mtime, &filesize);
    
    NAS.join();
    ASSERT_EQ(status, 0);
    EXPECT_EQ(filesize, 9);
    EXPECT_EQ(mtime, 255);
}

TEST_F(HTTPNASTest, GETRoutineErrorFromNAS) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002EGET /path/to/file login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000016ERROR Unknown request!"}, //22 16 + 6
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file";
    char* response;
    unsigned long long filesize;
    unsigned long long mtime;

    int status = pre_GET_routine_acc(NAS_sockets[0], path, login, password, &response, &mtime, &filesize);
    
    NAS.join();
    ASSERT_EQ(status, 1);
    EXPECT_STREQ(response, "ERROR Unknown request!");
    free(response);
}

TEST_F(HTTPNASTest, GETRoutineFatalError) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002EGET /path/to/file login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000V00000016ERROR Unknown request!"}, //22 16 + 6
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file";
    char* response;
    unsigned long long filesize;
    unsigned long long mtime;

    int status = pre_GET_routine_acc(NAS_sockets[0], path, login, password, &response, &mtime, &filesize);
    
    NAS.join();
    ASSERT_EQ(status, -4);
}


TEST_F(HTTPNASTest, DELRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000030DEL /path/to/file 1 login:thescorn password:1111"}, // 48 
        {step_struct:: SEND, "0000000000000003ACK"} //45
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file";
    char* response_buffer;
    int status = DEL_routine(NAS_sockets[0], path, login, password, &response_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 0);
}

TEST_F(HTTPNASTest, DELRoutineErrorFromNAS) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "0000000000000030DEL /path/to/file 1 login:thescorn password:1111"}, // 48 
        {step_struct:: SEND, "000000000000001FERROR No such file or directory"} //31
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file";
    char* response_buffer;
    int status = DEL_routine(NAS_sockets[0], path, login, password, &response_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 1);
    EXPECT_STREQ(response_buffer, "ERROR No such file or directory");
    free(response_buffer);
}

TEST_F(HTTPNASTest, MKDIRRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FMKDIR /path/to/dir login:thescorn password:1111"}, // 47
        {step_struct:: SEND, "0000000000000003ACK"} //45
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/dir";
    char* response_buffer;
    int status = MKDIR_routine(NAS_sockets[0], path, login, password, &response_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 0);

}


TEST_F(HTTPNASTest, MKDIRRoutineErrorFromNAS) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FMKDIR /path/to/dir login:thescorn password:1111"}, // 47
        {step_struct:: SEND, "0000000000000026ERROR no access rights to dir /path/to"} // 39 = 32 + 7
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/dir";
    char* response_buffer;
    int status = MKDIR_routine(NAS_sockets[0], path, login, password, &response_buffer);

    NAS.join();
    EXPECT_EQ(NAS.err(), "");
    ASSERT_EQ(status, 1);
    EXPECT_STREQ(response_buffer, "ERROR no access rights to dir /path/to");
    free(response_buffer);
}




TEST_F(HTTPNASTest, PUTRoutineSuccess) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FPUT /path/to/file/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000003ACK"}, //33
        {step_struct:: EXPECT, "00000000000000290000000000000000F000000000000000Fplik.txt"}, //41 = 32 + 9
        {step_struct:: SEND, "0000000000000006ACCEPT"}
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file/";
    char file[] = "plik.txt";
    char* response;
    unsigned long long filesize = 15;
    unsigned long long mtime = 15;

    int status = pre_PUT_routine(NAS_sockets[0], path, file, login, password, &filesize, &mtime, &response);
    
    NAS.join();
    ASSERT_EQ(status, 0);
}

TEST_F(HTTPNASTest, PUTRoutineErrorFromNAS) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FPUT /path/to/file/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000018ERROR no such directory!"}, //24
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file/";
    char file[] = "plik.txt";
    char* response;
    unsigned long long filesize = 15;
    unsigned long long mtime = 15;

    int status = pre_PUT_routine(NAS_sockets[0], path, file, login, password, &filesize, &mtime, &response);
    
    NAS.join();
    ASSERT_EQ(status, 1);
    EXPECT_STREQ("ERROR no such directory!", response);
    free(response);
}


TEST_F(HTTPNASTest, PUTRoutineREFUSE) {
    FakePeer NAS(NAS_sockets[1], {
        {step_struct:: EXPECT, "000000000000002FPUT /path/to/file/ login:thescorn password:1111"}, //47 32 + 15
        {step_struct:: SEND, "0000000000000003ACK"}, //33
        {step_struct:: EXPECT, "00000000000000290000000000000000F000000000000000Fplik.txt"}, //41 = 32 + 9
        {step_struct:: SEND, "0000000000000006REFUSE"}
    });

    NAS.start();

    char login[] = "thescorn";
    char password[] = "1111";
    char path[] = "/path/to/file/";
    char file[] = "plik.txt";
    char* response;
    unsigned long long filesize = 15;
    unsigned long long mtime = 15;

    int status = pre_PUT_routine(NAS_sockets[0], path, file, login, password, &filesize, &mtime, &response);
    
    NAS.join();
    ASSERT_EQ(status, -9);
}

