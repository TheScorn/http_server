#include "../../http_server_header.h"   
#include <gtest/gtest.h>
#include <cstring>

class base64Test : public testing::Test {
    protected:
};



TEST(base64Test, NullPointer) {
    const unsigned char* st = NULL;
    size_t out_len;
    char* ch = (char*)base64_decode(st, 0, &out_len);
    EXPECT_STREQ(ch, NULL);
}

TEST(base64Test, EmptyStr) {
    const unsigned char st[] = "";
    size_t out_len;
    char* ch = (char*)base64_decode(st, 0, &out_len);
    EXPECT_STREQ(ch, NULL);
}

TEST(base64Test, Message) {
    const unsigned char message[] = "TWVzc2FnZSBlbmNvZGVk";
    size_t out_len;
    char* ch = (char*)base64_decode(message, 20, &out_len);
    EXPECT_STREQ(ch, "Message encoded");
    free(ch);
}
