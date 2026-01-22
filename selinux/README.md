# Entrance SELinux Policy Module

This directory contains SELinux policy for the Entrance Display Manager.

## Files

- **entrance.te** - Type Enforcement policy (rules)
- **entrance.fc** - File Contexts (file labeling)  
- **Makefile** - Build and installation automation

## Installation

### Quick Install (Recommended)
```bash
cd selinux/
sudo make install
```

This will:
1. Compile the policy module
2. Load it into the kernel
3. Relabel all entrance files

### Manual Steps

1. **Build the policy:**
   ```bash
   make
   ```

2. **Load the policy:**
   ```bash
   sudo make load
   ```

3. **Relabel files:**
   ```bash
   sudo make relabel
   ```

## Verification

Check policy is loaded:
```bash
semodule -l | grep entrance
```

Check file contexts:
```bash
ls -Z /usr/sbin/entrance
ls -Z /var/log/entrance.log
ls -Z /var/run/entrance.auth
```

Check for denials:
```bash
sudo ausearch -m avc -ts recent | grep entrance
```

## Uninstallation

Remove the policy module:
```bash
sudo make unload
```

## Troubleshooting

### Policy fails to load
- Ensure you have selinux-policy-devel installed
- Check syntax: `checkmodule -M -m -o entrance.mod entrance.te`

### Still seeing denials
1. Check audit log: `sudo ausearch -m avc -ts recent`
2. Generate additional rules: `sudo audit2allow -a -M entrance_local`
3. Load local policy: `sudo semodule -i entrance_local.pp`

### Files not relabeling
- Ensure semanage is installed: `emerge sys-apps/policycoreutils`
- Manually add contexts: `semanage fcontext -a -t xdm_exec_t '/usr/sbin/entrance'`

## SELinux Modes

- **Enforcing**: Policy is active, denials block operations
- **Permissive**: Policy logs denials but doesn't block (current)
- **Disabled**: SELinux is off

Check mode: `getenforce`  
Set permissive: `sudo setenforce 0`  
Set enforcing: `sudo setenforce 1`

## Policy Details

The entrance policy extends the standard `xdm` domain:

- **entrance daemon** runs as `xdm_t`
- **xauth** (called by entrance) gets permissions to manage Xauthority files
- **File contexts** ensure proper labeling of runtime/config/log files

This allows entrance to work seamlessly with SELinux enforcing mode while maintaining security.
