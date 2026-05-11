#include <check.h>

#include "../../../src/daemon/entrance.h"
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
}

START_TEST(entrance_pam_test_user_bad)
{
    int id = 0;
    int result = 0;

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
    int result = 0;

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

    _entrance_pam_cleanup(id);
}
END_TEST

START_TEST(entrance_pam_test_user_nobody)
{
    int id = 0;
    int result = 0;

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

Suite *pam_suite(void)
{
    Suite *s;
    TCase *tc_pam;

    s = suite_create("Entrance Pam Tests");
    tc_pam = tcase_create("pam");

    tcase_add_test(tc_pam, entrance_pam_test_user_bad);
    tcase_add_test(tc_pam, entrance_pam_test_user_good);
    tcase_add_test(tc_pam, entrance_pam_test_user_nobody);
    suite_add_tcase(s, tc_pam);

    return s;
}

int main(void)
{
    int count = 1;
    int failed;
    Suite *s = pam_suite();
    SRunner *runner = srunner_create(s);

    eina_init();
    eina_log_color_disable_set(EINA_FALSE); // force color logging
    eina_log_threads_enable();
    _entrance_log = eina_log_domain_register("entrance", EINA_COLOR_CYAN);
    eina_log_domain_level_set("entrance", 5);

    entrance_pams_init(count);

    srunner_run_all(runner, CK_NORMAL);
    failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    entrance_pams_shutdown();

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
