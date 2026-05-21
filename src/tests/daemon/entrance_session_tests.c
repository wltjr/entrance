#include <pwd.h>

#include "entrance_tests.h"
#include "../../../src/daemon/entrance_session_utils.h"


START_TEST(entrance_session_test_shell_set_unset)
{
    static char name[] = "ubuntu";
    static char shell[] = {0};
    struct passwd pwd;

    PT("Test unset shell");

    pwd.pw_name = name;
    pwd.pw_gid = 1000;
    pwd.pw_uid = 1000;
    pwd.pw_shell = shell;
    entrance_session_shell_set(&pwd);
}
END_TEST

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

    // ensure user can write to coverage files, otherwise, whats the point?
    system("chown -R ubuntu:ubuntu /entrance/build/src/tests/daemon/entrance_tests.p/");

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

    tcase_add_test(tc, entrance_session_test_shell_set_unset);
    tcase_add_test(tc, entrance_session_test_userid_set_bad);
    tcase_add_test(tc, entrance_session_test_userid_set_good);
    suite_add_tcase(s, tc);

    return s;
}
