#%PAM-1.0
#
# PAM configuration for entrance auto-login (passwordless login)
# This file skips authentication checks and grants automatic access.
#

#
# Authentication - skip password verification for auto-login
#
auth       required   pam_env.so
auth       required   pam_permit.so

#
# Account validation - check if account is valid
#
account    required   pam_unix.so

#
# Password management - not used for auto-login
#
password   required   pam_deny.so

#
# Session setup - initialize user session environment
#
session    required   pam_limits.so
session    required   pam_unix.so
-session   optional   pam_elogind.so
-session   optional   pam_gnome_keyring.so auto_start
-session   optional   pam_kwallet6.so auto_start
session    optional   pam_env.so
