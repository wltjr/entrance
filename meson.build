# project
project('entrance', 'c',
        version        : '3.0.0_alpha6',
        license        : 'GPL-3.0',
        default_options: [ 'c_std=gnu99' ],
        meson_version  : '>= 0.40.0')
base_url = 'https://github.com/wltjr/'

# global arguments
add_global_arguments('-W', language : 'c')

# global variables
cc       = meson.get_compiler('c')
proj     = meson.project_name()
ver      = meson.project_version()
chmod    = join_paths(meson.source_root(), 'meson-chmod.sh')
top_inc  = include_directories('.')

# dependencies
efl_version = '>= 1.19.0'
crypt   = cc.find_library('crypt')
deps	= dependency('elementary', version: efl_version)
ecore_x	= dependency('ecore-x',	   version: efl_version)
edje	= dependency('edje',       version: efl_version)
xcb	= dependency('xcb')

has_pam = get_option('pam')
has_nls = get_option('nls')
has_logind = get_option('logind')

if has_pam
        pam_dep = cc.find_library('pam')
        pam_inc  = include_directories('/usr/include/security')
endif

##### edje_cc binary compiler tool
edje_cmd = get_option('edje-cc')
if edje_cmd == ''
  edje_cmd = join_paths(edje.get_pkgconfig_variable('prefix'),
                        'bin', 'edje_cc')
endif

##### dir locations
dir_prefix = get_option('prefix')
dir_cache  = join_paths(dir_prefix, get_option('cachedir'), proj)
dir_lib    = join_paths(dir_prefix, get_option('libdir'), proj)
dir_data   = join_paths(dir_prefix, get_option('datadir'))
dir_sbin   = join_paths(dir_prefix, get_option('sbindir'))
dir_syscfg = join_paths(dir_prefix, get_option('sysconfdir'))

# project args
add_project_arguments('-DHAVE_CONFIG_H=1', language : 'c')
if get_option('buildtype').startswith('debug')
	add_project_arguments('-g',
                              '-fprofile-arcs',
                              '-ftest-coverage',
                              language : 'c')
	add_project_link_arguments('-fprofile-arcs',
                              '-ftest-coverage',
                              language : 'c')
endif

# config.h
cfg = configuration_data()
cfg.set_quoted('PACKAGE'		, proj)
cfg.set_quoted('PACKAGE_CACHE'		, dir_cache)
cfg.set_quoted('PACKAGE_STRING'		, proj + ' ' + ver)
cfg.set_quoted('PACKAGE_BIN_DIR'	, dir_lib)
cfg.set_quoted('PACKAGE_DATA_DIR'	, join_paths(dir_data, proj))
cfg.set_quoted('PACKAGE_LIB_DIR'	, dir_lib)
cfg.set_quoted('PACKAGE_SBIN_DIR'	, dir_sbin)
cfg.set_quoted('SYSTEM_CONFIG_DIR'	, dir_syscfg)
cfg.set_quoted('VERSION'		, ver)

cfg.set('SYSTEM_CONFIG_DIR2'		, dir_syscfg)
cfg.set('ENABLE_NLS', has_nls)
cfg.set('HAVE_ALLOCA_H', cc.has_header('alloca.h'))
cfg.set('HAVE_LOGIND', has_logind)
cfg.set('HAVE_PAM', has_pam)

configure_file(output: 'config.h', configuration: cfg)
configure_file(input  : 'data/entrance.conf.in',
               output : 'entrance.conf',
               install_dir : join_paths(dir_syscfg, proj),
               configuration : cfg
)
# Only install systemd service file on systemd-based systems
# OpenRC/Gentoo users should use their own init scripts
systemd = dependency('systemd', required: false)
if has_logind and systemd.found()
	systemd_system_unit_dir = systemd.get_pkgconfig_variable('systemdsystemunitdir')
	configure_file(input  : 'data/entrance.service.in',
                       output : 'entrance.service',
                       install_dir : systemd_system_unit_dir,
                       configuration : cfg
	)
endif
if has_pam
	configure_file(input  : 'data/entrance.pam.d',
                       output : 'entrance',
                       install_dir : join_paths(dir_syscfg, 'pam.d'),
                       configuration : cfg
	)
endif

install_data(sources : 'data/Xsession',
             install_dir : join_paths(dir_syscfg, proj)
)

##### subdirs
subdir('src/bin')
subdir('src/daemon')
subdir('data/themes/default')
if has_nls
  subdir('po')
endif
