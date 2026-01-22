# SELinux Policy Module Build Integration
#
# This builds and installs the entrance SELinux policy module
# when -Dselinux=true is set
#

# Find required SELinux tools
checkmodule = find_program('checkmodule', required: true)
semodule_package = find_program('semodule_package', required: true)
semodule = find_program('semodule', required: false)

# Determine SELinux policy type (strict/targeted/mcs/mls)
selinux_type_cmd = run_command('sh', '-c',
    'selinuxenabled && sestatus | grep -oP \'Loaded policy name:\\s+\\K\\w+\' 2>/dev/null || echo targeted',
    check: false)
selinux_type = selinux_type_cmd.returncode() == 0 ? selinux_type_cmd.stdout().strip() : 'targeted'

message('SELinux policy type: ' + selinux_type)

# Check for refpolicy development environment
refpolicy_makefile = '/usr/share/selinux/@0@/include/Makefile'.format(selinux_type)
use_refpolicy = run_command('test', '-f', refpolicy_makefile, check: false).returncode() == 0

if use_refpolicy
    message('Using refpolicy build system')
    
    # Build using refpolicy Makefile - runs in source dir, outputs to build dir
    entrance_pp = custom_target('entrance_policy',
        input: ['entrance.te', 'entrance.fc'],
        output: 'entrance.pp',
        command: [
            'sh', '-c',
            'cd @0@ && make -f @1@ entrance.pp && cp -v entrance.pp @2@'.format(
                meson.current_source_dir(),
                refpolicy_makefile,
                meson.current_build_dir() / 'entrance.pp'
            )
        ],
        build_by_default: true,
        install: false
    )
else
    message('Using basic policy build (no refpolicy)')
    
    # Build policy module (.mod) from .te file
    entrance_mod = custom_target('entrance_mod',
        input: 'entrance.te',
        output: 'entrance.mod',
        command: [checkmodule, '-M', '-m', '-o', '@OUTPUT@', '@INPUT@'],
        build_by_default: true
    )
    
    # Package module with file contexts (.pp)
    entrance_pp = custom_target('entrance_policy',
        input: [entrance_mod, 'entrance.fc'],
        output: 'entrance.pp',
        command: [semodule_package, '-o', '@OUTPUT@', '-m', '@INPUT0@', '-f', '@INPUT1@'],
        build_by_default: true,
        install: false
    )
endif

# Install policy module (if semodule is available)
if semodule.found()
    # Build the absolute path to the policy file
    policy_path = meson.current_build_dir() / 'entrance.pp'
    
    # Install script: load the policy module
    meson.add_install_script('sh', '-c',
        'semodule -i "$1" 2>&1 || exit 0',
        '--',
        policy_path
    )
    
    # Relabel installed files with proper SELinux contexts
    meson.add_install_script('sh', '-c',
        'restorecon -R /usr/sbin/entrance /usr/lib64/entrance /etc/entrance /var/cache/entrance /var/log/entrance.log /var/run/entrance* 2>&1 || exit 0'
    )
else
    warning('semodule not found - SELinux policy will be built but not auto-installed')
    warning('Manual installation: sudo semodule -i ' + join_paths(meson.current_build_dir(), 'entrance.pp'))
endif

# Summary
message('SELinux policy module will be built and installed')
message('  Policy file: entrance.pp')
message('  Policy type: ' + selinux_type)
message('  Using refpolicy: ' + (use_refpolicy ? 'yes' : 'no'))
