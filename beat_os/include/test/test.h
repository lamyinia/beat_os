#ifndef BEAT_OS_TEST_H
#define BEAT_OS_TEST_H

int run_tests(void);
void test_expect_true(int condition, const char *name);
void test_log_section(const char *name);

#endif
