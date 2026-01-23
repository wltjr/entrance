#ifndef ENTRANCE_PAM_H_
#define ENTRANCE_PAM_H_

#include <security/pam_appl.h>

typedef enum ENTRANCE_PAM_ITEM_TYPE_ {
   ENTRANCE_PAM_ITEM_SERVICE = PAM_SERVICE,
   ENTRANCE_PAM_ITEM_USER = PAM_USER,
   ENTRANCE_PAM_ITEM_TTY = PAM_TTY,
   ENTRANCE_PAM_ITEM_RUSER = PAM_RUSER,
   ENTRANCE_PAM_ITEM_RHOST = PAM_RHOST,
   ENTRANCE_PAM_ITEM_CONV = PAM_CONV
} ENTRANCE_PAM_ITEM_TYPE;


int entrance_pam_item_set(ENTRANCE_PAM_ITEM_TYPE type, const void *value);
const void *entrance_pam_item_get(ENTRANCE_PAM_ITEM_TYPE);
int entrance_pam_env_set(const char *env, const char *value);
char **entrance_pam_env_list_get(void);
int entrance_pam_init(const char *service, const char *tty, const char *user);
void entrance_pam_shutdown(void);
int entrance_pam_open_session(void);
void entrance_pam_close_session(Eina_Bool opened);
int entrance_pam_authenticate(void);
int entrance_pam_passwd_set(const char *passwd);
int entrance_pam_end(void);

#endif /* ENTRANCE_PAM_H_ */
