inc = include_directories('.')
executable('entrance_client', [
		'../../src/event/entrance_event.h',	'../../src/event/entrance_event.c',
		'entrance_client.h',	'entrance_client.c',
		'entrance_connect.h',	'entrance_connect.c',
		'entrance_fill.h',	'entrance_fill.c',
		'entrance_login.h',	'entrance_login.c',
		'entrance_gui.h',	'entrance_gui.c',
		'entrance_system.h',	'entrance_system.c',
		'entrance_edje.h'
	],
	include_directories	: [inc, top_inc],
	dependencies		: [deps, ecore_x, ecore_file],
	gui_app			: true,
	install			: true,
	install_dir             : dir_lib
)
