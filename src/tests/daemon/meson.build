inc = include_directories('.')

test_deps = [check_dep, deps, ecore_x]
test_incs = [inc, top_inc]
test_sources = ['entrance_tests.h', 'entrance_tests.c']

if has_pam
	test_deps += [pam_dep]
	test_incs += [pam_inc]
    test_sources += [
        '../../../src/daemon/entrance_pam.h',
        '../../../src/daemon/entrance_pam.c',
        'entrance_pam_tests.c'
    ]
endif

entrance_tests = executable('entrance_tests', 
    sources             : test_sources,
    include_directories : test_incs,
    dependencies        : test_deps,
    win_subsystem       : 'console',
    install             : false
)

test('unit tests', entrance_tests)