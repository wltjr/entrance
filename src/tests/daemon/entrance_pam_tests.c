#include "entrance_tests.h"
#include "../../../src/daemon/entrance_pam.h"

static const char *_user_good = "ubuntu";
static const char *_user_bad = "stranger";
static const char *_passwd_good = "ubuntu";
static const char *_passwd_bad = "badpw";

static const char *_service = "entrance";
static const char *_tty = "vt7";

int _entrance_log;

static void
_entrance_pam_cleanup(int id)
{
    entrance_pam_session_close(id);
    entrance_pam_end(id);
    entrance_pam_shutdown(id);
    entrance_pams_shutdown();
}

START_TEST(entrance_pam_test_user_bad)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test unknown user");
    entrance_pams_init(count);

    /* test unknown user */
    result = entrance_pam_start(id, _service, _tty, _user_bad);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_session_open(id);
    ck_assert_int_eq(result, 0);

    /* test auth fail */
    result = entrance_pam_passwd_set(id, _passwd_bad);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_authenticate(id);
    ck_assert_int_eq(result, 1);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_user_good)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test known user");
    entrance_pams_init(count);

    /* test known user */
    result = entrance_pam_start(id, _service, _tty, _user_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_session_open(id);
    ck_assert_int_eq(result, 0);

    /* test known auth success */
    result = entrance_pam_passwd_set(id, _passwd_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_authenticate(id);
    ck_assert_int_eq(result, 0);

    /* needless call just for code coverage */
    result = entrance_pam_last_result_get(id);
    ck_assert_int_eq(result, 0);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_user_nobody)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test valid user");
    entrance_pams_init(count);

    /* test valid user */
    result = entrance_pam_start(id, _service, _tty, "nobody");
    ck_assert_int_eq(result, 0);
    result = entrance_pam_session_open(id);
    ck_assert_int_eq(result, 0);

    /* test auth error */
    result = entrance_pam_passwd_set(id, _passwd_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_authenticate(id);
    ck_assert_int_eq(result, 1);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_env_set_error)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test set env error");
    entrance_pams_init(count);

    /* test set item silent error */
    result = entrance_pam_start(id, _service, _tty, _user_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_end(id);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_env_set(id, "TESTVAR", "value");
    ck_assert_int_eq(result, 1);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_item_get_error)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test get item error");
    entrance_pams_init(count);

    /* test get item error nothing set */
    result = entrance_pam_start(id, _service, _tty, _user_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_end(id);
    ck_assert_int_eq(result, 0);
    const void *item = entrance_pam_item_get(id, ENTRANCE_PAM_ITEM_USER);
    ck_assert_ptr_null(item);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_item_set_error)
{
    int id = 0;
    int count = 1;
    int result = 0;

    PT("Test set item error");
    entrance_pams_init(count);

    /* test set item silent error */
    result = entrance_pam_start(id, _service, _tty, _user_good);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_end(id);
    ck_assert_int_eq(result, 0);
    result = entrance_pam_item_set(id, ENTRANCE_PAM_ITEM_RUSER, _user_good);
    ck_assert_int_eq(result, 1);

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_result_check)
{
    int result = 0;

    PT("Test result check");

    /* pathetic tests, but getting pam to return such codes is non-trivial */
    result = entrance_pam_result_check("test",PAM_BUF_ERR);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_ABORT);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_PERM_DENIED);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_CRED_INSUFFICIENT);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_AUTHINFO_UNAVAIL);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_USER_UNKNOWN);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_ACCT_EXPIRED);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_CRED_ERR);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_BAD_ITEM);
    ck_assert_int_eq(result, 1);
    result = entrance_pam_result_check("test",PAM_INCOMPLETE); // not handled
    ck_assert_int_eq(result, 1);
}
END_TEST

Suite *pam_suite(void)
{
    Suite *s;
    TCase *tc_pam;

    s = suite_create("Entrance Pam Tests");
    tc_pam = tcase_create("pam");

    tcase_add_test(tc_pam, entrance_pam_test_user_bad);
    tcase_add_test(tc_pam, entrance_pam_test_user_good);
    tcase_add_test(tc_pam, entrance_pam_test_user_nobody);
    tcase_add_test(tc_pam, entrance_pam_test_env_set_error);
    tcase_add_test(tc_pam, entrance_pam_test_item_get_error);
    tcase_add_test(tc_pam, entrance_pam_test_item_set_error);
    tcase_add_test(tc_pam, entrance_pam_test_result_check);
    suite_add_tcase(s, tc_pam);

    return s;
}
