#ifndef TSH_TEST_H
#define TSH_TEST_H

#define TEST_DIR "/tmp/tsh_test"
#define NB_TESTS 10

#define WHITE "\e[m"
#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
extern int tests_run;
void before(void);

#endif
