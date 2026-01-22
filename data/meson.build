##### entrance.conf
configure_file(input  : 'entrance.conf.in',
               output : 'entrance.conf',
               install_dir : join_paths(dir_syscfg, proj),
               configuration : cfg
)

##### entrance.service
if has_systemd
        configure_file(input  : 'entrance.service.in',
                       output : 'entrance.service',
                       install_dir : '/usr/lib/systemd/system' ,
                       configuration : cfg
        )
endif

##### Xsession
install_data(sources : 'Xsession',
             install_dir : join_paths(dir_syscfg, proj)
)

##### themes
subdir('themes/default')
