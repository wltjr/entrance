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

/**
 * @brief Authenticate account using PAM
 *
 * @param id of session in array, index value for now, could be seat id later
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_authenticate(int id);

int entrance_pam_item_set(ENTRANCE_PAM_ITEM_TYPE type, const void *value);
const void *entrance_pam_item_get(ENTRANCE_PAM_ITEM_TYPE);
int entrance_pam_env_set(const char *env, const char *value);
char **entrance_pam_env_list_get(void);

/**
 * @brief Allocate memory for Entrance_Pam structs, one per X server/seat
 *
 * @param count the number of pam structs to allocate memory
 */
void entrance_pams_init(int count);

/**
 * @brief Start a new pam conversation session
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param service the name of the service for PAM_SERVICE
 * @param tty terminal name tty or vt
 * @param user user name for login
 *
 * @return Eina_Bool true on success, false on failure
 */
Eina_Bool entrance_pam_start(int id, const char *service, const char *tty, const char *user);

void entrance_pam_shutdown(void);

/**
 * @brief Shutdown all pams ( free array pointer, may have other usage later )
 */
void entrance_pams_shutdown();

/**
 * @brief Start pam session management
 *
 * @param id of session in array, index value for now, could be seat id later
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_session_open(int id);

void entrance_pam_close_session(Eina_Bool opened);
int entrance_pam_passwd_set(const char *passwd);
int entrance_pam_end(void);

#endif /* ENTRANCE_PAM_H_ */
