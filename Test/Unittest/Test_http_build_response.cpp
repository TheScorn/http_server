#include "../../http_server_header.h"   
#include <gtest/gtest.h>
#include <cstring>

class loadFileTest : public testing::Test {
    protected:
};


class addHeaderTest : public testing::Test {
    protected:
};


TEST(loadFileTest, NullPointer) {
    char* file;
    int status = load_file(NULL, &file);
    EXPECT_EQ(-1, status);
}

TEST(loadFileTest, NoPath) {
    char st[] = "";
    char* file;
    int status = load_file(st, &file);
    EXPECT_EQ(-1, status);
}

TEST(loadFileTest, WrongPath) {
    char st[] = "this/path/is/wrong";
    char* file;
    int status = load_file(st , &file);
    EXPECT_EQ(-1, status);
}

TEST(loadFileTest, Correct) {
    char st[] = "./test_file.txt";
    char* file;
    int status = load_file(st, &file);
    ASSERT_EQ(0, status);
    EXPECT_STREQ(file, "zawartość pliku testowego");
    free(file);
}



TEST(addHeaderTest, NullRawMessageNullExt) {
    char* raw = NULL;
    char* ext = NULL;
    char* processed;
    add_response_header(raw, ext, &processed);

    ASSERT_TRUE(strlen(processed) > 0);
    EXPECT_EQ(strncasecmp(processed, "HTTP/1.1 200 OK\r\nDate: ", 23), 0);

    char* p;
    p = strstr(processed, "Content-Type: ");
    ASSERT_TRUE(p != NULL);
    p += 14;

    EXPECT_EQ(strncasecmp(p, "application/octet-stream", 24), 0);

    p = strstr(processed, "Content-Length: ");
    ASSERT_TRUE(p != NULL);
    p += 16;

    EXPECT_EQ(strncasecmp(p, "0", 1), 0);



    free(processed);
}


TEST(addHeaderTest, MessageExt) {
    char body[] = "This is body of a file";
    char ext[] = "wav";
    char ext_mime[] = "audio/wav";

    char* processed;
    add_response_header(body, ext, &processed);

    ASSERT_TRUE(strlen(processed) > 0);
    EXPECT_EQ(strncasecmp(processed, "HTTP/1.1 200 OK\r\nDate: ", 23), 0);

    char* p;
    p = strstr(processed, "Content-Type: ");
    ASSERT_TRUE(p != NULL);
    p += 14;

    EXPECT_EQ(strncasecmp(p, "audio/wav", 9), 0);

    p = strstr(processed, "Content-Length: ");
    ASSERT_TRUE(p != NULL);
    p += 16;

    EXPECT_EQ(strncasecmp(p, "22", 2), 0);

    p = strstr(processed, "\r\n\r\n");
    ASSERT_TRUE(p != NULL);
    p += 4;

    EXPECT_EQ(strcasecmp(p, body), 0);

    free(processed);
}

