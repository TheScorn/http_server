#include "../../http_server_header.h"   
#include <gtest/gtest.h>
#include <cstring>

class GenerateTokenTest : public testing::Test {
    protected:

};


class getFileExtensionTest : public testing::Test {
    protected:
};


class getUsernameTest : public testing::Test {
    protected:
};


class getPasswordTest : public testing::Test {
    protected:
};


class urlDecodeTest : public testing::Test {
    protected:
};


class getMimeType : public testing::Test {
    protected:
};



TEST(GenerateTokenTest, CreatesCorrectLength1) {
    int session_id_len = 1;
    char session_id[session_id_len + 1];
    generate_session_id(session_id_len, session_id);
    EXPECT_EQ(strlen(session_id), session_id_len);
}

TEST(GenerateTokenTest, CreatesCorrectLength12) {
    int session_id_len = 12;
    char session_id[session_id_len + 1];
    generate_session_id(session_id_len, session_id);
    EXPECT_EQ(strlen(session_id), session_id_len);
}

TEST(GenerateTokenTest, CreatesCorrectLength24) {
    int session_id_len = 24;
    char session_id[session_id_len + 1];
    generate_session_id(session_id_len, session_id);
    EXPECT_EQ(strlen(session_id), session_id_len);
}

TEST(getFileExtensionTest, EmptyString) {
    char empty[] = "";
    const char* ext = get_file_extension(empty);
    EXPECT_STREQ(ext, NULL);
}

TEST(getFileExtensionTest, StringWithoutDot) {
    char nodot[] = "no dot";
    const char* ext = get_file_extension(nodot);
    EXPECT_STREQ(ext, NULL);
}

TEST(getFileExtensionTest, StringWithDot) {
    char dot[] = "nodot.exe";
    const char* ext = get_file_extension(dot);
    EXPECT_STREQ(ext, "exe");

    char dot2[] = ".ext";
    const char* ext2 = get_file_extension(dot2);
    EXPECT_STREQ(ext2, NULL);

    char dot3[] = "ext.";
    const char* ext3 = get_file_extension(dot3);
    EXPECT_STREQ(ext3, NULL);

}

TEST(getFileExtensionTest, MultipleDots) {
    char dot[] = "no.dot.exe";
    const char* ext = get_file_extension(dot);
    EXPECT_STREQ(ext, "exe");

    char dot2[] = ".no.ext";
    const char* ext2 = get_file_extension(dot2);
    EXPECT_STREQ(ext2, "ext");

    char dot3[] = "ext.dot.dot.dot";
    const char* ext3 = get_file_extension(dot3);
    EXPECT_STREQ(ext3, "dot");

}


TEST(getUsernameTest, EmptyString) {
    char st[] = "";
    char* ch = get_username(st);
    EXPECT_STREQ(ch, NULL);
}

TEST(getPasswordTest, EmptyString) {
    char st[] = "";
    char* ch = get_password(st);
    EXPECT_STREQ(ch, NULL);
}

TEST(getUsernameTest, NullPointer) {
    char* st = NULL;
    char* ch = get_username(st);
    EXPECT_STREQ(ch, NULL);
}

TEST(getPasswordTest, NullPointer) {
    char* st = NULL;
    char* ch = get_password(st);
    EXPECT_STREQ(ch, NULL);
}

TEST(getUsernameTest, StrOK) {
    char st[] = "username:password";
    char* ch = get_username(st);
    EXPECT_STREQ(ch, "username");
    free(ch);
}

TEST(getPasswordTest, StrOK) {
    char st[] = "username:password";
    char* ch = get_password(st);
    EXPECT_STREQ(ch, "password");
}


TEST(urlDecodeTest, NullPointer) {
    char* st = NULL;
    char* ch = url_decode(st);
    EXPECT_STREQ(ch, NULL);
}

TEST(urlDecodeTest, EmptyString) {
    char st[] = "";
    char* ch = url_decode(st);
    EXPECT_STREQ(ch, "");
    free(ch);
}



TEST(urlDecodeTest, StrOK) {
    char st[] = "%21";
    char* ch = url_decode(st);
    EXPECT_STREQ(ch, "!");
    free(ch);

    char st2[] = "hello%20world%21";
    char* ch2 = url_decode(st2);
    EXPECT_STREQ(ch2, "hello world!");
    free(ch2);
}


TEST(getMimeType, NullPointer) {
    char* st = NULL;
    char* ch = (char*)get_mime_type(st);
    EXPECT_STREQ(ch, "application/octet-stream");
}

TEST(getMimeType, EmptyStr) {
    char st[] = "";
    char* ch = (char*)get_mime_type(st);
    EXPECT_STREQ(ch, "application/octet-stream");
}

TEST(getMimeType, Text) {
    char st[] = "txt";
    char* ch = (char*)get_mime_type(st);
    EXPECT_STREQ(ch, "text/plain");
}
