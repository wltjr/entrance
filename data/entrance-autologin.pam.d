#%PAM-1.0
#
# PAM configuration for entrance auto-login (passwordless login)
# This file skips authentication checks and grants automatic access.
#

#
# Authentication - skip password verification for auto-login
#
auth        required        pam_nologin.so
auth        required        pam_env.so
auth        required        pam_unix.so nullok
auth        optional        pam_cap.so
-auth       optional        pam_gnome_keyring.so
-auth       optional        pam_kwallet6.so

#
# Account validation - check if account is valid
#
account     required        pam_access.so
account     required        pam_nologin.so
account     required        pam_unix.so

#
# Password management - not used for auto-login
#
password   required   pam_deny.so

#
# Session setup - initialize user session environment
#
session     optional        pam_loginuid.so
session     required        pam_env.so envfile=/etc/profile.env
session     required        pam_limits.so
session     required        pam_env.so
session     required        pam_unix.so
-session    optional        pam_elogind.so
-session    optional        pam_openrc.so
-session    optional        pam_systemd.so
-session    optional        pam_gnome_keyring.so auto_start
-session    optional        pam_kwallet6.so auto_start
