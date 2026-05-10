inc = include_directories('.')

test_deps = [check_dep, deps, ecore_x]
test_incs = [inc, top_inc]

if has_pam
	test_deps += [pam_dep]
	test_incs += [pam_inc]

    entrance_pam_tests = executable('entrance_pam_tests', 
        sources             : [
            '../../../src/daemon/entrance_pam.h',
            '../../../src/daemon/entrance_pam.c',
            'entrance_pam_tests.c' ],
        include_directories : test_incs,
        dependencies        : test_deps,
        win_subsystem       : 'console',
        install             : false
    )

    test('unit tests', entrance_pam_tests)
endif
