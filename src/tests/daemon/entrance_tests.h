#ifndef ENTRANCE_TESTS_H_
#define ENTRANCE_TESTS_H_

#include <check.h>

#include "../../../src/daemon/entrance.h"

/**
 * @brief Entrance Pam functions test suite
 *
 * @return Suite* suite containing test functions
 */
Suite *pam_suite(void);

#endif /* ENTRANCE_TESTS_H_ */
