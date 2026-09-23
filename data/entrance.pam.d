#%PAM-1.0
#
# Portable PAM configuration for entrance display manager
# Works across Gentoo, Ubuntu, Debian, Arch, Fedora, etc.
#
# This file explicitly loads required PAM modules instead of using
# distribution-specific includes (system-login, common-*, etc.)
#

#
# Authentication - verify user credentials
#
auth        required        pam_nologin.so
auth        required        pam_env.so
auth        requisite       pam_faillock.so preauth
auth        [success=1 new_authtok_reqd=1 ignore=ignore default=bad]       pam_unix.so nullok
auth        [default=die]   pam_faillock.so authfail
auth        optional        pam_cap.so
-auth       optional        pam_gnome_keyring.so
-auth       optional        pam_kwallet6.so

#
# Account validation - check if account is valid
#
account     required        pam_access.so
account     required        pam_nologin.so
account     required        pam_unix.so
account     required        pam_faillock.so

#
# Password - not typically needed for display manager but included for completeness
#
password   required   pam_unix.so    nullok shadow

#
# Session management - critical for proper login environment
#
session     optional        pam_loginuid.so
session     required        pam_env.so envfile=/etc/profile.env
session     required        pam_limits.so
session     required        pam_env.so
session     required        pam_unix.so shadow yescrypt
-session    optional        pam_elogind.so
-session    optional        pam_openrc.so
-session    optional        pam_systemd.so
-session    optional        pam_gnome_keyring.so auto_start
-session    optional        pam_kwallet6.so auto_start

