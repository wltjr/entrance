inc = include_directories('.')

# entrance
entrance_deps = [crypt, deps, ecore_x, xcb]
entrance_incs = [inc, top_inc]
entrance_srcs = [
	'../../src/event/entrance_event.h',	'../../src/event/entrance_event.c',
	'entrance_config.h',	'entrance_config.c',
	'entrance_session.h',	'entrance_session.c',
	'entrance_xserver.h',	'entrance_xserver.c',
	'entrance_server.h',	'entrance_server.c',
	'entrance_history.h',	'entrance_history.c',
	'entrance_action.h',	'entrance_action.c',
	'entrance_image.h',	'entrance_image.c',
	'entrance_theme.h',	'entrance_theme.c',
	'entrance.h',		'entrance.c'
]

if has_pam
	entrance_deps += [pam_dep]
	entrance_incs += [pam_inc]
	entrance_srcs += ['entrance_pam.h', 'entrance_pam.c']
endif

if has_logind
	entrance_deps += [logind_dep]
	entrance_srcs += ['entrance_logind.h', 'entrance_logind.c']
endif

executable('entrance',
	sources			: entrance_srcs,
	include_directories	: entrance_incs,
	dependencies		: entrance_deps,
	win_subsystem			: 'windows',
	install			: true,
	install_dir             : dir_sbin
)
