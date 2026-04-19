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

/**
 * @brief Set PAM session environment variable
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param env environment variable name, uppercase
 * @param value item value to set
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_env_set(int id, const char *env, const char *value);

/**
 * @brief Get PAM information
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param type type of item, mapped from pam types
 *
 * @return const void* item value
 */
const void *entrance_pam_item_get(int id, ENTRANCE_PAM_ITEM_TYPE type);

/**
 * @brief Set PAM information
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param type type of item, mapped from pam types
 * @param value item value to set
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_item_set(int id, ENTRANCE_PAM_ITEM_TYPE type, const void *value);

char **entrance_pam_env_list_get(void);

/**
 * @brief Set the password for PAM authentication
 *
 * @param id of session in array, index value for now, could be seat id later
 * @param passwd user password for login
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_passwd_set(int id, const char *passwd);

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

/**
 * @brief Shutdown all pams ( free array pointer, may have other usage later )
 */
void entrance_pams_shutdown();

/**
 * @brief Close a specific PAM session
 *
 * @param id of session in array, index value for now, could be seat id later
 */
void entrance_pam_session_close(int id);

/**
 * @brief Start pam session management
 *
 * @param id of session in array, index value for now, could be seat id later
 *
 * @return int 0 on successful login, 1 if it fails
 */
int entrance_pam_session_open(int id);

/**
 * @brief Shutdown PAM, free's per pam memory, password and struct pointer
 *
 * @param id of session in array, index value for now, could be seat id later
 */
void entrance_pam_shutdown(int id);

int entrance_pam_end(void);

#endif /* ENTRANCE_PAM_H_ */
