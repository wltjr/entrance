#include "entrance_tests.h"

int main(void)
{
    int failed;
    SRunner *runner;

    eina_init();
    eina_log_color_disable_set(EINA_FALSE); // force color logging
    eina_log_threads_enable();
    _entrance_log = eina_log_domain_register("entrance", EINA_COLOR_CYAN);
    eina_log_domain_level_set("entrance", 5);

    runner = srunner_create(pam_suite());
    srunner_run_all(runner, CK_NORMAL);
    failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
