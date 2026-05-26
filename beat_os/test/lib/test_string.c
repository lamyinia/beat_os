#include "lib/string.h"
#include "test/test.h"
#include "test/test_string.h"

void run_string_tests(void) {
    char buffer[8];
    char copy[8];
    const char hello[] = "hello";
    const char hello_copy[] = "hello";
    const char world[] = "world";

    test_log_section("lib/string");

    memset(buffer, 'A', sizeof(buffer));
    test_expect_true(buffer[0] == 'A' && buffer[7] == 'A',
                     "memset fills the full buffer");

    memset(copy, 0, sizeof(copy));
    memcpy(copy, hello, sizeof(hello));
    test_expect_true(memcmp(copy, hello, sizeof(hello)) == 0,
                     "memcpy copies bytes");

    test_expect_true(memcmp(hello, hello_copy, sizeof(hello)) == 0,
                     "memcmp matches identical buffers");
    test_expect_true(memcmp(hello, world, sizeof(hello)) != 0,
                     "memcmp detects different buffers");
    test_expect_true(strlen("") == 0, "strlen handles empty strings");
    test_expect_true(strlen("beat_os") == 7, "strlen counts characters");
}
