#include <pwd.h>

#include "entrance_tests.h"
#include "../../../src/daemon/entrance_session_utils.h"


START_TEST(entrance_session_test_userid_set_bad)
{
    static char name[] = "notauser";
    struct passwd pwd;
    int result = 0;

    PT("Test set user id invalid");

    pwd.pw_name = name;
    pwd.pw_gid = -1;
    pwd.pw_uid = 11;
    result = entrance_session_userid_set(&pwd);
    ck_assert_int_eq(result, 1);
    pwd.pw_gid = 0;
    pwd.pw_uid = -1;
    result = entrance_session_userid_set(&pwd);
    ck_assert_int_eq(result, 1);
}
END_TEST

START_TEST(entrance_session_test_userid_set_good)
{
    static char buf[4096];
    static struct passwd pwd_buf;
    struct passwd *pwd = NULL;
    int result = 0;

    PT("Test set user id valid");

    result = getpwnam_r("ubuntu", &pwd_buf, buf, sizeof(buf), &pwd);
    ck_assert_int_eq(result, 0);
    result = entrance_session_userid_set(pwd);
    ck_assert_int_eq(result, 0);
}
END_TEST

Suite *session_suite(void)
{
    Suite *s;
    TCase *tc;

    s = suite_create("Entrance Session Tests");
    tc = tcase_create("session");

    tcase_add_test(tc, entrance_session_test_userid_set_bad);
    tcase_add_test(tc, entrance_session_test_userid_set_good);
    suite_add_tcase(s, tc);

    return s;
}
